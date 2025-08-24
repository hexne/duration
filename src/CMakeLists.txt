file(GLOB module_files "*.cppm")

add_library(module STATIC)
target_sources(module
    PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${module_files}
)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    module
)
