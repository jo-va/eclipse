set(TRACER_HEADERS tracer.h)

set(TRACER_SOURCES tracer.cpp)

add_library(eclipse_tracer ${TRACER_SOURCES} ${TRACER_HEADERS})
target_link_libraries(eclipse_tracer ${OpenCL_LIBRARIES})
