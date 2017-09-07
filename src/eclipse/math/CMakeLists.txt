set(MATH_HEADERS math.h
                 vec2.h
                 vec3.h
                 vec4.h
                 mat4.h
                 bbox.h
                 quaternion.h
                 transform.h)

set(MATH_SOURCES math.cpp
                 vec3.cpp
                 mat4.cpp
                 quaternion.cpp
                 transform.cpp)

add_library(eclipse_math ${MATH_SOURCES} ${MATH_HEADERS})
