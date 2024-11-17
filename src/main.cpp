#include "tests.hpp"

using namespace std::literals::string_literals;
namespace fs = std::filesystem;

namespace {
} // namespace

int main(int argc, char **argv) {
    fs::path target_path{"./pages"};
    fs::path output_path{"./out"};

    for (char **arg = argv + 1; arg < argv + argc; arg++) {
        std::string_view sw_arg{*arg};

        #ifdef PROCESSOR_BUILD_TESTS
        if (sw_arg == "--test") {
            neng::run_tests();
            return EXIT_SUCCESS;
        }
        #endif

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
