cmake_minimum_required(VERSION 3.16)
project(togee_infinity_station VERSION 2.1 LANGUAGES CXX)
# 全局预配置项
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 标准化架构名称
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(TARGET_ARCH "x64")
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(TARGET_ARCH "arm64")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()
# 检查当前平台
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TARGET_OS "windows")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TARGET_OS "linux")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin") 
    set(TARGET_OS "macos")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# 设置第三方库根目录
set(THIRD_PARTY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/${VCPKG_TARGET_TRIPLET}")
set(THIRD_PARTY_BIN "")
set(THIRD_PARTY_INCLUDE "${THIRD_PARTY_ROOT}/include")
set(THIRD_PARTY_LIB "")
# 动态设置输出目录
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
if(BUILD_TYPE_LOWER STREQUAL "debug")
    set(THIRD_PARTY_BIN "${THIRD_PARTY_ROOT}/debug/bin")
    set(THIRD_PARTY_LIB "${THIRD_PARTY_ROOT}/debug/lib")
    add_compile_definitions(QT_QML_DEBUG=1)
else()
    set(THIRD_PARTY_BIN "${THIRD_PARTY_ROOT}/bin")
    set(THIRD_PARTY_LIB "${THIRD_PARTY_ROOT}/lib")
    add_compile_definitions(QT_QML_DEBUG=0)
endif()

# 添加选项以启用或禁用 AddressSanitizer
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
message(STATUS "AddressSanitizer ${CMAKE_CXX_COMPILER_ID}")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8))
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/utf-8 /Wall)
    else()
        message(WARNING "AddressSanitizer is not supported by the current compiler")
    endif()
endif()


# 添加子文件夹路径到 CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "${THIRD_PARTY_ROOT}/share" ${CMAKE_PREFIX_PATH})
# 设置库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs) # 静态库 .lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs) # 动态库 .dll/.so
# 设置运行目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${VCPKG_TARGET_TRIPLET}-${BUILD_TYPE_LOWER}")
message("=================== Platform Begin ===================")
message("Build Type: ${BUILD_TYPE_LOWER}")
message("Target Platform: ${VCPKG_TARGET_TRIPLET}")
message("Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("=================== Platform End   ===================")



# 添加基础源文件
file(GLOB SRC_GLOBAL ${CMAKE_CURRENT_SOURCE_DIR}/src/tis_global/*.h)
# 统一依赖管理 begin
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Quick Gui Qml QuickControls2 Network Sql Charts)
find_package(spdlog CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(libzippp CONFIG REQUIRED)

add_library(shared_dependencies INTERFACE ${SRC_GLOBAL})
target_include_directories(shared_dependencies INTERFACE ${THIRD_PARTY_INCLUDE} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(shared_dependencies INTERFACE
    Qt6::Core Qt6::Quick Qt6::GuiPrivate Qt6::Qml Qt6::QuickControls2 Qt6::Network Qt6::Sql Qt6::Charts
    spdlog::spdlog
    JsonCpp::JsonCpp
    yaml-cpp::yaml-cpp
    libzippp::libzippp
)

# 统一依赖管理 end

qt_standard_project_setup(REQUIRES 6.8)
# 查找 tis_generator
find_program(TIS_GENERATOR_PATH
    NAMES tis_generator tis_generator.exe  # 支持跨平台名称
    PATHS ${CMAKE_CURRENT_BINARY_DIR}/tools  # 自定义搜索路径
          ${CMAKE_SOURCE_DIR}/tools
          /usr/local/bin  # 其他可能的路径
    NO_DEFAULT_PATH  # 仅在指定路径中查找
)

# 如果未找到 tis_generator，报错
if (NOT TIS_GENERATOR_PATH)
    message(FATAL_ERROR "tis_generator not found! Please build tis_generator first.")
else()
    message(STATUS "Found tis_generator: ${TIS_GENERATOR_PATH}")
    # 1. 先调用程序生成文件
    execute_process(
        COMMAND ${TIS_GENERATOR_PATH} ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE gen_result
        OUTPUT_VARIABLE gen_output
        ERROR_VARIABLE gen_error
    )

    # 检查生成结果是否成功
    if (gen_result EQUAL 0)
        message(STATUS "Files generated successfully.")
    else()
        message(FATAL_ERROR "Failed to generate files: ${gen_error}")
    endif()

endif()


qt_add_executable(${PROJECT_NAME}
    src/main.cpp
)

file(GLOB_RECURSE RES_QML RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/res/qml/*.qml)
file(GLOB_RECURSE RES_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/res/icon/*)

qt_add_qml_module(${PROJECT_NAME}
    URI infinity_station
    VERSION 1.0
    QML_FILES 
        ${RES_QML}
    RESOURCES
        ${RES_FILES}
    RESOURCE_PREFIX "/"
)


set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)


add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    lib_controllers 
    lib_modules 
    lib_service 
)


#添加部署脚本
#为了确保执行正确，所以使用add_custom_target
#在确认生成正确的release工程后
# 终端使用命令启用脚本：
# cmake --build build --target Script-DeployRelease
if (BUILD_TYPE_LOWER MATCHES "release")
    find_program(WIN_DEPLOY_QT6 NAMES windeployqt6)
    message("========== DEPLOY BEGIN ==========")
    message("${WIN_DEPLOY_QT6}")
    message("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    message("========== DEPLOY END   ==========")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/dist
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist
    )
    add_custom_target(Script-DeployRelease
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${WIN_DEPLOY_QT6} --qmldir ${CMAKE_SOURCE_DIR}/res/qml --plugindir ${CMAKE_SOURCE_DIR}/dist/plugins --no-translations --compiler-runtime ${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}.exe
            COMMENT "Windows Deploying Qt Dependencies After Build."
            SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif ()