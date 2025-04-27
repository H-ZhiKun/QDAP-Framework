// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include "../../public/frame/Frame.h"

namespace geniranger {
class EnumSelectorEntries;
}

namespace geniranger { namespace frame {

/**
 * Representation of a frame that is grabbed via GenTL. The actual data is kept
 * in the GenTL buffer object.
 *
 * Note: This class currently supports only a single region.
 */
class GenTlBackedFrame : public IFrame
{
public:
  GenTlBackedFrame(std::shared_ptr<gentlcpp::Buffer> buffer,
                   const EnumSelectorEntries& regionLookup,
                   const EnumSelectorEntries& componentLookup,
                   std::shared_ptr<LineMetadata> lineMetadata,
                   const FrameConfigurationInfo& frameInfo);

  virtual ConstRegionList getRegions() const override;
  virtual bool hasRegion(RegionId regionId) const override;
  virtual std::shared_ptr<const Region>
  getRegion(RegionId regionId) const override;

  virtual std::shared_ptr<const LineMetadata> getLineMetadata() const override;

  virtual uint64_t getFrameId() const override;
  virtual bool isIncomplete() const override;

  virtual void save(const std::string& filePath) override;

  virtual std::shared_ptr<gentlcpp::Buffer> getBuffer() override;

protected:
  virtual void logTo(std::ostream& s) const override;

private:
  void initMultiPart(std::shared_ptr<gentlcpp::Buffer> buffer,
                     const EnumSelectorEntries& regionLookup,
                     const EnumSelectorEntries& componentLookup,
                     const FrameConfigurationInfo& frameInfo);
  void initSinglePartGray(std::shared_ptr<gentlcpp::Buffer> buffer,
                          const RegionConfigurationInfo& regionInfo);
  void initSinglePart3d(std::shared_ptr<gentlcpp::Buffer> buffer,
                        const RegionConfigurationInfo& regionInfo);

  std::shared_ptr<Region> find(RegionId regionId) const;
  std::shared_ptr<Region> findOrAdd(RegionId regionId,
                                    const RegionConfigurationInfo& regionInfo);

  static void setAoi(RegionPtr& legacyRegion,
                     std::shared_ptr<const Region> region);
  static void setRangeComponent(RegionPtr& legacyRegion,
                                std::shared_ptr<const Region> region);
  static void setReflectanceComponent(RegionPtr& legacyRegion,
                                      std::shared_ptr<const Region> region);
  static void setScatterComponent(RegionPtr& legacyRegion,
                                  std::shared_ptr<const Region> region);

  static RegionPtr
  createLegacyLinescan3dRegion(Frame& frame,
                               std::shared_ptr<const Region> region);

  void saveLinescan3d(const std::string& filePath);

private:
  RegionList mRegions;
  std::shared_ptr<gentlcpp::Buffer> mBuffer;
  std::shared_ptr<LineMetadata> mLineMetadata;

  // To allow implementing the deprecated getBuffer() function
#pragma warning(suppress : 4996)
};
}}
