#pragma once
#include <memory>
#include <string>
#include <variant>

#ifdef GAMES106_WINDOWS
#ifdef SHADER_COMPILER_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif

class IDXByteBlob {
public:
    virtual ~IDXByteBlob() noexcept = default;
    virtual std::byte const* buffer_ptr() const noexcept = 0;
    virtual size_t buffer_size() const noexcept = 0;
};
using CompileResult = std::variant<
    std::unique_ptr<IDXByteBlob>,
    std::string>;
struct RasterCompileResult {
    CompileResult vertex;
    CompileResult pixel;
};
class IShaderCompiler {
protected:
    ~IShaderCompiler() = default;

public:
    virtual CompileResult compile_compute(
        std::string_view code,
        bool optimize,
        uint32_t shader_model,
        bool enable_unsafe_math) noexcept = 0;
    virtual RasterCompileResult compile_raster(
        std::string_view code,
        bool optimize,
        uint32_t shader_model,
        bool enable_unsafe_math) noexcept = 0;
};
extern "C" DLL_EXPORT IShaderCompiler* singleton();