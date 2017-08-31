set(SCENE_HEADERS resource.h
                  obj_reader.h
                  compiler/compiler.h
                  compiler/bvh_builder.h
                  compiler/input_scene_types.h
                  material/bxdf.h
                  material/node.h)

set(SCENE_SOURCES resource.cpp
                  obj_reader.cpp
                  compiler/compiler.cpp)

add_library(eclipse_scene ${SCENE_SOURCES} ${SCENE_HEADERS})
target_link_libraries(eclipse_scene eclipse_math eclipse_util)
