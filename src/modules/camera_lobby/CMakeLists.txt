set(MODULE_NAME "mod_camera_lobby")
message("---------- Building ${MODULE_NAME}  ----------")
file(GLOB_RECURSE DEV_INFO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
                                ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${MODULE_NAME} OBJECT ${DEV_INFO_SRC})
target_sources(${MODULE_NAME} PUBLIC 
$<TARGET_OBJECTS:kit_module_base>
)

target_link_libraries(${MODULE_NAME} PUBLIC
kit_module_base
)

# 相机目标如果存在就添加进模块
set(KIT_CAMERA_LIST "kit_camera_baumer" "kit_camera_hik")
foreach(KIT_CAMERA ${KIT_CAMERA_LIST})
    if (TARGET ${KIT_CAMERA})
        target_sources(${MODULE_NAME} PUBLIC 
            $<TARGET_OBJECTS:${KIT_CAMERA}>
        )
        target_link_libraries(${MODULE_NAME} PUBLIC
            ${KIT_CAMERA}
        )
        
    endif()
endforeach()


target_sources(${LIB_MODULES} PUBLIC 
$<TARGET_OBJECTS:${MODULE_NAME}>
)     
target_link_libraries(${LIB_MODULES} PUBLIC ${MODULE_NAME})