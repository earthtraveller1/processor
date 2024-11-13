#include <cstring>

#include "string_utils.hpp"

namespace neng {
std::vector<std::string> split_string(std::string_view string,
                                      std::string_view delimiter) {
    std::string current_segment;
    std::vector<std::string> segments;

    for (int i = 0; i < string.size(); i++) {
        if (i + delimiter.size() - 1 < string.size()) {
            bool hit_delimiter = true;
            for (int j = 0; j < delimiter.size(); j++) {
                if (string[i + j] != delimiter[j]) {
                    hit_delimiter = false;
                }
            }

            if (hit_delimiter) {
                i += delimiter.size() - 1;
                segments.push_back(current_segment);
                current_segment = "";
                continue;
            }
        }

        current_segment.push_back(string[i]);
    }

    segments.push_back(current_segment);

    return segments;
}

std::string trim_string_start(std::string_view string) {
    std::string result;

    bool trimming = true;
    for (auto character : string) {
        if (!std::isspace(character)) {
            trimming = false;
        }

        if (!trimming) {
            result.push_back(character);
        }
    }

    return result;
}

std::string trim_string_end(std::string_view string) {
    std::string result;

    size_t trim_location = 0;
    for (int32_t i = string.size() - 1; i >= 0; i--) {
        if (!std::isspace(string[i])) {
            trim_location = i;
            break;
        }
    }

    result = string.substr(0, trim_location+1);

    return result;
}
} // namespace neng
