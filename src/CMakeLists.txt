aux_source_directory(. SRC_LIST)
add_library(tinyRpc ${SRC_LIST})
target_link_libraries(tinyRpc pthread muduozdh zookeeper_mt)