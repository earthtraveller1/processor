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
} // namespace neng
