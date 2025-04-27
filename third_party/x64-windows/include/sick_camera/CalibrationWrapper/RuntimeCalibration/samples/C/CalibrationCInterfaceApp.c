// Copyright 2019 SICK AG. All rights reserved.

#include "SickSheetOfLight.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Returns read size */
size_t loadData(const char* filename, uint8_t** result)
{
  if (result == NULL)
  {
    return 0;
  }

#if defined(_MSC_VER) && (_MSC_VER >= 1400) && (!defined WINCE)
  FILE* f = 0;
  const errno_t err = fopen_s(&f, filename, "rb");
  if (err)
  {
    return 0;
  }
#else
  FILE* f = fopen(filename, "rb");
#endif

  fseek(f, 0, SEEK_END);
  const size_t size = ftell(f);
  fseek(f, 0, SEEK_SET);
  *result = malloc(size);
  if (*result == NULL)
  {
    fclose(f);
    return 0;
  }
  const size_t readSize = fread(*result, sizeof(char), size, f);
  if (size != readSize)
  {
    free(*result);
    fclose(f);
    return 0;
  }
  fclose(f);
  return size;
}

// Program showcasing how to use the calibration API from C
int main()
{
  // These settings should be parsed from the configuration / camera.
  const int aoiWidth = 2560;
  const int numberOfLines = 1600;
  ModelHandle model;
  const SensorTraits sensorTraits = {.xT.scale = 1.0,
                                     .zT.scale = -1.0f / 16.0f,
                                     .xT.origin = 0,
                                     .zT.origin = 744,
                                     .aoiWidth = aoiWidth,
                                     .aoiHeight = 340};
  const ImageSize imageSize = {aoiWidth, numberOfLines};
  const size_t dataSize = numberOfLines * aoiWidth;
  // Allocate memory for calibrated output
  float* x = calloc(dataSize, sizeof(float));
  float* z = calloc(dataSize, sizeof(float));

  // Read test-buffer from file.
  uint8_t* buffer;
  const size_t size = loadData("../../../../realTests/1.dat", &buffer);
  if (buffer == NULL || size == 0)
  {
    printf("Buffer not found \n");
    free(x);
    free(z);
    return 1;
  }
  // Read test-model from file.
  if (!createModelFromFilePath("../../../../realTests/model0.json", &model))
  {
    printf("Model not found \n");
    free(buffer);
    free(x);
    free(z);
    return 1;
  }

  // Setup worldRangeTraits and get recommended splat settings
  WorldRangeTraits worldRangeTraits;
  SplatSettings splatSettings;
  if (!calculateCalibrationBounds(
        model, sensorTraits, imageSize, &worldRangeTraits, &splatSettings, 0))
  {
    free(buffer);
    free(x);
    free(z);
    return 1;
  }

  // Extract pointers to the reflectance and scatter part of the buffer.
  uint8_t* reflectance = (uint8_t*)(buffer + sizeof(uint16_t) * dataSize);
  uint16_t* scatter =
    (uint16_t*)(buffer + (sizeof(uint16_t) + sizeof(uint8_t)) * dataSize);
  // Construct the void * array of the data that is to be co-rectified.
  void* images2D[2] = {reflectance, scatter};
  // reflectance is 8-bit, and in this case the scatter is 16-bit.
  PixelFormat format2D[2] = {MONO_8, MONO_16};

  // Allocate space for the output of the rectified data
  float* rectifiedRange = calloc(dataSize, sizeof(float));
  uint8_t* rectifiedReflectance = calloc(dataSize, sizeof(uint8_t));
  uint16_t* rectifiedScatter = calloc(dataSize, sizeof(uint16_t));

  // Construct the void* array, and provide it with pre-allocated memory where
  // the rectified data will be placed
  void* rectifiedImages2D[2] = {rectifiedReflectance, rectifiedScatter};
  // Create the description of the co-rectified data, 2 images, and their
  // formats
  const CoRectifiedDescription coRectifiedDescription = {2, format2D};

  // Measure some timings
  const clock_t startTime = clock();

  // Calibrate (x,z now describes a point cloud with implicit y-coordinates)
  CalibrationFilterHandle calibrationFilter;
  if (!createCalibrationFilter(
        model, sensorTraits, imageSize, PFNC_COORD_3D_C16, &calibrationFilter))
  {
    return 1;
  }

  RectificationFilterHandle rectificationFilter;
  if (!createRectificationFilter(splatSettings,
                                 MEAN,
                                 worldRangeTraits,
                                 imageSize,
                                 coRectifiedDescription,
                                 0,
                                 &rectificationFilter))
  {
    return 1;
  }

  // setNumberOfThreads(2);
  int numThreads = getNumberOfThreads();
  printf("Using %d threads \n", numThreads);

  if (!applyCalibration(calibrationFilter, buffer, x, z))
  {
    free(rectifiedRange);
    free(rectifiedReflectance);
    free(rectifiedScatter);
    free(buffer);
    free(x);
    free(z);
    return 1;
  }

  // Apply the rectification
  if (!rectify(
        rectificationFilter, x, z, images2D, rectifiedRange, rectifiedImages2D))
  {
    free(rectifiedRange);
    free(rectifiedReflectance);
    free(rectifiedScatter);
    free(buffer);
    free(x);
    free(z);
    return 1;
  }

  const clock_t endTime = clock();
  printf("Calibration and rectification took: %f",
         ((float)(endTime - startTime) / CLOCKS_PER_SEC));

  // Remember to free up allocated memory
  free(rectifiedRange);
  free(rectifiedReflectance);
  free(rectifiedScatter);
  free(buffer);
  free(x);
  free(z);
  destroyCalibrationFilter(&calibrationFilter);
  destroyRectificationFilter(&rectificationFilter);
  destroyModel(&model);
  return 0;
}
