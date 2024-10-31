#include "string_utils.hpp"
#include <sstream>

struct TestResult {
    bool passed;
    std::string error_message;
};

template <typename F>
concept TestFunction = requires(F f) {
    { f() } -> std::same_as<TestResult>;
};

template <TestFunction F>
inline void run_test(std::string_view name, F function) {
    std::cout << "Running \033[36m" << name << "\033[0m \t.........";
    auto result = function();
    if (result.passed) {
        std::cout << "\033[32m Passed \033[0m\n";
    } else {
        std::cout << "\033[91m Failed \n";
        std::cout << "[ERROR]: " << result.error_message << "\033[0m\n";
    }
}

#define ASSERT(condition)                                                      \
    do {                                                                       \
        if (!(condition)) {                                                    \
            return TestResult{                                                 \
                .passed = false,                                               \
                .error_message = "Assertion " #condition " failed!",           \
            };                                                                 \
        }                                                                      \
    } while (0)

#define ASSERT_EQ(a, b, msg_a, msg_b)                                          \
    do {                                                                       \
        if ((a) != (b)) {                                                      \
            std::stringstream ss;                                              \
            ss << "Assertion '" << msg_a << " == " << msg_b << "' failed!";    \
            return TestResult{.passed = false, .error_message = ss.str()};     \
        }                                                                      \
    } while (0)

#define SUCCESS                                                                \
    return TestResult { .passed = true, .error_message = "", }

#define TEST []() -> TestResult

namespace neng {
void run_tests() {
    run_test(
        "splitting strings", TEST {
            std::string hello = "Yes,No,You,Bad";
            std::vector<std::string> expected{"Yes", "No", "You", "Bad"};

            const auto results = neng::split_string(hello, ",");

            for (int i = 0; i < expected.size(); i++) {
                ASSERT_EQ(expected[i], results[i], expected[i], results[i]);
            }

            SUCCESS;
        });
}
} // namespace neng
