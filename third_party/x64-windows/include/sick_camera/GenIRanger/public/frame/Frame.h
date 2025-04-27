// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include "../../public/Avoid.h"
#include "../../public/StreamData.h"
#include "../../public/frame/Aoi.h"
#include "Region.h"

namespace gentlcpp {
class Buffer;
class DataStream;
}
namespace geniranger {
class ConnectedDevice;
class FrameGrabber;
class NodeMap;
}

namespace geniranger { namespace frame {

/**
 * Configuration info regarding a specific region that is cached in the Frame
 * class to avoid having to fetch the configuration from the device during
 * acquisition.
 */
struct RegionConfigurationInfo
{
  Aoi aoi;
  RangeAxis rangeAxis;
};

/**
 * Complete configuration info that is cached in an \ref IFrame to avoid having
 * to fetch the configuration from the device during acquisition.
 */
struct FrameConfigurationInfo
{
  DeviceScanType deviceScanType;
  std::map<RegionId, RegionConfigurationInfo> regions;
};

/**
 * This is an interface for classes that provide an abstraction on top of GenTL
 * buffers. Instead of working with buffers and parts it is easier to work with
 * region and component concepts, since that is what is configured in the
 * camera.
 *
 * Each buffer part contains data corresponding to a specific \ref Component in
 * a specific \ref Region.
 */
class IFrame
{
public:
  virtual ~IFrame() = default;

  /** \return the \ref Region%s within the frame */
  virtual ConstRegionList getRegions() const = 0;

  /**
   * \return true if the frame contains a \ref Region with the given \ref
   *         RegionId
   */
  virtual bool hasRegion(RegionId regionId) const = 0;

  /**
   * \return the object for a specific RegionId
   * \throw std::invalid_argument if the RegionId is not available
   */
  virtual std::shared_ptr<const Region> getRegion(RegionId regionId) const = 0;

  /** \return the chunk metadata per scanned line */
  virtual std::shared_ptr<const LineMetadata> getLineMetadata() const = 0;

  /** \return the identification number of the frame */
  virtual uint64_t getFrameId() const = 0;

  /** \return true if the frame was not fully received */
  virtual bool isIncomplete() const = 0;

  /**
   * Saves this frame into ICon file format.
   *
   * Range data is automatically converted to 16-bit.
   *
   * TODO: RNX-4506 Implement support for saving multiple regions
   */
  virtual void save(const std::string& filePath) = 0;

  /**
   * \return the underlying \ref gentlcpp::Buffer if the frame was created from
   *         a buffer
   * \throw NotAvailableException if the frame was created some other way
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID virtual std::shared_ptr<gentlcpp::Buffer> getBuffer() = 0;

protected:
  friend std::ostream& operator<<(std::ostream&, const IFrame&);

  virtual void logTo(std::ostream& s) const = 0;
};

/**
 * Logs information about a \ref Frame and the \ref Region%s and \ref
 * Component%s within it.
 */
inline std::ostream& operator<<(std::ostream& s, const IFrame& frame)
{
  frame.logTo(s);
  return s;
}

/**
 * Creates an \ref IFrame object wrapping a GenTL buffer. When the object is
 * destroyed, i.e., there are no more shared_ptr references to it, the buffer
 * will be queued again via the \ref FrameGrabber.
 */
std::shared_ptr<IFrame> fromBuffer(std::shared_ptr<gentlcpp::Buffer> buffer,
                                   NodeMap deviceNodeMap,
                                   const FrameConfigurationInfo& frameInfo,
                                   std::shared_ptr<FrameGrabber> grabber);

/**
 * Creates an \ref IFrame object wrapping a GenTL buffer. When the object is
 * destroyed, i.e., there are no more shared_ptr references to it, the buffer
 * will be queued again via the \ref gentlcpp::DataStream.
 */
std::shared_ptr<IFrame>
fromBuffer(std::shared_ptr<gentlcpp::Buffer> buffer,
           NodeMap deviceNodeMap,
           const FrameConfigurationInfo& frameInfo,
           std::shared_ptr<gentlcpp::DataStream> dataStream);

/**
 * Creates a \ref FrameConfigurationInfo object that can be reused for each \ref
 * IFrame created with \ref fromBuffer as long as parameters are not changed on
 * the device.
 *
 * The purpose is to cache the configuration to avoid asking the device for the
 * interesting parameter settings each time a GenTL buffer is accessed via the
 * \ref IFrame interface.
 */
FrameConfigurationInfo fromDeviceConfiguration(NodeMap device);

}}
