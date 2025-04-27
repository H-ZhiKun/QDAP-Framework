
set(READ_CSV_KITS_PORJECT_NAME "kit_read_csv")

message("---------- Building ${READ_CSV_KITS_PORJECT_NAME}  ----------")
file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${READ_CSV_KITS_PORJECT_NAME} OBJECT ${SRC_CURRENT})

target_sources(${READ_CSV_KITS_PORJECT_NAME} PUBLIC 
            $<TARGET_OBJECTS:kit_log>
)
target_include_directories(${READ_CSV_KITS_PORJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(${READ_CSV_KITS_PORJECT_NAME} PUBLIC
    shared_dependencies
    kit_log
)