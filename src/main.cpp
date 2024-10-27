#include "string_utils.hpp"

namespace fs = std::filesystem;

namespace {
enum class Error {
    OK = 0,
    FILE_READ_ERROR = 1,
    FILE_OPEN_ERROR = 2,
};

struct DocumentTemplate {
    std::string css_file;
    std::string title_element;
    std::string paragraph_element;
    std::string header;
    std::string footer;

    std::tuple<DocumentTemplate, Error> from_file(std::string_view file_path) {
        std::ifstream file(file_path.data());

        if (!file) {
            return {{}, Error::FILE_OPEN_ERROR};
        }

        std::string css_file;
        std::string title_element;
        std::string paragraph_element;
        std::string header;
        std::string footer;

        bool reading_header = false, reading_footer = false;

        while (!file.eof()) {
            std::string line;
            std::getline(file, line);

            if (reading_header) {
                if (line != "}") {
                    header.append(line);
                } else {
                    reading_header = false;
                }

                continue;
            }

            if (reading_footer) {
                if (line != "}") {
                    footer.append(line);
                } else {
                    reading_footer = false;
                }

                continue;
            }

            const auto statement_parts = neng::split_string(line, "=");

            if (statement_parts.at(0) == "css_file") {
                css_file = statement_parts.at(1);
            } else if (statement_parts.at(0) == "title_element") {
                title_element = statement_parts.at(1);
            } else if (statement_parts.at(0) == "paragraph_element") {
                paragraph_element = statement_parts.at(1);
            } else if (statement_parts.at(0) == "header" &&
                       statement_parts.at(1) == "{") {
                reading_header = true;
            } else if (statement_parts.at(0) == "footer" &&
                       statement_parts.at(1) == "{") {
                reading_footer = false;
            }
        }

        return {
            DocumentTemplate{
                .css_file = css_file,
                .title_element = title_element,
                .paragraph_element = paragraph_element,
                .header = header,
                .footer = footer,
            },
            Error::OK,
        };
    }
};
} // namespace

void run_tests();

int main(int argc, char **argv) {
    fs::path target_path{"."};
    fs::path output_path{"./out"};

    for (char **arg = argv + 1; arg < argv + argc; arg++) {
        std::string_view sw_arg{*arg};

        if (sw_arg == "--test") {
            run_tests();
            return EXIT_SUCCESS;
        }

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
