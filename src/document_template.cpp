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
                i += 3;
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

                i += 2;
            }
        } else {
            accumulator.push_back(string[i]);
        }
    }

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
