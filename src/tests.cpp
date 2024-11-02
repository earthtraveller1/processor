#include <sstream>

#include "document.hpp"
#include "string_utils.hpp"

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

#define ASSERT_EQ(a, b)                                                        \
    do {                                                                       \
        if ((a) != (b)) {                                                      \
            std::stringstream ss;                                              \
            ss << "Assertion '" << (a) << " == " << (b) << "' failed!";        \
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
                ASSERT_EQ(expected[i], results[i]);
            }

            SUCCESS;
        });

    run_test(
        "parsing basic templates", TEST {
            using neng::DocumentTemplate;

            const auto [templ, err] =
                DocumentTemplate::from_file("tests/basic.neng");
            ASSERT_EQ(err, neng::Error::OK);

            ASSERT_EQ(templ.title_class, "title");
            ASSERT_EQ(templ.paragraph_class, "paragraph");
            ASSERT_EQ(templ.header,
                      "    <nav>The end of the world is upon us!</nav>");
            ASSERT_EQ(templ.footer, "    <small>Yes, indeed!</small>");

            SUCCESS;
        });

    run_test(
        "rendering basic template", TEST {
            using neng::DocumentTemplate;
            using neng::Document;

            const auto [templ, err] =
                DocumentTemplate::from_file("tests/basic.neng");
            ASSERT_EQ(err, neng::Error::OK);

            Document document;
            document.paragraphs.push_back({
                .type = ParagraphType::H1,
                .content = "Hello!",
            });
            document.paragraphs.push_back({
                .type = ParagraphType::NORMAL,
                .content = "This is a test!",
            });

            const auto result = templ.render_html_to_string(document);
            ASSERT_EQ(
                result,
                "<body>    <nav>The end of the world is upon us!</nav><h3 "
                "class=\"header3\">Hello!</h3><h3 class=\"header3\">This "
                "is a test!</h3>    <small>Yes, indeed!</small></body>");

            SUCCESS;
        });
}
} // namespace neng
