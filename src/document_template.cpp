#include "document_template.hpp"
#include "string_utils.hpp"

namespace {
using neng::TemplateSegment;

TemplateSegment parse_expression(std::string_view expression) {
    // For now, this only parses out variables, though in the future it might
    // extend to something more.

    // No validations are done so far, though that can be added later.

    return {
        .type = TemplateSegment::Type::VARIABLE,
        .a = neng::trim_string(expression),
    };
}
} // namespace

namespace neng {
std::ostream &operator<<(std::ostream &stream, TemplateSegment::Type type) {
    switch (type) {
        case TemplateSegment::Type::VARIABLE:
            stream << "TemplateSegment::Type::VARIABLE";
            break;
        case TemplateSegment::Type::TEXT:
            stream << "TemplateSegment::Type::TEXT";
            break;
    }

    return stream;
}

std::tuple<DocumentTemplate, Error>
DocumentTemplate::from_string(std::string_view string) {
    bool collecting_expression = false;
    std::string accumulator;

    std::vector<TemplateSegment> segments;

    for (size_t i = 0; i < string.size(); i++) {
        if (string.substr(i, 3) == "${{") {
            if (collecting_expression) {
                std::cerr << "[ERROR]: Invalid syntax.\n";
                return {{}, Error::INVALID_SYNTAX};
            } else {
                segments.push_back({
                    .type = TemplateSegment::Type::TEXT,
                    .a = accumulator,
                });

                accumulator.clear();
                i += 2;
            }

            collecting_expression = true;
        } else if (string.substr(i, 2) == "}}") {
            if (!collecting_expression) {
                std::cerr << "[ERROR]: Invalid syntax.\n";
                return {{}, Error::INVALID_SYNTAX};
            } else {
                segments.push_back(parse_expression(accumulator));
                accumulator.clear();
                collecting_expression = false;

                i += 1;
            }
        } else {
            accumulator.push_back(string[i]);
        }
    }

    segments.push_back({
        .type = TemplateSegment::Type::TEXT,
        .a = trim_string(accumulator),
    });

    if (collecting_expression) {
        std::cerr << "[ERROR]: Unclosed expression.\n";
    }

    return {
        {
            .segments = segments,
        },
        Error::OK,
    };
}

std::tuple<DocumentTemplate, Error>
DocumentTemplate::from_file(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {{}, Error::FILE_OPEN_ERROR};
    }

    std::string source;

    while (!file.eof()) {
        char buffer[4096];
        file.read(buffer, 4096);
        source.append(buffer, file.gcount());
    }

    return from_string(source);
}

std::string DocumentTemplate::render_to_string(std::string_view title,
                                               std::string_view body) const {
    std::string acc;

    for (const auto &segment : segments) {
        switch (segment.type) {
        case TemplateSegment::Type::TEXT:
            acc += segment.a;
            break;
        case TemplateSegment::Type::VARIABLE:
            if (segment.a == "title") {
                acc += title;
            } else if (segment.a == "body") {
                acc += body;
            }
            break;
        }
    }

    return acc;
}
} // namespace neng
