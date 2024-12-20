#include <exception>
#include <sstream>

#include "document.hpp"
#include "document_template.hpp"
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
    try {
        auto result = function();
        if (result.passed) {
            std::cout << "\033[32m Passed \033[0m\n";
        } else {
            std::cout << "\033[91m Failed \n";
            std::cout << "[ERROR]: " << result.error_message << "\033[0m\n";
        }
    } catch (const std::exception& exception) {
        std::cout << "\033[91m Failed \n";
        std::cout << "[ERROR]: " << exception.what() << "\033[0m\n";
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
    std::cout << "[INFO]: Working directory at "
              << std::filesystem::current_path() << '\n';

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
            using neng::DocumentConfiguration;

            const auto [templ, err] =
                DocumentConfiguration::from_file("tests/basic.neng");
            ASSERT_EQ(err, neng::Error::OK);

            ASSERT_EQ(templ.title_class, "title");
            ASSERT_EQ(templ.paragraph_class, "paragraph");

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
            using neng::DocumentConfiguration;
            using neng::Document;

            const auto [templ, err] =
                DocumentConfiguration::from_file("tests/basic.neng");
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
                R"html(<h1 class="title">Hello!</h1><p class="paragraph">This is a test!</p>)html");

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
            ASSERT_EQ(document.paragraphs[0].header_level, 1);
            ASSERT_EQ(document.paragraphs[1].content,
                      "This is a basic test! Welcome to my show!");
            ASSERT_EQ(document.paragraphs[1].type, ParagraphType::NORMAL);
            ASSERT_EQ(document.paragraphs[2].content,
                      "Neng Li is the President of China!");
            ASSERT_EQ(document.paragraphs[2].type, ParagraphType::NORMAL);

            SUCCESS;
        });

    run_test(
        "obtaining header levels", TEST {
            ASSERT_EQ(count_title_level("## Hello!"), 2);
            ASSERT_EQ(count_title_level("Yes, indeed!"), 0);
            ASSERT_EQ(count_title_level("# Yes!"), 1);
            ASSERT_EQ(count_title_level("### Neng Li"), 3);
            SUCCESS;
        });

    run_test(
        "parsing out basic slot templates", TEST {
            using neng::BasicDocumentTemplate;

            const auto [templ, error] =
                BasicDocumentTemplate::from_file("tests/basic-slot.html");

            ASSERT_EQ(templ.before, "Hello, this is a test!");
            ASSERT_EQ(templ.after, "Yes, this is indeed a test!");
            SUCCESS;
        });

    run_test(
        "parsing out basic templates", TEST {
            using neng::DocumentTemplate;
            using neng::TemplateSegment;

            const auto [templ, error] =
                DocumentTemplate::from_file("tests/basic.html");

            ASSERT_EQ(templ.segments.at(0).type, TemplateSegment::Type::TEXT);
            ASSERT_EQ(templ.segments.at(0).a, "Hello! Here is the title: ");
            ASSERT_EQ(templ.segments.at(1).type,
                      TemplateSegment::Type::VARIABLE);
            ASSERT_EQ(templ.segments.at(1).a, "title");
            ASSERT_EQ(templ.segments.at(2).type, TemplateSegment::Type::TEXT);
            ASSERT_EQ(templ.segments.at(2).a, ".\nAnd here is the body: ");
            ASSERT_EQ(templ.segments.at(3).type,
                      TemplateSegment::Type::VARIABLE);
            ASSERT_EQ(templ.segments.at(3).a, "body");
            ASSERT_EQ(templ.segments.at(4).type, TemplateSegment::Type::TEXT);
            ASSERT_EQ(templ.segments.at(4).a, "!\nAmazing, I know.");

            SUCCESS;
        });
}
} // namespace neng
