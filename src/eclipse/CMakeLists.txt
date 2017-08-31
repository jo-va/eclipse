add_subdirectory(math)
add_subdirectory(util)
add_subdirectory(scene)
add_subdirectory(tracer)
add_subdirectory(renderer)

add_executable(eclipse main.cpp)
target_link_libraries(eclipse eclipse_math eclipse_util eclipse_scene eclipse_tracer eclipse_renderer)

