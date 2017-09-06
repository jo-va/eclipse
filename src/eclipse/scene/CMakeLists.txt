

BISON_TARGET(mat_expr_parser mat_expr_parser.y ${CMAKE_CURRENT_BINARY_DIR}/mat_expr_parser.cxx
             COMPILE_FLAGS --defines=${CMAKE_CURRENT_BINARY_DIR}/mat_expr_parser.hxx)

FLEX_TARGET(mat_expr_scanner mat_expr_scanner.l ${CMAKE_CURRENT_BINARY_DIR}/mat_expr_scanner.cxx
            COMPILE_FLAGS --header-file=${CMAKE_CURRENT_BINARY_DIR}/mat_expr_scanner.hxx)

ADD_FLEX_BISON_DEPENDENCY(mat_expr_scanner mat_expr_parser)

include_directories(${CMAKE_BINARY_DIR}/src)

set(SCENE_HEADERS scene.h
                  resource.h
                  raw_scene.h
                  obj_loader.h
                  material_node.h
                  material_except.h
                  compiler.h
                  bvh_node.h
                  bvh_builder.h
                  known_ior.h
                  mat_expr.h
                  mat_expr_scanner.h)

set(SCENE_SOURCES resource.cpp
                  obj_loader.cpp
                  material_node.cpp
                  compiler.cpp
                  known_ior.cpp
                  mat_expr.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/mat_expr_parser.cxx
                  ${CMAKE_CURRENT_BINARY_DIR}/mat_expr_scanner.cxx)

add_library(eclipse_scene ${SCENE_HEADERS} ${SCENE_SOURCES})
target_link_libraries(eclipse_scene eclipse_math eclipse_util)
