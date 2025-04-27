
message("---------- Building kit_kafka  ----------")
file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
find_package(RdKafka CONFIG REQUIRED)
add_library(kit_kafka OBJECT ${SRC_CURRENT})

target_sources(kit_kafka PUBLIC 
$<TARGET_OBJECTS:kit_log>
)  

target_include_directories(kit_kafka PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(kit_kafka PRIVATE
    shared_dependencies
    kit_log
    RdKafka::rdkafka RdKafka::rdkafka++
)
