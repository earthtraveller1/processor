#include "document.hpp"
#include "document_template.hpp"
#include "tests.hpp"

#include <fstream>
#include <optional>

using namespace std::literals::string_literals;
namespace fs = std::filesystem;

namespace {
using neng::Document;
using neng::DocumentConfiguration;
using neng::DocumentTemplate;
using neng::Error;

Error render_single_file(const fs::path &in_path, const fs::path &out_path,
                         const DocumentConfiguration &document_config,
                         const DocumentTemplate &document_template) {
    const auto [document, error] = Document::parse_document_from_file(in_path);
    if (error != Error::OK) {
        return error;
    }

    const auto rendered_result =
        document_config.render_html_to_string(document);
    const auto slotted_result = document_template.render_to_string(
        document.get_title(), rendered_result);

    std::ofstream out_file(out_path);
    if (!out_file.is_open()) {
        return Error::FILE_OPEN_ERROR;
    }

    out_file << slotted_result;

    return Error::OK;
}

Error render_directory(const fs::path &config_path,
                       const fs::path &template_path, const fs::path &in_path,
                       const fs::path &out_path) {
    if (!fs::exists(config_path)) {
        std::cerr << "[ERROR]: config.neng file does not exist. Make sure a "
                     "config.neng file exists in "
                  << in_path << '\n';
        return Error::FILE_DOES_NOT_EXIST;
    }

    if (!fs::exists(template_path)) {
        std::cerr << "[ERROR]: template.html file does not exist. Make sure a "
                     "template.html file exists in "
                  << in_path << '\n';
        return Error::FILE_DOES_NOT_EXIST;
    }

    const auto [document_config, error] =
        DocumentConfiguration::from_file(config_path.string());
    if (error != Error::OK) {
        std::cerr << "[ERROR]: Failed to parse the configuration: " << error
                  << '\n';
        return error;
    }

    const auto [document_template, error2] =
        neng::DocumentTemplate::from_file(template_path);
    if (error2 != Error::OK) {
        std::cerr << "[ERROR]: Failed to parse the template: " << error << '\n';
        return error;
    }

    // Ensure the output directory exists
    fs::create_directory(out_path);

    fs::path pages_path = in_path / "pages/";

    if (!fs::exists(pages_path)) {
        std::cerr << "[ERROR]: pages directory does not exist.\n";
        return Error::NO_PAGES_DIRECTORY;
    }

    for (const auto &file : fs::recursive_directory_iterator{in_path}) {
        using neng::Document;

        const auto file_path = file.path();
        if (file_path.extension() == ".md") {
            const auto file_output =
                out_path /
                fs::relative(file_path, in_path).replace_extension(".html");

            auto file_output_dir = file_output;
            file_output_dir.remove_filename();

            fs::create_directories(file_output_dir);

            render_single_file(file_path, file_output, document_config,
                               document_template);
        }
    }

    return Error::OK;
}
} // namespace

int main(int argc, char **argv) {
    fs::path target_path{"./"};
    fs::path output_path{"./out"};

    std::optional<fs::path> user_specified_config_path;
    std::optional<fs::path> user_specified_template_path;

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
            } else if (previous_arg == "-c") {
                user_specified_config_path = fs::path{*arg};
            } else if (previous_arg == "-t") {
                user_specified_template_path = fs::path{*arg};
            }
        }
    }

    if (fs::is_directory(target_path)) {
        if (!fs::is_directory(output_path)) {
            std::cerr << "[ERROR]: " << output_path << " is not a directory.\n";
            return EXIT_FAILURE;
        }

        const auto config_path = target_path / "config.neng";
        const auto template_path = target_path / "template.html";

        const auto result = render_directory(config_path, template_path,
                                             target_path, output_path);
        if (result != Error::OK) {
            std::cerr << "[ERROR]: Failed to process the directory: " << result
                      << "\n";
        }
    } else {
        if (fs::is_directory(output_path)) {
            std::cerr << "[ERROR]: " << output_path << " is a directory.\n";
            return EXIT_FAILURE;
        }

        if (!user_specified_config_path.has_value()) {
            std::cerr << "[ERROR]: When processing a single file, you need to "
                         "specify a configuration path.\n";
            return EXIT_FAILURE;
        }

        if (!user_specified_template_path.has_value()) {
            std::cerr << "[ERROR]: When processing a single file, you need to "
                         "specify a template path.\n";
            return EXIT_FAILURE;
        }

        const auto config_path = user_specified_config_path.value();
        const auto template_path = user_specified_template_path.value();

        const auto [document_config, error] =
            DocumentConfiguration::from_file(config_path.string());
        if (error != Error::OK) {
            std::cerr << "[ERROR]: Failed to parse the configuration: " << error
                      << '\n';
            return EXIT_FAILURE;
        }

        const auto [document_template, error2] =
            neng::DocumentTemplate::from_file(template_path);
        if (error2 != Error::OK) {
            std::cerr << "[ERROR]: Failed to parse the template: " << error
                      << '\n';
            return EXIT_FAILURE;
        }

        const auto error3 = render_single_file(
            target_path, output_path, document_config, document_template);
        if (error3 != Error::OK) {
            std::cerr << "[ERROR]: Failed to render " << target_path << ": "
                      << error3 << '\n';
            return EXIT_FAILURE;
        }
    }

    return 0;
}
