#include <filesystem>
#include <iostream>
#include <string_view>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    fs::path target_path{"."};
    fs::path output_path{"./out"};

    for (char **arg = argv + 1; arg < argv + argc; arg++) {
        std::string_view sw_arg{*arg};

        if (arg > argv + 1) {
            std::string_view previous_arg{*(arg - 1)};

            if (previous_arg == "-i") {
                target_path = fs::path{*arg};
            } else if (previous_arg == "-o") {
                output_path = fs::path{*arg};
            }
        }
    }

    // Ensure the output directory exists
    fs::create_directory(output_path);

    for (const auto &file : fs::recursive_directory_iterator{target_path}) {
        std::cout << "Found file " << file.path() << "\n";
    }

    return 0;
}
