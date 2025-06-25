file(GLOB BASELINE_SRC "src/*.cpp")

add_executable(baseline ${BASELINE_SRC})

add_compile_definitions(BASELINE_RESOURCE_DIR="./baseline/resource/")

target_link_libraries(baseline LINK_PRIVATE spennyrender glad)
