# required kits begin
add_subdirectory(required/controller_base)
add_subdirectory(required/log)
add_subdirectory(required/invoke)
add_subdirectory(required/thread_pool)
add_subdirectory(required/module_base)
add_subdirectory(required/utils)
# required kits end
message("===================== KITS BEGIN =====================")
foreach(KITS_DIR ${KITS_CONFIGS})
    add_subdirectory(${KITS_DIR})
endforeach()
message("===================== KITS END   =====================")