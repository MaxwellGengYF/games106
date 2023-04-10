#include "interface.h"
#include <iostream>
#include <filesystem>
int main(int argc, char** argv) {
    auto runtime_dir = std::filesystem::path{(argv[0])}.parent_path();
    while (true) {
        std::string file_name;
        std::cout << "File name:\n";
        std::cin >> file_name;
        auto file_path = runtime_dir / file_name;
        auto f = fopen(file_path.string().c_str(), "rb");
        if (!f) {
            std::cout << "File not found!\n";
            continue;
        }
        fseek(f, 0, SEEK_END);
        size_t length = ftell(f);
        fseek(f, 0, SEEK_SET);
        std::vector<char> data;
        data.resize(length);
        fread(data.data(), length, 1, f);
        fclose(f);
        auto compile_result = singleton()->compile_raster(std::string_view{data.data(), data.size()}, true, 60, true);
        char const* ext_name = "vert.spv";
        char const* shader_name = "Vertex-shader";
        file_path.replace_extension();
        auto shader_result = [&]<typename T>(T const& r) {
            // compile error
            if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "Compile error: " << r << '\n';
            } else {
                auto str = file_path.string();
                str += ".";
                str += ext_name;
                auto f = fopen(str.c_str(), "wb");
                if (f) {
                    fwrite(r->buffer_ptr(), r->buffer_size(), 1, f);
                    fclose(f);
                    std::cout << shader_name << " compile finished, file saved as " << str << '\n';
                } else {
                    std::cout << shader_name << " compile finished, file saved failed.\n";
                }
            }
        };
        std::visit(
            shader_result,
            compile_result.vertex);
        ext_name = "frag.spv";
        shader_name = "Pixel-shader";
        std::visit(
            shader_result,
            compile_result.pixel);
    }
    return 0;
}