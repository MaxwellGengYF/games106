#include "compiler.h"
#include <assert.h>
#include <iostream>
#ifndef CHECK_HRESULT
#ifdef NDEBUG
#define CHECK_HRESULT(x) (x)
#else
#define CHECK_HRESULT(x)     \
    {                        \
        HRESULT hr_ = (x);   \
        assert(hr_ == S_OK); \
    }
#endif
#endif

#ifdef GAMES106_WINDOWS
namespace detail {
template<typename PathChar>
void set_dll_directory(PathChar const* path) noexcept {
    if constexpr (sizeof(PathChar) == 1) {
        SetDllDirectoryA(path);
    } else {
        SetDllDirectoryW(path);
    }
}

void* dynamic_module_load(const std::filesystem::path& path) noexcept {
    bool has_parent_path = path.has_parent_path();
    using PathType = std::filesystem::path::value_type;
    if (has_parent_path) {
        set_dll_directory(path.parent_path().c_str());
    }
    auto path_string = path.filename().string();
    auto module = LoadLibraryA(path_string.c_str());
    if (has_parent_path) {
        set_dll_directory<PathType>(nullptr);
    }
    return module;
}
void dynamic_module_destroy(void* handle) noexcept {
    if (handle != nullptr) { FreeLibrary(reinterpret_cast<HMODULE>(handle)); }
}
void* dynamic_module_find_symbol(void* handle, std::string_view name_view) noexcept {
    static thread_local std::string name;
    name = name_view;
    auto symbol = GetProcAddress(reinterpret_cast<HMODULE>(handle), name.c_str());
    return reinterpret_cast<void*>(symbol);
}
}// namespace detail
#elif defined(GAMES106_MACOSX) or defined(GAMES106_LINUX)
#include <unistd.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <cxxabi.h>
namespace detail {
void* dynamic_module_load(const std::filesystem::path& path) noexcept {
    auto p = path;
    for (auto ext : {".so", ".dylib"}) {
        p.replace_extension(ext);
        if (auto module = dlopen(p.c_str(), RTLD_LAZY); module != nullptr) {
            return module;
        }
    }
    return nullptr;
}
void dynamic_module_destroy(void* handle) noexcept {
    if (handle != nullptr) { dlclose(handle); }
}
void* dynamic_module_find_symbol(void* handle, std::string_view name_view) noexcept {
    static thread_local std::string name;
    name = name_view;
    Clock clock;
    auto symbol = dlsym(handle, name.c_str());
    return symbol;
}
}// namespace detail
#endif

ShaderCompilerModule::ShaderCompilerModule() noexcept
    : _dxil{detail::dynamic_module_load("dxil")},
      _dxc(detail::dynamic_module_load("dxcompiler")) {
    using namespace std::literals;
    assert(_dxil);
    assert(_dxc);
    auto void_ptr = detail::dynamic_module_find_symbol(_dxc, "DxcCreateInstance"sv);
    reinterpret_cast<HRESULT(__stdcall*)(const IID&, const IID&, LPVOID*)>(void_ptr)(CLSID_DxcCompiler, IID_PPV_ARGS(_comp.GetAddressOf()));
}
ShaderCompilerModule::~ShaderCompilerModule() {
    _comp.clear();
    detail::dynamic_module_destroy(_dxc);
    detail::dynamic_module_destroy(_dxil);
}
CompileResult ShaderCompiler::compile(
    std::string_view code,
    std::span<LPCWSTR> args) noexcept {
    DxcBuffer buffer{
        code.data(),
        code.size(),
        CP_ACP};
    DxcPtr<IDxcResult> compile_result;
    CHECK_HRESULT(compiler()->Compile(
        &buffer,
        args.data(),
        args.size(),
        nullptr,
        IID_PPV_ARGS(compile_result.GetAddressOf())));
    HRESULT status;
    CHECK_HRESULT(compile_result->GetStatus(&status));
    if (status == 0) {
        DxcPtr<IDxcBlob> result_blob;
        CHECK_HRESULT(compile_result->GetResult(result_blob.GetAddressOf()));
        return std::make_unique<DXByteBlob>(std::move(result_blob));
    } else {
        DxcPtr<IDxcBlobEncoding> err_buffer;
        CHECK_HRESULT(compile_result->GetErrorBuffer(err_buffer.GetAddressOf()));
        auto err_str = std::string_view(
            reinterpret_cast<char const*>(err_buffer->GetBufferPointer()),
            err_buffer->GetBufferSize());
        return std::string(err_str);
    }
    if (compile_result) {
        compile_result->Release();
    }
}
IDxcCompiler3* ShaderCompiler::compiler() noexcept {
    std::lock_guard lck{_module_inst_mtx};
    if (_module) return _module->_comp.Get();
    _module.emplace();
    return _module->_comp.Get();
}
namespace detail {
template<typename Vec>
static void add_basic_flags(Vec& args) noexcept {
    auto new_args =
        {L"-Gfa",
         L"-all-resources-bound",
         L"-no-warnings",
         L"-spirv",
         L"-fspv-extension=SPV_KHR_multiview",
         L"-fspv-extension=SPV_KHR_shader_draw_parameters",
         L"-fspv-extension=SPV_EXT_descriptor_indexing",
         L"-HV 2021"};
    for (auto&& i : new_args) {
        args.push_back(i);
    }
}
template<typename Vec>
static void add_unsafe_math_flags(Vec& args) noexcept {
    auto new_args =
        {L"-opt-enable",
         L"-funsafe-math-optimizations",
         L"-opt-enable",
         L"-fassociative-math",
         L"-opt-enable",
         L"-freciprocal-math"};
    for (auto&& i : new_args) {
        args.push_back(i);
    }
}
static std::wstring get_sm(uint32_t shader_model) noexcept {
    std::wstring sm_str;
    sm_str += std::to_wstring(shader_model / 10);
    sm_str += L'_';
    sm_str += std::to_wstring(shader_model % 10);
    return sm_str;
}
}// namespace detail
CompileResult ShaderCompiler::compile_compute(
    std::string_view code,
    bool optimize,
    uint32_t shader_model,
    bool enable_unsafe_math) noexcept {
    std::vector<LPCWSTR> args;
    std::wstring sm_str;
    sm_str += L"cs_";
    sm_str += detail::get_sm(shader_model);
    args.push_back(L"/T");
    args.push_back(sm_str.c_str());
    detail::add_basic_flags(args);
    if (enable_unsafe_math) {
        detail::add_unsafe_math_flags(args);
    }
    if (optimize) {
        args.push_back(L"-O3");
    }
    return compile(code, args);
}
RasterCompileResult ShaderCompiler::compile_raster(
    std::string_view code,
    bool optimize,
    uint32_t shader_model,
    bool enable_unsafe_math) noexcept {
    std::vector<LPCWSTR> args;
    detail::add_basic_flags(args);
    if (enable_unsafe_math) {
        detail::add_unsafe_math_flags(args);
    }
    if (optimize) {
        args.push_back(L"-O3");
    }
    args.push_back(L"/T");
    auto size = args.size();
    std::wstring sm_str;
    sm_str += L"vs_";
    sm_str += detail::get_sm(shader_model);
    args.push_back(sm_str.c_str());
    args.push_back(L"/DVS");
    RasterCompileResult bin;
    bin.vertex = compile(code, args);
    args.resize(size);
    sm_str.clear();
    sm_str += L"ps_";
    sm_str += detail::get_sm(shader_model);
    args.push_back(sm_str.c_str());
    args.push_back(L"/DPS");
    bin.pixel = compile(code, args);
    return bin;
}
extern "C" DLL_EXPORT IShaderCompiler* singleton() {
    static ShaderCompiler compiler;
    return &compiler;
}
#undef CHECK_HRESULT