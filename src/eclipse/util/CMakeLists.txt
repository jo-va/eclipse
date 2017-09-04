set(UTIL_HEADERS logger.h
                 log_message.h
                 stop_watch.h
                 http_downloader.h)

set(UTIL_SOURCES logger.cpp
                 log_message.cpp
                 stop_watch.cpp
                 http_downloader.cpp)

add_library(eclipse_util ${UTIL_SOURCES} ${UTIL_HEADERS})
target_link_libraries(eclipse_util ${CURL_LIBRARIES})
