target_sources(
    processor PRIVATE

    main.cpp
    string_utils.cpp
    tests.cpp

    string_utils.hpp
)

target_precompile_headers(processor PRIVATE pch.hpp)
