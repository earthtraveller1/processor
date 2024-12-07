target_sources(
    processor PRIVATE

    document.cpp
    document.hpp
    document_template.cpp
    document_template.hpp
    main.cpp
    pch.hpp
    string_utils.cpp
    string_utils.hpp
    tests.cpp
    tests.hpp
)

if (PROCESSOR_BUILD_TESTS)
    target_sources(processor PRIVATE tests.cpp tests.hpp)
endif()

target_precompile_headers(processor PRIVATE pch.hpp)
