# Copyright 2016 SICK AG. All rights reserved.
# Build definitions for Ranger GenICam helper library
project(CalibrationWrapper)
cmake_minimum_required(VERSION 2.8)

# List all sources files
set(SOURCE
        CalibrationWrapper.cpp
        CalibrationWrapper.h
        Ranger3RuntimeCalibration/include/CalibrationAPI.h
        Ranger3RuntimeCalibration/include/CalibrationFilter.h
        Ranger3RuntimeCalibration/include/CalibrationParametersFactory.h
        Ranger3RuntimeCalibration/include/Point2D.h
        Ranger3RuntimeCalibration/include/RectificationFilter.h
        Ranger3RuntimeCalibration/include/Settings.h
        Ranger3RuntimeCalibration/include/Transformations.h
        Ranger3RuntimeCalibration/src/CalibrationAPI.cpp
        Ranger3RuntimeCalibration/src/CalibrationFilter.cpp
        Ranger3RuntimeCalibration/src/CalibrationParametersFactory.cpp
        Ranger3RuntimeCalibration/src/Point2D.cpp
        Ranger3RuntimeCalibration/src/RectificationFilter.cpp
        Ranger3RuntimeCalibration/src/Transformations.cpp
        tinyxml2/tinyxml2.cpp
        tinyxml2/tinyxml2.h
        ${SickSheetOfLight_H}
        )


set(OUTPUT_FILE_NAME "${PROJECT_NAME}${BUILD_TYPE_EXTENSION}")


# Create a shared library
add_library(${OUTPUT_FILE_NAME} SHARED ${SOURCE})
target_link_libraries(${OUTPUT_FILE_NAME} ${SickSheetOfLight_SO})

add_library(${OUTPUT_FILE_NAME}_STATIC STATIC ${SOURCE})
target_link_libraries(${OUTPUT_FILE_NAME} ${SickSheetOfLight_SO})
