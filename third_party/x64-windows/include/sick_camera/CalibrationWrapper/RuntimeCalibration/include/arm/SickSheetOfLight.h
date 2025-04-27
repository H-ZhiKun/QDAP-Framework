// Copyright 2020 SICK AG. All rights reserved.
/**
 * \file SickSheetOfLight.h
 * \copyright 2020 SICK AG. All rights reserved.
 *
 * Calibration library containing functions to use for calibration and
 * rectification of images captured with a SICK camera.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __linux__
#  define CALIBRATION_API
#else
#  ifdef CALIBRATION_EXPORTS
#    define CALIBRATION_API __declspec(dllexport)
#  else
#    define CALIBRATION_API __declspec(dllimport)
#  endif
#endif

#ifdef __cplusplus
namespace geniranger {
namespace sheetoflight {
extern "C" {
#endif

/** Pointer to an image buffer of some data type. */
typedef void* ImageBuffer;

/** Array of \ref ImageBuffer%s */
typedef ImageBuffer Images2D[];

/**
 * Certain cameras can transfer their data with different pixel format. These
 * are the pixel formats currently supported by the calibration library.
 */
typedef enum
{
  /** Used for 8-bit 2D images */
  MONO_8 = 0x01080001,
  /** Used for 16-bit 2D images */
  MONO_16 = 0x01100007,
  /** Used for 16-bit 3D images */
  PFNC_COORD_3D_C16 = 0x011000B8,
} PixelFormat;

/** Controls how the rectification handles attempts to write the same pixel. */
typedef enum
{
  /** Takes the average of the height values during rectification */
  MEAN,
  /** Takes the highest height value encountered */
  TOP,
  /** Takes the lowest height value encountered */
  BOTTOM,
  /** Takes the height value with the lowest reflectance value */
  DARKEST,
  /** Takes the height value with the highest reflectance value */
  BRIGHTEST
} RectificationMode;

/**
 * Controls the splat settings for the rectification.
 *
 * `k` and `m` can be allowed to be auto-calculated by the function \ref
 * calculateCalibrationBounds(). `k` and `m` control how much the splat setting
 * changes throughout the fov as the fov is normally approximately a trapezoid.
 *
 * \code
 * TotalSplatSize = (k * z + m ) * splat
 * \endcode
 *
 * describes the X,Y maximum distance to a pixel in the rectified buffer a point
 * may have if it is to affect the pixel's value. `TotalSplatSize` is measured
 * in pixels.
 */
typedef struct
{
  double k;
  double m;
  double splat;
} SplatSettings;


/**
 * Describes the layout of the co-rectified data.
 *
 * The co-rectified that could be, e.g., a reflectance or scatter image, which
 * was acquired with the range image.
 */
typedef struct
{
  /** The number of images to be co-rectified */
  unsigned int numberOf2DImages;
  /**
   * An array of \ref PixelFormat%s containing the data type of the co-rectified
   * images
   */
  PixelFormat* imageDataTypes;
} CoRectifiedDescription;

/**
 * An interval of values.
 *
 * The interval describes: `minimum, minimum + step, ... minimum + step x
 * (steps- 1)`, where `step = (maximum - minimum)/(steps - 1)`.
 */
typedef struct
{
  double minimum;
  double maximum;
  int steps;
} Interval;

/**
 * A scaling and translation transformation.
 *
 * Used as
 *
 * \code
 * out = scale * in + origin.
 * \endcode
 */
typedef struct
{
  /** The scale or multiplication factor */
  double scale;
  /** Used for the translation part of the transformation */
  double origin;
} Transform;

/**
 * The bounds of a rectified buffer expressed in intervals.
 *
 * Normally, `xRange` and `zRange` are the interesting values, `yRange` is
 * usually important in the case of outside laserplane alignment.
 */
typedef struct
{
  Interval xRange;
  Interval yRange;
  Interval zRange;
} WorldRangeTraits;

/** The transform of a rectified range buffer. */
typedef struct
{
  Transform xTransform;
  Transform yTransform;
  /**
   * `zTransform` is not needed for float output as the float buffers have the
   * transform applied to them in advance.
   */
  Transform zTransform;
} WorldTransformTraits;

/** Describes how the data is positioned on the sensor. */
typedef struct
{
  /**
   * Describes the translation of the data to sensor position `sensorPositionX =
   * xT.scale * dataX + xT.origin`
   */
  Transform xT;
  /**
   * Describes the translation of the data to sensor position `sensorPositionZ =
   * zT.scale * dataZ + zT.origin`
   */
  Transform zT;
  /** The width of input buffers in pixels */
  uint16_t aoiWidth;
  /** The maximum output of the buffers in sensor positions */
  uint16_t aoiHeight;
} SensorTraits;

/** Describes the size of an image in width and height. */
typedef struct
{
  /**
   * The width of X/Z buffer. Should be equal to
   * [aoi width](\ref SensorTraits.aoiWidth) / [xScale](\ref SensorTraits.xT).
   */
  uint16_t width;
  /** The height of buffers, number of lines or number of profiles */
  uint16_t lineCount;
} ImageSize;

/**
 * A handle to a resource allocated withing the dll. Could be an \ref
 * ModelHandle, \ref CalibrationFilterHandle or \ref RectificationFilterHandle.
 */
typedef uint32_t ResourceHandle;

const ResourceHandle INVALID_RESOURCE_HANDLE = 0;

/**
 * The model handle is a reference to a model created using \ref
 * createModelFromJson() or \ref createModelFromFilePath(). The model contains
 * information on how to transform raw data from the camera into world
 * coordinates.
 */
typedef ResourceHandle ModelHandle;


/**
 * The calibration filter handle is a reference to a combination of a model and
 * descriptors of the expected input data. The filter can be created with \ref
 * createCalibrationFilter().
 */
typedef ResourceHandle CalibrationFilterHandle;

/**
 * The rectification filter handle is a reference to a combination of
 * rectification setting and buffer descriptors. The filter can be created with
 * \ref createRectificationFilter().
 */
typedef ResourceHandle RectificationFilterHandle;

/** Log handler signature for replacing the default standard out handler. */
typedef void (*LogMessageHandler)(const char* logMessage);
const LogMessageHandler STDOUT_MESSAGE_REDIRECT = NULL;

/**
 * Maximum amount of models, calibration filters or rectification filters that
 * can be allocated in the library. If more models than this is attempted to be
 * allocated a return indicating error will be returned instead, when a new
 * [resource](\ref ResourceHandle) is attempted to be allocated.
 */
static const size_t MAX_NUM_RESOURCE_HANDLES = 256;

/**
 * Calculates \ref WorldTransformTraits based on \ref WorldRangeTraits.
 *
 * Convenience function to get `xRange` and `yRange` transformations, which are
 * needed during calibration.
 *
 * \param traits where in the world the rectified buffer is
 * \return transform How to transform pixel index to world position. Currently
 *         only float values are supported for zRange Hence `zRange` does not
 *         need to be transformed.
 */
CALIBRATION_API WorldTransformTraits
rangeTraitsToTransform(WorldRangeTraits traits);

/**
 * Warnings and errors are forwarded to standard out by a default log handler.
 * By registering a new log handler callback it is possible to redirect log
 * messages such as error information and warnings.
 *
 * \param handler function called on each generated log message
 */
CALIBRATION_API void registerLogHandler(LogMessageHandler handler);

/**
 * Allocate and create a calibration model from a Json specification and return
 * a reference to it.
 *
 * \param jsonData Json string
 * \param modelHandle pointer to where the output model handle is written. Set
 *        to \ref INVALID_RESOURCE_HANDLE on failure.
 * \return true on success
 */
CALIBRATION_API bool createModelFromJson(const char* jsonData,
                                         ModelHandle* modelHandle);

/**
 * Allocate and create a calibration model from a Json file and return a
 * reference to it.
 *
 * \param filePath a file path to a Json file containing the calibration model
 * \param modelHandle pointer to where the output model handle is written. Set
 *        to \ref INVALID_RESOURCE_HANDLE on failure.
 * \return true on success
 */
CALIBRATION_API bool createModelFromFilePath(const char* filePath,
                                             ModelHandle* modelHandle);

/**
 * Destroy the calibration model that the handle refers to and release its
 * memory.
 *
 * \param modelHandle reference to the model to destroy. Set to \ref
 *        INVALID_RESOURCE_HANDLE on a successful release.
 * \return true when the modelHandle is successfully released
 */
CALIBRATION_API bool destroyModel(ModelHandle* modelHandle);

/**
 * Destroy the calibration filter that the handle refers to and release its
 * memory.
 *
 * \param handle reference to the filter to destroy. Set to \ref
 *        INVALID_RESOURCE_HANDLE on a successful release.
 * \return true when the handle is successfully released
 */
CALIBRATION_API bool destroyCalibrationFilter(CalibrationFilterHandle* handle);

/**
 * Destroy the rectification filter that the handle refers to and release its
 * memory.
 *
 * \param handle reference to the filter to destroy. Set to \ref
 *        INVALID_RESOURCE_HANDLE on a successful release.
 * \return true when the handle is successfully released
 */
CALIBRATION_API bool
destroyRectificationFilter(RectificationFilterHandle* handle);

/** Destroy all calibration models and release their memory. */
CALIBRATION_API void destroyAllModels();

/** Destroy all calibration filters and release their memory. */
CALIBRATION_API void destroyAllCalibrationFilters();

/** Destroy all rectification filters and release their memory. */
CALIBRATION_API void destroyAllRectificationFilters();

/**
 * Calibrates a range buffer by converting it to a point cloud in X and Z. The
 * output consists of two buffers each containing one of the X/Z point pair.
 *
 * \param calibrationFilterHandle handle to a calibration filter
 * \param inputBufferRange Pointer to a buffer containing range data according
 *        to sensorTraits. Should contain `imageSize.height * imageSize.width`
 *        elements of type pixelFormat.
 * \param outputBufferX Pointer to a pre-allocated buffer of size `width *
 *        height * sizeof(float)` where the X outputs shall be written.
 * \param outputBufferZ Pointer to a pre-allocated buffer of size `width *
 *        height * sizeof(float)` where the Z outputs shall be written.
 * \return true when successful
 */
CALIBRATION_API bool
applyCalibration(CalibrationFilterHandle calibrationFilterHandle,
                 uint8_t inputBufferRange[],
                 float outputBufferX[],
                 float outputBufferZ[]);
/**
 * Creates a calibration filter to be used with \ref applyCalibration().
 *
 * \param modelHandle model describing how to transform into world coordinates
 * \param sensorTraits description of the data format from the sensor
 * \param imageSize description of the width and height of the inputBufferRange
 * \param pixelFormat the data type of the inputBufferRange
 * \param calibrationFilterHandle a handle to the created filter
 * \return true if the filter is created successfully
 */
CALIBRATION_API bool
createCalibrationFilter(ModelHandle modelHandle,
                        SensorTraits sensorTraits,
                        ImageSize imageSize,
                        PixelFormat pixelFormat,
                        CalibrationFilterHandle* calibrationFilterHandle);

/**
 * Given a [calibration model](\ref ModelHandle) and
 * [sensor traits](\ref SensorTraits) calculates a bounding box of were the data
 * will end up in the world coordinates.
 *
 * \param modelHandle model describing how to transform into world coordinates
 * \param sensorTraits description of the data format from the sensor
 * \param imageSize description of the width and height of the `inputBuffer`
 * \param outputWorldRangeTraits recommended bounding box for the
 *        maximum/minimum world values
 * \param outputSplatSettings default splat setting according to the model/FOV
 * \return true when successful
 */
CALIBRATION_API
bool calculateCalibrationBounds(ModelHandle modelHandle,
                                SensorTraits sensorTraits,
                                ImageSize imageSize,
                                WorldRangeTraits* outputWorldRangeTraits,
                                SplatSettings* outputSplatSettings);

/**
 * Rectifies a point cloud given by x,y into a heightmap.
 *
 * \param rectificationFilterHandle Handle to the rectification filter. Can be
 *        created with \ref createRectificationFilter().
 * \param inputBufferX a buffer containing the X positions of the calibrated
 *        data
 * \param inputBufferZ a buffer containing the Z positions of the calibrated
 *        data
 * \param inputCoRectified An array of pointers each pointing to the start of a
 *        buffer to be co-rectified. This might, e.g., be an reflectance or
 *        scatter buffer. All buffers should have the same size as the size of
 *        the calibrated data. The size of the array and the data-types of the
 *        buffers are set in the coRectifiedDescription.
 * \param outputRange Pointer to a pre-allocated buffer where to put the
 *        rectified range data. The data type is float and the buffer should
 *        have the size `worldRangeTraits.xRange.steps *
 *        worldRangeTraits.yRange.steps`.
 * \param outputCoRectified An array of pointers each pointing to a
 *        pre-allocated buffer where the co-rectified data should be put. All
 *        buffers are `uint8_t` and should have the same size as the size of the
 *        rectified data.
 * \return true when successful
 */
CALIBRATION_API bool
rectify(RectificationFilterHandle rectificationFilterHandle,
        float inputBufferX[],
        float inputBufferZ[],
        Images2D inputCoRectified,
        float outputRange[],
        Images2D outputCoRectified);

/**
 * Creates a filter for point cloud rectification. The filter can be used from
 * \ref rectify():
 *
 * \param splatSettings splat settings for the rectification
 * \param rectificationMode treatment when multiple Z/co-rectified points end up
 *        on the same pixel in the rectified buffer
 * \param worldRangeTraits where in in the world rectified buffer is located
 * \param imageSize the size of inputBufferX, inputBufferZ and inputCoRectified
 * \param coRectifiedDescription a description of the coRectified data
 * \param missingDataValue value that will be used for missing data, e.g., 0
 * \param rectificationFilterHandle handle to the created rectification filter.
 * \return true when successful
 */
CALIBRATION_API bool
createRectificationFilter(SplatSettings splatSettings,
                          RectificationMode rectificationMode,
                          WorldRangeTraits worldRangeTraits,
                          ImageSize imageSize,
                          CoRectifiedDescription coRectifiedDescription,
                          float missingDataValue,
                          RectificationFilterHandle* rectificationFilterHandle);
/**
 * Writes a calibration model to a Json string.
 *
 * \param modelHandle handle to the calibration model
 * \param jsonData where to write the Json string
 * \param jsonDataSize The size of the jsonData
 * \return false if the model string is larger than jsonDataSize
 */
CALIBRATION_API bool createJsonFromModel(ModelHandle modelHandle,
                                         char* jsonData,
                                         size_t jsonDataSize);

#ifdef __cplusplus
}
}
}
#endif
