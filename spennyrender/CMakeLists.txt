file(GLOB SPENNY_RENDER_SOURCES "src/*.cpp")

add_library(spennyrender STATIC ${SPENNY_RENDER_SOURCES})
target_include_directories(spennyrender PUBLIC include)

target_link_libraries(spennyrender LINK_PRIVATE SDL3::SDL3 glad stb_image assimp m)
