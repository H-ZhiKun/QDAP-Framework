// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

// GenICam.h indirectly includes windows.h. These defines prevents
// including less commonly used functionality.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "CustomCommands.h"
#include "FileOperation.h"
#include "GenICam.h"
#include "GenIRangerDataTypes.h"
#include "GenIRangerDll.h"
#include "StreamData.h"

#ifndef SWIG
#  include <fstream>
#endif

namespace geniranger {
/**
 * Exports all writable parameters as a CSV file with the format
 * "<name>,<value>", one parameter per row. Prefer using \ref
 * ConnectedDevice::exportParameters.
 */
GENIRANGER_API void exportDeviceParameters(GenApi::INodeMap* const nodeMap,
                                           std::ostream& outputCsv);

/**
 * Modified by VisionLab, Shanghai, China. \n
 * \n
 * Exports all writable parameters as a CSV file with the format
 * "<name>,<value>", one parameter per row.
 */
GENIRANGER_API void exportDeviceParametersVLB(	GenApi::INodeMap* const nodeMap,
												std::ostream& outputCsv);

/**
 * Imports parameters from a CSV file. The format is "<name>,<value>", one
 * parameter per row. Prefer using \ref ConnectedDevice::importParameters.
 */
GENIRANGER_API ConfigurationResult
importDeviceParameters(GenApi::INodeMap* const nodeMap, std::istream& inputCsv);

/**
 * Deprecated. Only used for easier wrapping to other languages.
 *
 * \see \ref UserSet::needsMigration()
 */
GENIRANGER_API bool userSetNeedsMigration(GenApi::CNodeMapRef nodeMap,
                                          const std::string& userSetName);

/**
 * Deprecated. Only used for easier wrapping to other languages.
 *
 * \see \ref UserSet::migrate()
 */
GENIRANGER_API ConfigurationResult
migrateUserSet(GenApi::CNodeMapRef nodeMap, const std::string& userSetName);

/**
 * Unpacks a buffer using a 12 bit packed pixel format into a 16 bit pixel
 * format. The 4 MSB will be set to zero.
 *
 * \param inBuffer buffer containing 12p data to be converted
 * \param inSize size of data in 12p data buffer in bytes
 * \param outBuffer buffer which resulting data will be stored
 * \param inOutSize size of the provided buffer in bytes. After unpacking this
 *        parameter holds the number of bytes actually written.
 */
GENIRANGER_API void convert12pTo16(const uint8_t* inBuffer,
                                   const int64_t inSize,
                                   uint8_t* outBuffer,
                                   int64_t* inOutSize);

/**
 * Packs a buffer from a 16 bit to 12 bit packed pixel format using the 12 LSB.
 *
 * \param inBuffer buffer containing 16 bit data to be converted
 * \param inSize size of data in 16 bit data buffer in bytes
 * \param outBuffer buffer which resulting data will be stored
 * \param inOutSize size of the provided buffer in bytes. After packing this
 *        parameter holds the number of bytes actually written.
 */
GENIRANGER_API void convert16To12p(const uint16_t* inBuffer,
                                   const int64_t inSize,
                                   uint8_t* outBuffer,
                                   int64_t* inOutSize);

/**
 * Saves a 16 bit buffer of range data to specified file path.
 *
 * \param buffer buffer containing 16 bit data
 * \param bufferWidth width of the data in the buffer in number of pixels
 * \param bufferHeight height of the data in the buffer in number of pixels
 * \param aoiHeight height of the area of interest used for extracting the
 *        buffer data in number of pixels
 * \param aoiOffsetX offset in X of the data in the buffer in number of pixels
 *        from the image origin to areas of interest
 * \param aoiOffsetY offset in Y of the data in the buffer in number of pixels
 *        from the image origin to area of interest
 * \param rangeAxis the direction of the range axis
 * \param filePath name and location of resulting dat/xml files
 * \param arbitraryXml optional arbitrary xml content to add to xml file
 */
GENIRANGER_API void saveBuffer16(const uint8_t* buffer,
                                 const int64_t bufferWidth,
                                 const int64_t bufferHeight,
                                 const int64_t aoiHeight,
                                 const int64_t aoiOffsetX,
                                 const int64_t aoiOffsetY,
                                 const RangeAxis rangeAxis,
                                 const std::string& filePath,
                                 const std::string& arbitraryXml = "");

/**
 * Saves a 16 bit buffer of range data and possible 8 bit reflectance data
 * and/or scatter data.
 *
 * \param bufferRange16 buffer containing 16 bit data
 * \param bufferReflectance8 buffer containing 8 bit reflectance data, or
 *        nullptr if not present.
 * \param bufferScatter8 buffer containing 8 bit scatter data, or nullptr if not
 *        present
 * \param bufferWidth width of the data in the buffer in number of pixels
 * \param bufferHeight height of the data in the buffer in number of pixels
 * \param aoiHeight height of the area of interest used for extracting the
 *        buffer data in number of pixels
 * \param aoiOffsetX offset in X of the data in the buffer in number of pixels
 *        from the image origin to areas of interest
 * \param aoiOffsetY offset in Y of the data in the buffer in number of pixels
 *        from the image origin to area of interest
 * \param rangeAxis the direction of the range axis
 * \param filePath name and location of resulting dat/xml files
 * \param arbitraryXml optional arbitrary xml content to add to xml file
 */
GENIRANGER_API void saveMultiPartBuffer(const uint8_t* bufferRange16,
                                        const uint8_t* bufferReflectance8,
                                        const uint8_t* bufferScatter8,
                                        const int64_t bufferWidth,
                                        const int64_t bufferHeight,
                                        const int64_t aoiHeight,
                                        const int64_t aoiOffsetX,
                                        const int64_t aoiOffsetY,
                                        const RangeAxis rangeAxis,
                                        const std::string& filePath,
                                        const std::string& arbitraryXml = "");

/**
 * Saves a 8 bit pixel format sensor image buffer to specified file path.
 *
 * \param buffer buffer containing 8 bit data to be saved to disk
 * \param bufferWidth width of the data in the buffer in number of pixels
 * \param bufferHeight height of the data in the buffer in number of pixels
 * \param offsetX offset in X of the data in the buffer in number of pixels from
 *        the image origin to areas of interest
 * \param offsetY offset in Y of the data in the buffer in number of pixels from
 *        the image origin to area of interest
 * \param filePath name and location of resulting dat/xml files
 * \param arbitraryXml optional arbitrary xml content to add to xml file
 */
GENIRANGER_API void saveBuffer8(const uint8_t* buffer,
                                const int64_t bufferWidth,
                                const int64_t bufferHeight,
                                const int64_t offsetX,
                                const int64_t offsetY,
                                const std::string& filePath,
                                const std::string& arbitraryXml = "");


/**
 * Saves a frame to file. Range frame data that can be handled is 16 bit range
 * data with a possible 8 bit reflectance data and/or scatter data and/or
 * additional line meta data. Not complying with these requirements will throw a
 * ValidationException.
 *
 * \param frame frame containing data to save.
 * \param filePath name and location of resulting dat/xml files
 * \param arbitraryXml optional arbitrary xml content to add to xml file
 */
GENIRANGER_API void
saveMultipartRangeFrame(const Frame& frame,
                        const std::string& filePath,
                        const std::string& arbitraryXml = "");
GENIRANGER_API
void saveMultipartRangeFrame(const Frame& frame,
                             std::ostream& dataStream,
                             std::ostream& xmlStream,
                             const std::string& arbitraryXml = "");


/**
 * Saves a float buffer of range data (Calibrated Range Data) to specified file path. \n
 * Use std funtion only!!
 * 
 * Add by VisionLab, Shanghai China. \n 
 *
 * \param buffer Buffer containing: range data (float) | reflectance data (float or byte)
 * \param bufferWidth Width of the data in the buffer in number of pixels
 * \param bufferHeight Height of the data in the buffer in number of pixels
 * \param aoiHeight Height of the area of interest used for extracting the
 * buffer data in number of pixels
 * \param aoiOffsetX Offset in X of the data in the buffer in number of pixels
 * from the image origin to areas of interest
 * \param aoiOffsetY Offset in Y of the data in the buffer in number of pixels
 * from the image origin to area of interest
 * 
 * \param filePath Name and location of resulting dat/xml files
 * \param arbitraryXml Optional arbitrary xml content to add to xml file
*/
GENIRANGER_API 
void saveBufferFloat(	const std::vector<void*> & buffers, 
						const int64_t & bufferWidth, 
						const int64_t & bufferHeight, 
						//const int64_t & aoiHeight, 
						//const int64_t & aoiOffsetX, 
						const float & Lx, 
						const float & Lr, 
						const float & Ux, 
						const float & Ur, 
						const std::string & filePath);


}
