#include "string_utils.hpp"
#include "tests.hpp"

using namespace std::literals::string_literals;
namespace fs = std::filesystem;

namespace {
enum class Error {
    OK = 0,
    FILE_READ_ERROR = 1,
    FILE_OPEN_ERROR = 2,
};

enum class ParagraphType { NORMAL, H1, H2, H3 };

struct Paragraph {
    ParagraphType type;
    std::string content;

    std::string render_to_html(std::string_view paragraph_class) const {
        std::string opener;
        std::string closer;
        switch (type) {
        case ParagraphType::NORMAL:
            opener = "<p class=\""s + std::string(paragraph_class) + "\"/>"s;
            closer = "</p>";
        case ParagraphType::H1:
            opener = "<h1 class=\"header1\">";
            closer = "</h1>";
        case ParagraphType::H2:
            opener = "<h2 class=\"header2\">";
            closer = "</h2>";
        case ParagraphType::H3:
            opener = "<h3 class=\"header3\">";
            closer = "</h3>";
        }

        std::string result = opener + content + closer;

        return result;
    }
};

struct Document {
    std::vector<Paragraph> paragraphs;
};

struct DocumentTemplate {
    std::string css_file;
    std::string title_class;
    std::string paragraph_class;
    std::string header;
    std::string footer;

    static std::tuple<DocumentTemplate, Error>
    from_file(std::string_view file_path) {
        std::ifstream file(file_path.data());

        if (!file) {
            return {{}, Error::FILE_OPEN_ERROR};
        }

        std::string css_file;
        std::string title_class;
        std::string paragraph_class;
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
            } else if (statement_parts.at(0) == "title_class") {
                title_class = statement_parts.at(1);
            } else if (statement_parts.at(0) == "paragraph_class") {
                paragraph_class = statement_parts.at(1);
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
                .title_class = title_class,
                .paragraph_class = paragraph_class,
                .header = header,
                .footer = footer,
            },
            Error::OK,
        };
    }

    std::string render_html_to_string(const Document &document) {
        std::string result;

        result.append("<body>");
        for (const auto &paragraph : document.paragraphs) {
            result.append(paragraph.render_to_html(paragraph_class));
        }
        result.append("</body>");

        return result;
    }
};
} // namespace

int main(int argc, char **argv) {
    fs::path target_path{"."};
    fs::path output_path{"./out"};

    for (char **arg = argv + 1; arg < argv + argc; arg++) {
        std::string_view sw_arg{*arg};

        if (sw_arg == "--test") {
            neng::run_tests();
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
