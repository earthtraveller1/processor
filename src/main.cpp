#include "tests.hpp"
#include "document.hpp"

#include <fstream>

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

    if (!fs::exists(target_path)) {
        std::cerr << "[ERROR]: target directory does not exist.\n";
        return EXIT_FAILURE;
    }

    const auto template_path = target_path / "template.neng";
    if (!fs::exists(template_path)) {
        std::cerr << "[ERROR]: template.neng file does not exist. Make sure a "
                     "template.neng file exists in "
                  << target_path << '\n';
        return EXIT_FAILURE;
    }

    using neng::DocumentTemplate;
    using neng::Error;

    const auto [document_template, error] = DocumentTemplate::from_file(template_path.string());
    if (error != Error::OK) {
        std::cerr << "[ERROR]: Failed to parse the template: " << error << '\n';
        return EXIT_FAILURE;
    }

    // Ensure the output directory exists
    fs::create_directory(output_path);

    for (const auto &file : fs::recursive_directory_iterator{target_path}) {
        using neng::Document;

        const auto file_path = file.path();
        if (file_path.extension() == ".md") {
            const auto [document, error] = Document::parse_document_from_file(file_path.string());
            if (error != Error::OK) {
                std::cerr << "[ERROR]: Failed to process document " << file_path
                          << '\n';
                continue;
            }
            
            const auto result = document_template.render_html_to_string(document);
            const auto file_output =
                output_path / fs::relative(file_path, target_path).concat(".html");

            auto file_output_dir = file_output;
            file_output_dir.remove_filename();

            fs::create_directories(file_output_dir);

            std::ofstream output_file(file_output);
            if (!output_file.is_open()) {
                std::cerr << "[ERROR]: Failed to open " << file_output << " for writing.\n";
                continue;
            }

            output_file << result;
        }
    }

    return 0;
}
