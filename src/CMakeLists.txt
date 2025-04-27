
# ---- 板块子目录名称列表 BEGIN ----
set(KITS_CONFIGS "")
set(MODS_CONFIGS "")
set(CTLS_CONFIGS "")
# ---- 板块子目录名称列表 END   ----

# ---- 开发版本构建名 BEGIN ----
set(HZK_VERSION "hzk开发版本")
# ---- 开发版本构建名 END   ----

set(CURRENT_VERSION ${HZK_VERSION})

# ---- 匹配发布版本构建 END   ----

# ---- 匹配开发版本构建 BEGIN ----
if(CURRENT_VERSION STREQUAL ${HZK_VERSION})
    list(APPEND KITS_CONFIGS "database" "qml_kits" "camera/baumer")
    list(APPEND MODS_CONFIGS "qml_engine" "dev_info" "camera_lobby")
    list(APPEND CTLS_CONFIGS "unit_test/hzk_ctls")
else()
endif()
# ---- 匹配开发版本构建 END   ----

add_subdirectory(kits)
add_subdirectory(modules)
add_subdirectory(controllers)
add_subdirectory(service)