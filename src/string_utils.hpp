#pragma once

namespace neng {
std::vector<std::string> split_string(std::string_view string,
                                      std::string_view delimiter);

std::string trim_string_start(std::string_view string);

std::string trim_string_end(std::string_view string);

std::string trim_string(std::string_view string);
} // namespace neng
