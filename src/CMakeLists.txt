target_sources(
    processor PRIVATE

    document.cpp
    document.hpp
    main.cpp
    pch.hpp
    string_utils.cpp
    string_utils.hpp
    tests.cpp
    tests.hpp
)

target_precompile_headers(processor PRIVATE pch.hpp)
