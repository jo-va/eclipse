set(UTIL_HEADERS logger.h
                 log_message.h
                 except.h
                 file_util.h
                 resource.h
                 texture.h
                 stop_watch.h
                 http_downloader.h)

set(UTIL_SOURCES logger.cpp
                 log_message.cpp
                 file_util.cpp
                 resource.cpp
                 texture.cpp
                 stop_watch.cpp
                 http_downloader.cpp)

add_library(eclipse_util ${UTIL_SOURCES} ${UTIL_HEADERS})
target_link_libraries(eclipse_util ${CURL_LIBRARIES})
