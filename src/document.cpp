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
    case Error::TOO_MANY_SLOTS_ERROR:
        os << "TOO_MANY_SLOTS_ERROR";
        break;
    case Error::INVALID_SYNTAX:
        os << "INVALID_SYNTAX";
        break;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, ParagraphType paragraph_type) {
    switch (paragraph_type) {
    case ParagraphType::HEADER:
        os << "ParagraphType::HEADER";
        break;
    case ParagraphType::NORMAL:
        os << "ParagraphType::NORMAL";
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

uint8_t count_title_level(std::string_view line) {
    if (!is_line_title(line)) {
        return 0;
    }

    int level = 0;
    for (auto character : line) {
        if (character != '#') {
            break;
        }

        level += 1;
    }

    return level;
}

std::string Paragraph::render_to_html(std::string_view paragraph_class,
                                      std::string_view title_class) const {
    std::string opener;
    std::string closer;
    switch (type) {
    case ParagraphType::NORMAL:
        opener = "<p class=\""s + std::string(paragraph_class) + "\">"s;
        closer = "</p>";
        break;
    case ParagraphType::HEADER:
        opener = "<h"s + std::to_string(header_level);
        if (header_level == 1) {
            opener += "class=\"" + std::string(title_class) + "\"";
        }
        opener += '>';
        closer = "</h"s + std::to_string(header_level) + ">";
        break;
    }

    std::string result = opener + content + closer;

    return result;
}

std::tuple<DocumentConfiguration, Error>
DocumentConfiguration::from_file(std::string_view file_path) {
    std::ifstream file(file_path.data());

    if (!file) {
        return {{}, Error::FILE_OPEN_ERROR};
    }

    std::string title_class;
    std::string paragraph_class;

    uint32_t line_number = 1;
    while (!file.eof()) {
        std::string line;
        std::getline(file, line);

        // Ignore blank lines
        if (trim_string(line) == "") {
            continue;
        }

        const auto statement_parts = neng::split_string(line, "=");

        if (statement_parts.size() != 2) {
            std::cerr << "[ERROR]: " << file_path << ":" << line_number
                      << ": Invalid syntax\n";
            return {{}, Error::INVALID_SYNTAX};
        }

        if (statement_parts.at(0) == "title_class") {
            title_class = statement_parts.at(1);
        } else if (statement_parts.at(0) == "paragraph_class") {
            paragraph_class = statement_parts.at(1);
        } else {
            std::cerr << "[ERROR]: " << file_path << ":" << line_number
                      << ": Unknown key '" << statement_parts.at(0) << "'\n";
        }

        line_number++;
    }

    return {
        DocumentConfiguration{
            .title_class = title_class,
            .paragraph_class = paragraph_class,
        },
        Error::OK,
    };
}

void Document::parse_document_line(std::string_view line,
                                   std::string &current_paragraph,
                                   std::vector<Paragraph> &paragraphs) {
    bool line_blank = true;
    for (auto character : line) {
        if (!std::isspace(character)) {
            line_blank = false;
            break;
        }
    }

    if (line_blank) {
        if (current_paragraph != "") {
            paragraphs.push_back(Paragraph{
                .type = ParagraphType::NORMAL,
                .content = trim_string(current_paragraph),
            });
        }
        current_paragraph = "";
        return;
    }

    const auto trimmed_line = trim_string(line);
    const auto header_level = count_title_level(trimmed_line);
    if (header_level > 0) {
        if (current_paragraph != "") {
            paragraphs.push_back(Paragraph{
                .type = ParagraphType::NORMAL,
                .content = trim_string(current_paragraph),
            });
        }
        current_paragraph = trimmed_line;
        paragraphs.push_back(Paragraph{
            .type = ParagraphType::HEADER,
            .content = trim_string(current_paragraph.substr(
                current_paragraph.find_first_of(' ') + 1)),
            .header_level = header_level,
        });
        current_paragraph = "";
        return;
    }

    current_paragraph += trim_string(line) + " ";
}

Document Document::parse_document(std::string_view document) {
    std::vector<Paragraph> paragraphs;
    std::string current_paragraph;

    const auto lines = split_string(document, "\n");

    for (const auto &line : lines) {
        parse_document_line(line, current_paragraph, paragraphs);
    }

    paragraphs.push_back(Paragraph{
        .type = ParagraphType::NORMAL,
        .content = trim_string(current_paragraph),
    });

    return Document{.paragraphs = paragraphs};
}

std::tuple<Document, Error>
Document::parse_document_from_file(std::string_view file_path) {
    std::ifstream file(file_path.data());
    if (!file.is_open()) {
        return {{}, Error::FILE_OPEN_ERROR};
    }

    std::vector<Paragraph> paragraphs;
    std::string current_paragraph;

    while (!file.eof()) {
        std::string line;
        std::getline(file, line);

        parse_document_line(line, current_paragraph, paragraphs);
    }

    paragraphs.push_back(Paragraph{
        .type = ParagraphType::NORMAL,
        .content = trim_string(current_paragraph),
    });

    return {{paragraphs}, Error::OK};
}

std::string
DocumentConfiguration::render_html_to_string(const Document &document) const {
    std::string result;

    result.append("<body>");
    for (const auto &paragraph : document.paragraphs) {
        result.append(paragraph.render_to_html(paragraph_class, title_class));
    }
    result.append("</body>");

    return result;
}

std::tuple<BasicDocumentTemplate, Error>
BasicDocumentTemplate::from_file(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {{}, Error::FILE_OPEN_ERROR};
    }

    std::string before;
    std::string after;

    bool at_before = true;

    while (!file.eof()) {
        std::string line;
        std::getline(file, line);

        const std::string_view slot_literal = "${{slot}}";
        const auto slot_position = line.find("${{slot}}");
        if (slot_position == std::string::npos) {
            if (at_before) {
                before += line + ' ';
                continue;
            } else {
                after += line + ' ';
                continue;
            }
        }

        if (!at_before) {
            std::cerr << "[ERROR]: The slot is found too many times! You can't "
                         "have two slots in a template!\n";
            return {{}, Error::TOO_MANY_SLOTS_ERROR};
        }

        before += line.substr(0, slot_position);
        at_before = false;
        after += line.substr(slot_position + slot_literal.size()) + ' ';
    }

    return {
        BasicDocumentTemplate{
            .before = trim_string(before),
            .after = trim_string(after),
        },
        Error::OK,
    };
}

std::string BasicDocumentTemplate::insert_body(std::string_view result) const {
    return before + ' ' + std::string(result) + ' ' + after;
}

} // namespace neng
