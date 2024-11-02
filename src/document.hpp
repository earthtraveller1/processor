#pragma once

namespace neng {
// TODO: Move this to somewhere more appropriate
enum class Error {
    OK = 0,
    FILE_READ_ERROR = 1,
    FILE_OPEN_ERROR = 2,
};

std::ostream& operator<<(std::ostream& os, Error error);

enum class ParagraphType { NORMAL, H1, H2, H3 };

struct Paragraph {
    ParagraphType type;
    std::string content;

    std::string render_to_html(std::string_view paragraph_class) const;
};

struct Document {
    std::vector<Paragraph> paragraphs;
};

struct DocumentTemplate {
    std::string title_class;
    std::string paragraph_class;
    std::string header;
    std::string footer;

    static std::tuple<DocumentTemplate, Error>
    from_file(std::string_view file_path);

    std::string render_html_to_string(const Document &document) const;
};
} // namespace neng
