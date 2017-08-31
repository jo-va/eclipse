set(SCENE_HEADERS resource.h
                  obj_loader.h
                  scene.h
                  raw_scene.h
                  compiler.h
                  bvh_builder.h
                  bxdf.h
                  material_node.h)

set(SCENE_SOURCES resource.cpp
                  obj_loader.cpp
                  compiler.cpp)

add_library(eclipse_scene ${SCENE_SOURCES} ${SCENE_HEADERS})
target_link_libraries(eclipse_scene eclipse_math eclipse_util)
