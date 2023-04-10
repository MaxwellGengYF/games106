#pragma once
#include <winerror.h>
#include <combaseapi.h>
#include "dxcapi.h"
#include <filesystem>
#include <optional>
#include <mutex>
#include <string_view>
#include <span>
#include "dxc_ptr.h"
#include "interface.h"

class DXByteBlob : public IDXByteBlob {
private:
    DxcPtr<IDxcBlob> _blob;

public:
    DXByteBlob(
        DxcPtr<IDxcBlob>&& blob) noexcept : _blob{std::move(blob)} {}
    std::byte const* buffer_ptr() const noexcept override {
        return reinterpret_cast<std::byte const*>(_blob->GetBufferPointer());
    }
    size_t buffer_size() const noexcept override {
        return _blob->GetBufferSize();
    }
};
class ShaderCompilerModule {
    void* _dxil;
    void* _dxc;

public:
    DxcPtr<IDxcCompiler3> _comp;
    ShaderCompilerModule() noexcept;
    ~ShaderCompilerModule() noexcept;
};
class ShaderCompiler : public IShaderCompiler {
private:
    std::optional<ShaderCompilerModule> _module;
    std::mutex _module_inst_mtx;
    CompileResult compile(
        std::string_view code,
        std::span<LPCWSTR> args) noexcept;
    IDxcCompiler3* compiler() noexcept;

public:
    ShaderCompiler() noexcept {}
    ~ShaderCompiler() noexcept {}
    CompileResult compile_compute(
        std::string_view code,
        bool optimize,
        uint32_t shader_model,
        bool enable_unsafe_math) noexcept override;
    RasterCompileResult compile_raster(
        std::string_view code,
        bool optimize,
        uint32_t shader_model,
        bool enable_unsafe_math) noexcept override;
};