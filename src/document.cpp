#include "document.hpp"

#include "string_utils.hpp"

namespace neng {

using namespace std::literals::string_literals;

std::string Paragraph::render_to_html(std::string_view paragraph_class) const {
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
            reading_footer = false;
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

std::string DocumentTemplate::render_html_to_string(const Document &document) {
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