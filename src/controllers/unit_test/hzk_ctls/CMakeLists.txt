message("---------- Building ctl_hzk  ----------")

file(GLOB_RECURSE CURRENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
                          ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

find_package(Qt6 6.8 REQUIRED COMPONENTS HttpServer)

add_library(ctl_hzk OBJECT ${CURRENT_SRC})
target_sources(ctl_hzk PUBLIC 
$<TARGET_OBJECTS:kit_controller_base>
$<TARGET_OBJECTS:kit_database>
)         
target_link_libraries(ctl_hzk PUBLIC shared_dependencies Qt6::HttpServer kit_controller_base kit_database)

target_sources(${LIB_CONTROLLERS} PUBLIC 
$<TARGET_OBJECTS:ctl_hzk>
)     
target_link_libraries(${LIB_CONTROLLERS} PUBLIC ctl_hzk)
