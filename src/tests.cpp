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
        "trimming strings", TEST {
            using neng::trim_string_end;
            using neng::trim_string_start;

            auto result = trim_string_start("      Neng is cringe!");
            ASSERT_EQ(result, "Neng is cringe!");

            auto result2 = trim_string_end("Tony is based!        ");
            ASSERT_EQ(result2, "Tony is based!");

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
                .type = ParagraphType::HEADER,
                .content = "Hello!",
                .header_level = 1,
            });
            document.paragraphs.push_back({
                .type = ParagraphType::NORMAL,
                .content = "This is a test!",
            });

            const auto result = templ.render_html_to_string(document);
            ASSERT_EQ(
                result,
                R"html(<body>    <nav>The end of the world is upon us!</nav><h1 class="header1">Hello!</h1><p class="paragraph"/>This is a test!</p>    <small>Yes, indeed!</small></body>)html");

            SUCCESS;
        });

    run_test(
        "testing if lines are titles", TEST {
            ASSERT(neng::is_line_title("# Hello!"));
            ASSERT(neng::is_line_title("## Yes!"));
            ASSERT(!neng::is_line_title("Bozo!"));
            ASSERT(!neng::is_line_title("#Cringe"));

            SUCCESS;
        });

    run_test(
        "parsing markdown documents", TEST {
            const auto [document, error] =
                Document::parse_document_from_file("tests/basic.md");
            ASSERT_EQ(error, Error::OK);

            ASSERT_EQ(document.paragraphs[0].content, "Hello");
            ASSERT_EQ(document.paragraphs[0].type, ParagraphType::HEADER);
            ASSERT_EQ(document.paragraphs[1].content,
                      "This is a basic test! Welcome to my show!");
            ASSERT_EQ(document.paragraphs[1].type, ParagraphType::NORMAL);
            ASSERT_EQ(document.paragraphs[2].content,
                      "Neng Li is the President of China!");
            ASSERT_EQ(document.paragraphs[2].type, ParagraphType::NORMAL);

            SUCCESS;
        });

    run_test("obtaining header levels", TEST { 
        ASSERT_EQ(count_title_level("## Hello!"), 2);
        ASSERT_EQ(count_title_level("Yes, indeed!"), 0);
        ASSERT_EQ(count_title_level("# Yes!"), 1);
        ASSERT_EQ(count_title_level("### Neng Li"), 3);
        SUCCESS; 
    });
}
} // namespace neng
