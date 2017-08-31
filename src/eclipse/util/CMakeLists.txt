set(UTIL_HEADERS logger.h
                 log_message.h
                 thread_pool.h
                 timer.h
                 mpmc_queue.h
                 thread_pool.h
                 http_downloader.h)

set(UTIL_SOURCES logger.cpp
                 timer.cpp
                 http_downloader.cpp)

add_library(eclipse_util ${UTIL_SOURCES} ${UTIL_HEADERS})
target_link_libraries(eclipse_util ${CURL_LIBRARIES})
