
set(KIT_NAME "kit_qml_kits")
message("---------- Building ${KIT_NAME}  ----------")
file(GLOB_RECURSE CURRENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${KIT_NAME} OBJECT ${CURRENT_SRC})
target_include_directories(${KIT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(${KIT_NAME} PRIVATE
    shared_dependencies
)
