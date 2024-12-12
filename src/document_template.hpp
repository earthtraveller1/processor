#pragma once

#include "document.hpp"

namespace neng {
struct TemplateSegment {
    enum class Type {
        VARIABLE,
        TEXT,
    };

    Type type;

    std::string a;
};

std::ostream& operator<<(std::ostream& stream, TemplateSegment::Type type);

struct DocumentTemplate {
    std::vector<TemplateSegment> segments;

    static std::tuple<DocumentTemplate, Error>
    from_string(std::string_view string);

    static std::tuple<DocumentTemplate, Error>
    from_file(const std::filesystem::path& path);

    std::string render_to_string(std::string_view title,
                                 std::string_view body) const;
};
} // namespace neng
