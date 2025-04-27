# Copyright 2023 SICK AG. All rights reserved.
# Build definitions for Ranger GenICam helper library
project(ApiDll)
cmake_minimum_required(VERSION 2.8)

# List all sources files
set(SOURCE
        CameraShared.h
        CameraShared.cpp
        CustomerLog.cpp
        CustomerLog.h
        DeviceConnection.cpp
        DeviceConnection.h
        ImageTable.cpp
        ImageTable.h
        Parameters.cpp
        Parameters.h
        Ranger3.cpp
        Ranger3.h
        Typedef.h
        Version.cpp
        Version.h
        tinyxml2/tinyxml2.cpp
        tinyxml2/tinyxml2.h
        )


source_group("public" FILES ${PUBLIC_SOURCE})

# Set path to headers
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${GENAPI_INCLUDE_DIR}
    ${GENIRANGER_INCLUDE_DIR}
    ${SAMPLE_COMMON_INCLUDE_DIR}
)
if(${USE_PC_CALIBRATION} STREQUAL "TRUE")
include_directories(
    ${CALIBRATION_WRAPPER_INCLUDE_DIR}
)
endif()


link_directories(
    ${CMAKE_BINARY_DIR}/Examples/Sample/GenIRanger_linux/GenIRanger
    ${CMAKE_BINARY_DIR}/Examples/Sample/GenIRanger_linux/Common
)
if(${USE_PC_CALIBRATION} STREQUAL "TRUE")
link_directories(
    ${CMAKE_BINARY_DIR}/Examples/Sample/CalibrationWrapper
)
endif()


set(EXECUTABLE_FILE_NAME "${PROJECT_NAME}${BUILD_TYPE_EXTENSION}")

# Create a executable
add_library(${EXECUTABLE_FILE_NAME} SHARED ${SOURCE})
target_link_libraries(
    ${EXECUTABLE_FILE_NAME}
    ${GENAPI_PATH_LIB_libGCBase}
    ${GENAPI_PATH_LIB_libGenApi}
    ${GENAPI_PATH_LIB_libCLProtocol}
    ${GENAPI_PATH_LIB_libFirmwareUpdate}
    ${GENAPI_PATH_LIB_liblog4cpp}
    ${GENAPI_PATH_LIB_libLog}
    ${GENAPI_PATH_LIB_libMathParser}
    ${GENAPI_PATH_LIB_libNodeMapData}
    ${GENAPI_PATH_LIB_libXmlParser}

    libGenIRanger.so
    libCommon.so
)
if(${USE_PC_CALIBRATION} STREQUAL "TRUE")
    target_link_libraries(
        libCalibrationWrapper.so
    )
endif()



add_library(${EXECUTABLE_FILE_NAME}_STATIC STATIC ${SOURCE})
target_link_libraries(
    ${EXECUTABLE_FILE_NAME}
    ${GENAPI_PATH_LIB_libGCBase}
    ${GENAPI_PATH_LIB_libGenApi}
    ${GENAPI_PATH_LIB_libCLProtocol}
    ${GENAPI_PATH_LIB_libFirmwareUpdate}
    ${GENAPI_PATH_LIB_libGenApi}
    ${GENAPI_PATH_LIB_liblog4cpp}
    ${GENAPI_PATH_LIB_libLog}
    ${GENAPI_PATH_LIB_libNodeMapData}
    ${GENAPI_PATH_LIB_libXmlParser}

    libGenIRanger.so
    libCommon.so
)
if(${USE_PC_CALIBRATION} STREQUAL "TRUE")
    target_link_libraries(
        libCalibrationWrapper.so
    )
endif()