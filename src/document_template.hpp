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

struct DocumentTemplate {
    std::vector<TemplateSegment> segments;

    static std::tuple<DocumentTemplate, Error>
    from_string(std::string_view string);

    static std::tuple<DocumentTemplate, Error>
    from_file(std::filesystem::path path);
};
} // namespace neng
