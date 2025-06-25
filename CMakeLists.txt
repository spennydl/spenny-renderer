cmake_minimum_required(VERSION 3.18)
project(
  spennyrender
  VERSION 0.0.1
  DESCRIPTION "Spenny Demo Renderer"
  LANGUAGES CXX C)

add_subdirectory(vendor)

# Compiler options (inherited by sub-folders)
set(CMAKE_CXX_STANDARD 20)

#add_compile_options(-Wall -Werror)
add_compile_options(-fdiagnostics-color)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(SPENNY_RENDER_DEBUG)
endif()

add_subdirectory(spennyrender)
add_subdirectory(baseline)

target_include_directories(spennyrender PUBLIC include)
target_link_libraries(spennyrender LINK_PRIVATE SDL3::SDL3 glad stb_image assimp m)
