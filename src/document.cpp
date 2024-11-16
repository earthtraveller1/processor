#include "document.hpp"

#include "string_utils.hpp"
#include <fstream>

namespace neng {

using namespace std::literals::string_literals;

std::ostream &operator<<(std::ostream &os, Error error) {
    switch (error) {
    case Error::OK:
        os << "OK";
        break;
    case Error::FILE_OPEN_ERROR:
        os << "FILE_OPEN_ERROR";
        break;
    case Error::FILE_READ_ERROR:
        os << "FILE_READ_ERROR";
        break;
    }

    return os;
}

bool is_line_title(std::string_view line) {
    if (!line.starts_with('#')) {
        return false;
    }

    char previous_char = 0;
    for (auto character : line) {
        if (character != '#') {
            if (character == ' ') {
                return true;
            } else {
                return false;
            }
        }
        previous_char = character;
    }

    return false;
}

std::string Paragraph::render_to_html(std::string_view paragraph_class) const {
    std::string opener;
    std::string closer;
    switch (type) {
    case ParagraphType::NORMAL:
        opener = "<p class=\""s + std::string(paragraph_class) + "\"/>"s;
        closer = "</p>";
        break;
    case ParagraphType::H1:
        opener = "<h1 class=\"header1\">";
        closer = "</h1>";
        break;
    case ParagraphType::H2:
        opener = "<h2 class=\"header2\">";
        closer = "</h2>";
        break;
    case ParagraphType::H3:
        opener = "<h3 class=\"header3\">";
        closer = "</h3>";
        break;
    }

    std::string result = opener + content + closer;

    return result;
}

std::tuple<DocumentTemplate, Error>
DocumentTemplate::from_file(std::string_view file_path) {
    std::ifstream file(file_path.data());

    if (!file) {
        return {{}, Error::FILE_OPEN_ERROR};
    }

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

        if (statement_parts.at(0) == "title_class") {
            title_class = statement_parts.at(1);
        } else if (statement_parts.at(0) == "paragraph_class") {
            paragraph_class = statement_parts.at(1);
        } else if (statement_parts.at(0) == "header" &&
                   statement_parts.at(1) == "{") {
            reading_header = true;
        } else if (statement_parts.at(0) == "footer" &&
                   statement_parts.at(1) == "{") {
            reading_footer = true;
        }
    }

    return {
        DocumentTemplate{
            .title_class = title_class,
            .paragraph_class = paragraph_class,
            .header = header,
            .footer = footer,
        },
        Error::OK,
    };
}

Document Document::parse_document(std::string_view document) {
    std::vector<Paragraph> paragraphs;
    std::string current_paragraph;

    const auto lines = split_string(document, "\n");

    for (const auto &line : lines) {
        bool line_blank = true;
        for (auto character : line) {
            if (!std::isspace(character)) {
                line_blank = false;
                break;
            }
        }

        if (line_blank) {
            paragraphs.push_back(Paragraph{
                .type = ParagraphType::NORMAL,
                .content = current_paragraph,
            });
            current_paragraph = "";
            continue;
        }

        const auto trimmed_line = trim_string(line);
        if (is_line_title(trimmed_line)) {
            paragraphs.push_back(Paragraph{.type = ParagraphType::NORMAL,
                                           .content = current_paragraph});
            current_paragraph = trimmed_line;
            paragraphs.push_back(Paragraph{.type = ParagraphType::H1,
                                           .content = current_paragraph});
            current_paragraph = "";
            continue;
        }

        current_paragraph += trim_string(line) + " ";
    }

    return Document{.paragraphs = paragraphs};
}

std::tuple<Document, Error>
Document::parse_document_from_file(std::string_view file_path) {
    std::ifstream file(file_path.data());
    if (!file.is_open()) {
        return {{}, Error::FILE_OPEN_ERROR};
    }

    std::string contents;

    while (!file.eof()) {
        std::string line;
        std::getline(file, line);

        contents += line;
    }

    return {parse_document(contents), Error::OK};
}

std::string
DocumentTemplate::render_html_to_string(const Document &document) const {
    std::string result;

    result.append("<body>");
    result.append(header);
    for (const auto &paragraph : document.paragraphs) {
        result.append(paragraph.render_to_html(paragraph_class));
    }
    result.append(footer);
    result.append("</body>");

    return result;
}

} // namespace neng
