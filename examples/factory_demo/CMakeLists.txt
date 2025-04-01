# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    ../../components
    )

set(EXCLUDE_COMPONENTS
    "lvgl__lvgl/examples" # If you only want to exclude LVGL examples
)

set(COMPONENT_EXCLUDE_PATTERNS 
    "managed_components/*/examples/*"
    "managed_components/*/test_apps/*"
    "managed_components/*/test/*"
)

add_compile_options(-fdiagnostics-color=always
                    -Wno-ignored-qualifiers
                    -Wno-deprecated-declarations
                    -Wno-unused-but-set-variable)

project(factory_demo)
