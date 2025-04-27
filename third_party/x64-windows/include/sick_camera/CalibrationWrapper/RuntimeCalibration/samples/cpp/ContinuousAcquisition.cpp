// Copyright 2016-2021 SICK AG. All rights reserved.
#include "SickSheetOfLight.h"

#include "CameraSelector.h"
#include "ExampleUtils.h"
#include "genistream/Camera.h"
#include "genistream/CameraDiscovery.h"
#include "genistream/ChunkAdapter.h"
#include "genistream/FrameGrabber.h"
#include "genistream/LineMetadata.h"
#include "genistream/frame/IFrame.h"

#include <algorithm>
#include <fstream>

void usage(const std::string& program)
{
  std::cout << "Usage:" << std::endl
            << program << " <directory to save frames in>" << std::endl;
  common::CameraSelector::printOptionsHelp();
  std::cout << std::endl;
  std::cout << "Path is to the directory where frames should be saved"
            << std::endl
            << "E.g. C:\\MyFrames" << std::endl
            << std::endl;
}

genistream::sheetoflight::ImageSize imageSizeFromCamera(
  const std::shared_ptr < genistream::CameraParameters >& params)
{
  uint16_t aoiWidth = static_cast<uint16_t>(
    params->region(genistream::RegionId::REGION_1)->width);
  uint16_t numberOfLines = static_cast<uint16_t>(
    params->region(genistream::RegionId::SCAN_3D_EXTRACTION_1)->height);
  return genistream::sheetoflight::ImageSize{ aoiWidth,numberOfLines };
}

genistream::sheetoflight::SensorTraits sensorTraitsFromCamera(
  const std::shared_ptr < genistream::CameraParameters >& params)
{
  genistream::sheetoflight::SensorTraits sensorTraits;
  // Get AOI heights, widths and offsets
  sensorTraits.aoiWidth = static_cast<uint16_t>(
    params->region(genistream::RegionId::REGION_1)->width);
  sensorTraits.aoiHeight = static_cast<uint16_t>(
    params->region(genistream::RegionId::REGION_1)->height);
  sensorTraits.xT.origin = static_cast<double>(
    params->region(genistream::RegionId::REGION_1)->offsetX);
  sensorTraits.xT.scale = 1.0;
  double offsetZ = static_cast<double>(
    params->region(genistream::RegionId::REGION_1)->offsetY);
  const double ranger3SubPixeling = 1.0 / 16.0;
  bool rangeAxisReversed = params->
    scan3dExtraction(genistream::Scan3dExtractionId::SCAN_3D_EXTRACTION_1)->
    rangeAxis == genistream::RangeAxis::REVERSE;
  sensorTraits.zT.scale =
    rangeAxisReversed ? -ranger3SubPixeling : ranger3SubPixeling;
  sensorTraits.zT.origin =
    rangeAxisReversed ? offsetZ + sensorTraits.aoiHeight : offsetZ;
  return sensorTraits;
}

/** Create log file for saving frame information */
std::ofstream openLogFile(const std::string& directoryPath)
{
  // Create directories if they do not already exist
  size_t pos = 0;
  do
  {
    pos = directoryPath.find_first_of("\\/", pos + 1);
    CreateDirectory(directoryPath.substr(0, pos).c_str(), nullptr);
  } while (pos != std::string::npos);

  std::ofstream logFile;
  const std::string filename =
    directoryPath + "\\log-" + common::currentDateTime() + ".txt";
  logFile.exceptions(std::ios::failbit | std::ios::badbit);
  logFile.open(filename, std::ios_base::app);
  return logFile;
}


/**
 * Demonstrates how to configure the camera.
 *
 * There are several ways to set parameters in the camera through the GenIStream
 * API. You can either load a user set (a set of parameters stored on the
 * camera), import a file with parameters in CSV format or set them
 * programatically.
 */
void configureCamera(std::shared_ptr<genistream::ICamera> camera,
                     bool shortFramesMode,
                     bool enableScatter,
                     uint16_t aoiOffsetX,
                     uint16_t aoiWidth,
                     uint16_t aoiOffsetY,
                     uint16_t aoiHeight,
                     uint32_t lineCount)
{
  // Load default user set to set camera in a known configuration state
  camera->openUserSet(genistream::UserSetId::DEFAULT)->load();

  // Register streaming will make all changes in one atomic operation
  camera->withRegisterStreaming(
    [=](std::shared_ptr<genistream::CameraParameters> parameters) {
      // Enable chunk metadata
      parameters->chunkModeActive = true;

      // Switch to 3D
      parameters->deviceScanType = genistream::DeviceScanType::LINESCAN_3D;

      // Switch to Continuous Acquisition
      parameters->acquisitionMode = genistream::AcquisitionMode::CONTINUOUS;

      auto frameStartTrigger =
        parameters->trigger(genistream::TriggerId::FRAME_START);
      if (shortFramesMode)
      {
        parameters->acquisitionStopMode =
          genistream::AcquisitionStopMode::IMMEDIATE;
        frameStartTrigger->triggerMode = genistream::TriggerMode::ON;
        frameStartTrigger->triggerActivation =
          genistream::TriggerActivation::LEVEL_HIGH;
      }
      else
      {
        parameters->acquisitionStopMode =
          genistream::AcquisitionStopMode::COMPLETE;
        frameStartTrigger->triggerMode = genistream::TriggerMode::OFF;
      }

      // Set AcquisitionLineRate
      parameters->acquisitionLineRate = 1000;

      // Enable Reflectance in addition to Range data
      auto extractionRegion =
        parameters->region(genistream::RegionId::SCAN_3D_EXTRACTION_1);
      extractionRegion->component(genistream::ComponentId::REFLECTANCE)
        ->componentEnable = true;

      // Enable/disable Scatter, set Mono8 pixel format
      try
      {
        auto scatterComponent =
          extractionRegion->component(genistream::ComponentId::SCATTER);
        scatterComponent->componentEnable = enableScatter;
        scatterComponent->pixelFormat = genistream::PixelFormat::MONO_8;
      }
      catch (GenICam::AccessException&)
      {
        // Log but continue without Scatter if camera doesn't support it
        std::cerr
          << "The firmware of the camera doesn't seem to support Scatter"
          << std::endl;
      }

      // Set the number of lines in frames
      extractionRegion->height = lineCount;

      // Set AOI height and Y offset, i.e., Area Of Interest, the part of the
      // sensor to use
      auto aoiRegion = parameters->region(genistream::RegionId::REGION_1);
      aoiRegion->offsetX = aoiOffsetX;
      aoiRegion->width = aoiWidth;
      aoiRegion->offsetY = aoiOffsetY;
      aoiRegion->height = aoiHeight;

      // Set exposure time for 3D acquisition
      aoiRegion->exposureTime = 25;
    });
}


/**
 * Performs some sanity checks on the frame, to demonstrate how to access some
 * information on the frame.
 */
void checkFrame(std::shared_ptr<genistream::frame::IFrame> frame,
                uint16_t expectedAoiWidth,
                uint16_t expectedAoiHeight,
                uint32_t expectedLineCount)
{
  assert(1 == frame->getRegions().size());
  std::shared_ptr<const genistream::frame::IRegion> region =
    frame->getRegion(genistream::RegionId::SCAN_3D_EXTRACTION_1);

  const genistream::frame::Aoi aoi = region->getAoi();
  assert(expectedAoiWidth == aoi.width);
  assert(expectedAoiHeight == aoi.height);

  const size_t numberOfComponents = region->getComponents().size();
  // Range, reflectance and possibly scatter are expected components
  assert(numberOfComponents == 2 || numberOfComponents == 3);

  std::shared_ptr<const genistream::frame::IComponent> range =
    frame->getRange();
  std::shared_ptr<const genistream::frame::IComponent> reflectance =
    frame->getReflectance();

  assert(expectedLineCount >= range->getDeliveredHeight());
  assert(expectedLineCount >= reflectance->getDeliveredHeight());
  if (region->hasComponent(genistream::ComponentId::SCATTER))
  {
    std::shared_ptr<const genistream::frame::IComponent> scatter =
      frame->getScatter();
    assert(expectedLineCount >= scatter->getDeliveredHeight());
  }
}


/**
 * Prints the range value in the middle of the A/B (X/Y) plane of the 3D image.
 * Demonstrates how to use the Image helper class to access the data from a
 * component.
 *
 * If you want to access the data directly, you can use IComponent::getData().
 * This is probably what you want to do unless your data is calibrated or
 * rectified.
 *
 * You can also access the coordinate system used by the Image class directly
 * with IComponent::getCoordinateSystem() and manually apply transformation of
 * data for each axis using AxisTransform::apply().
 *
 * A complete example of this is available in the C# sample program
 * ExampleGrabFrame, where center of gravity is calculated for a range image.
 */
void printCenterRangeValue(
  std::shared_ptr<const genistream::frame::IComponent> range)
{
  // The copy function handles conversion from packed 12-bit format.
  // If data is known to be 16-bit createReferringImage can be used,
  // avoiding data copy.
  std::shared_ptr<genistream::frame::Image> rangeImage =
    range->createCopiedImage();

  // If the device provides calibrated data, the point will contain a range
  // value that is transformed into world coordinates, i.e., a value in
  // millimeters.
  const genistream::frame::Image::Point center = rangeImage->getPoint(
    rangeImage->getWidth() / 2, rangeImage->getHeight() / 2);
  if (isnan(center.z))
  {
    std::cout << "Center of image range is missing data" << std::endl;
  }
  else
  {
    std::cout << "Center of image range value " << center.z << std::endl;
  }
}

/** Emulate processing of a frame by performing some logging. */
void processFrame(std::shared_ptr<genistream::frame::IFrame> frame,
                  std::ofstream& logFile)
{
  std::shared_ptr<const genistream::frame::IComponent> range =
    frame->getRange();

  std::cout << "Frame height " << range->getDeliveredHeight() << ", width "
            << range->getWidth() << std::endl;

  printCenterRangeValue(range);

  // Log information about the received frame and some chunk metadata
  logFile << *frame;

  // Demonstrate the per frame metadata (also logged via the frame logging)
  logFile << "Frame trigger counter: "
          << frame->getFrameMetadata().frameTriggerCounter << std::endl;

  // Demonstrate line metadata by logging the timestamp of the few first
  // lines only
  const genistream::ConstLineMetadataVectorPtr metadata =
    frame->getLineMetadata();
  const size_t linesToLog = std::min(metadata->size(), static_cast<size_t>(10));
  for (size_t line = 0; line < linesToLog; ++line)
  {
    logFile << "Line timestamp " << line << ": " << metadata->at(line).timestamp
            << std::endl;
  }
}


/** Saves a frame to file */
void saveFrame(const std::string& directoryPath,
               const std::string& frameFileName,
               std::shared_ptr<genistream::frame::IFrame> frame)
{
  std::ostringstream framePath;
  // Append frame ID to frame file name
  framePath << directoryPath << "\\" << frameFileName << "-"
            << frame->getFrameId();

  frame->save(framePath.str());
}


/** Creates a frame grabber and grabs a number of frames */
void grabFrames(std::shared_ptr<genistream::ICamera> camera,
                size_t frameCount,
                bool saveFramesToDisk,
                const std::string& directoryPath,
                const std::string& frameFileName,
                uint16_t aoiWidth,
                uint16_t aoiHeight,
                uint32_t lineCount,
                std::ofstream& logFile)
{
  genistream::sheetoflight::ModelHandle modelHandle;
  bool success = genistream::sheetoflight::createModelFromFilePath(
    "..\\CalibrationResult.json", &modelHandle);
  assert(success);

  std::shared_ptr<genistream::CameraParameters> parameters =
    camera->getCameraParameters();
  genistream::sheetoflight::SensorTraits sensorTraits =
    sensorTraitsFromCamera(parameters);
  genistream::sheetoflight::ImageSize imageSize =
    imageSizeFromCamera(parameters);
  genistream::sheetoflight::WorldRangeTraits worldRangeTraits;
  genistream::sheetoflight::SplatSettings splatSettings;
  const int dataSize = imageSize.width * imageSize.lineCount;
  std::shared_ptr<genistream::RegionParameters> extractionRegion =
    parameters->region(genistream::RegionId::SCAN_3D_EXTRACTION_1);
  int scatterEnabled = extractionRegion->
    component(genistream::ComponentId::SCATTER)->componentEnable;
  genistream::sheetoflight::PixelFormat format2D[2];
  format2D[0] = genistream::sheetoflight::MONO_8;
  genistream::sheetoflight::PixelFormat scatterFormat;
  if (scatterEnabled)
  {
    scatterFormat = extractionRegion->
      component(genistream::ComponentId::SCATTER)->
      pixelFormat == genistream::PixelFormat::MONO_16 ?
      genistream::sheetoflight::PixelFormat::MONO_16 :
      genistream::sheetoflight::PixelFormat::MONO_8;
    format2D[1] = scatterFormat;
  }
  unsigned int nCoRectified = scatterEnabled ? 2 : 1;
  genistream::sheetoflight::CoRectifiedDescription coRectifiedDescription
    = { nCoRectified, format2D };

  success = calculateCalibrationBounds(modelHandle,
                                       sensorTraits,
                                       imageSize,
                                       &worldRangeTraits,
                                       &splatSettings,
                                       0);
  assert(success);
  genistream::sheetoflight::CalibrationFilterHandle calibrationFilterHandle;
  success = genistream::sheetoflight::createCalibrationFilter(
    modelHandle,
    sensorTraits,
    imageSize,
    genistream::sheetoflight::PFNC_COORD_3D_C16,
    &calibrationFilterHandle);
  assert(success);

  float missingDataValue = 0.0;

  genistream::sheetoflight::RectificationFilterHandle rectificationFilterHandle;
  success = genistream::sheetoflight::createRectificationFilter(
    splatSettings,
    genistream::sheetoflight::MEAN,
    worldRangeTraits,
    imageSize,
    coRectifiedDescription,
    missingDataValue,
    &rectificationFilterHandle);
  assert(success);

  // allocate some data for the output containers.
  std::vector<float> rangeOut(dataSize);
  std::vector<float> X(dataSize);
  std::vector<float> Z(dataSize);
  std::vector<unsigned char> reflectanceOut(dataSize);
  std::vector<unsigned short> scatterOutUint16;
  std::vector<unsigned char> scatterOutUint8;
  void* rectifiedImages2D[2];
  rectifiedImages2D[0] = reflectanceOut.data();
  if (scatterEnabled &&
      scatterFormat == genistream::sheetoflight::PixelFormat::MONO_16)
  {
    scatterOutUint16 = std::vector<unsigned short>(dataSize);
    rectifiedImages2D[1] = scatterOutUint16.data();
  }
  else if (scatterEnabled &&
           scatterFormat == genistream::sheetoflight::PixelFormat::MONO_8)
  {
    scatterOutUint8 = std::vector<unsigned char>(dataSize);
    rectifiedImages2D[1] = scatterOutUint8.data();
  }

  // This is needed to calibrate on the borrowed data. The user is responsible
  // for transforming the calibration input data to 16 bit if
  // genistream::PixelFormat::COORD_3D_C12 is used.
  extractionRegion->component(genistream::ComponentId::RANGE)->pixelFormat =
    genistream::PixelFormat::COORD_3D_C16;

  // Setup a sufficient amount of buffers
  const size_t buffersCount = 20;
  std::shared_ptr<genistream::FrameGrabber> grabber =
    camera->createFrameGrabber(buffersCount);

  std::cout << "Grabbing frames..." << std::endl;
  grabber->start();
  for (size_t i = 1; i < frameCount + 1; i++)
  {
    std::cout << "Grabbing frame " << i << "..." << std::endl;

    genistream::FrameGrabber::IFrameWaitResult result =
      grabber->grab(std::chrono::seconds(5));

    if (result.hasTimedOut())
    {
      std::cerr << "Grab timed out!" << std::endl;
      continue;
    }
    std::shared_ptr<genistream::frame::IFrame> frame = result.getOrThrow();

    // If the frame is not complete it will have corrupt image data and/or
    // line metadata
    if (frame->isComplete())
    {
      checkFrame(frame, aoiWidth, aoiHeight, lineCount);
      processFrame(frame, logFile);

      void* inputCoRectified[2] = { reinterpret_cast<void*>(
      frame->getReflectance()->getData()), nullptr };
      if (scatterEnabled)
      {
        std::shared_ptr<const genistream::frame::IComponent> scatter =
          frame->getRegion(genistream::RegionId::REGION_1)->getComponent(
            genistream::ComponentId::SCATTER);
        inputCoRectified[1] = scatter->getData();
      }
      genistream::sheetoflight::applyCalibration(
        calibrationFilterHandle,
        frame->getRange()->getData(),
        X.data(),
        Z.data());
      genistream::sheetoflight::rectify(
        rectificationFilterHandle,
        X.data(),
        Z.data(),
        inputCoRectified,
        rangeOut.data(),
        rectifiedImages2D);
      float sum = 0; //example of accessing rectified range data.
      int count = 0;
      for (int pixelIndex = 0; pixelIndex < dataSize; pixelIndex++) {
        if (rangeOut[pixelIndex] != missingDataValue)
        {
          sum += rangeOut[pixelIndex];
          count += 1;
        }
      }
      if (count == 0)
      {
        std::cout << "Buffer is only missing data" << std::endl;
      }
      else
      {
        std::cout << "Average height value in buffer: "
          << sum / static_cast<float>(count) << std::endl;
      }
    }
    else
    {
      std::cout << "Frame is incomplete" << std::endl;
    }

    if (saveFramesToDisk)
    {
      saveFrame(directoryPath, frameFileName, frame);
    }
  }

  genistream::sheetoflight::destroyAllRectificationFilters();
  genistream::sheetoflight::destroyAllCalibrationFilters();
  genistream::sheetoflight::destroyAllModels();
}


/**
 * This sample demonstrates how to configure and continuously acquire range data
 * frames from the Ranger3. It also logs various information about each frame to
 * a timestamped file in a folder next to the frames.
 *
 * This sample program takes one command line argument: Absolute path to the
 * directory where frames should be saved. This folder must exist!
 *
 * After selecting interface and camera it provides a couple of options:
 * - Number of frames to acquire
 * - To save the frames to disk or just throw them away
 * - Directory to save to Name (acquisition ID will be appended)
 */
int main(int argc, char* argv[])
{
  common::InputWaiter waiter;
  common::CameraSelector selector;

  if (!selector.parseArguments(argc, argv))
  {
    return 1;
  }

  std::string savePath = "continuous_saved_frames";
  if (!common::parseArgumentHelper(
        argc, argv, "directory to save frames in", savePath))
  {
    usage(argv[0]);
    return 1;
  }

  const std::string ctiFile = common::getPathToProducer();
  if (ctiFile.empty())
  {
    return 1;
  }

  std::shared_ptr<genistream::CameraDiscovery> discovery =
    genistream::CameraDiscovery::createFromProducerFile(ctiFile);

  std::shared_ptr<genistream::ICamera> camera =
    selector.tryOpenCamera(discovery);
  if (!camera)
  {
    return 2;
  }
  // Use custom log subscription instead of default to avoid race condition on
  // stdout
  auto subscription = common::subscribeToInterestingLogs(camera);

  const size_t frameCount =
    common::promptInt("Enter the number of frames to acquire");

  const bool shortFramesMode =
    common::promptLine("Enable Frame trigger and Immediate "
                       "AcquisitionStopMode [y/N]")
    == "y";
  const bool enableScatter =
    common::promptLine("Enable Scatter component? [y/N]") == "y";
  const bool saveToDisk =
    common::promptLine("Save frames to disk? [y/N]") == "y";

  std::string frameFileName;
  if (saveToDisk)
  {
    frameFileName = common::promptLine("Frame file name", false);
  }

  // Increase the heartbeat timeout for the camera when debugging
  if (IsDebuggerPresent())
  {
    const uint64_t heartbeatTimeoutSeconds = 120;
    camera->setHeartbeatTimeout(std::chrono::seconds(heartbeatTimeoutSeconds));
  }

  const uint32_t lineCount = 500;
  const uint16_t aoiOffsetX = 0;
  const uint16_t aoiWidth = 2560;
  const uint16_t aoiOffsetY = 200;
  const uint16_t aoiHeight = 200;

  configureCamera(camera,
                  shortFramesMode,
                  enableScatter,
                  aoiOffsetX,
                  aoiWidth,
                  aoiOffsetY,
                  aoiHeight,
                  lineCount);

  std::ofstream logFile = openLogFile(savePath + "\\acquisition_log");

  // Register callback to log if device is disconnected
  camera->subscribeToDisconnectEvent([&logFile](const gentlcpp::DeviceId&) {
    logFile << "Connection lost" << std::endl;
  });

  try
  {
    grabFrames(camera,
               frameCount,
               saveToDisk,
               savePath,
               frameFileName,
               aoiWidth,
               aoiHeight,
               lineCount,
               logFile);
    if (saveToDisk)
    {
      std::cout << "Saved frames to directory " << savePath << std::endl;
    }

    logFile << std::endl << "Image acquisition stopped" << std::endl;
    common::logDataStreamStatistics(camera, logFile);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  logFile.close();

  return 0;
}
