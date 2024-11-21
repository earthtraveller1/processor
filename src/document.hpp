#pragma once

namespace neng {
// TODO: Move this to somewhere more appropriate
enum class Error {
    OK = 0,
    FILE_READ_ERROR = 1,
    FILE_OPEN_ERROR = 2,
};

std::ostream &operator<<(std::ostream &os, Error error);

bool is_line_title(std::string_view line);

// The assumption is that the line is trimmed (so no leading whitespace).
uint8_t count_title_level(std::string_view line);

enum class ParagraphType { NORMAL, HEADER };

std::ostream &operator<<(std::ostream &os, ParagraphType paragraph_type);

struct Paragraph {
    ParagraphType type;
    std::string content;
    uint8_t header_level{0};

    std::string render_to_html(std::string_view paragraph_class) const;
};

struct Document {
    std::vector<Paragraph> paragraphs;

    static void parse_document_line(std::string_view line,
                                    std::string &current_paragraph,
                                    std::vector<Paragraph> &paragraphs);

    static Document parse_document(std::string_view content);

    static std::tuple<Document, Error>
    parse_document_from_file(std::string_view file_path);
};

struct DocumentConfiguration {
    std::string title_class;
    std::string paragraph_class;

    static std::tuple<DocumentConfiguration, Error>
    from_file(std::string_view file_path);

    std::string render_html_to_string(const Document &document) const;
};

} // namespace neng
