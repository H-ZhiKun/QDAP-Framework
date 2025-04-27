// Copyright 2019-2020 SICK AG. All rights reserved.
#include "../public/NodeMap.h"
#include "../public/NodeHelpers.h"
#include "../public/gentlcpp/Port.h"

namespace {

/** Represents a URL to a local file on the PC. */
struct LocalUrl
{
  std::string filename;
  uint64_t address;
  size_t length;

  /**
   * Parses a string of representing the local URL, e.g.,
   * "Local:GevDeviceDescription_037f0f35b.zip;FFA10000;0000A250".
   */
  static LocalUrl fromString(const std::string& url)
  {
    size_t pos = url.find(':');
    std::string withoutPrefix = url.substr(pos + 1);

    std::vector<std::string> tokens;
    std::istringstream ss(withoutPrefix);
    std::string token;
    while (std::getline(ss, token, ';'))
    {
      tokens.push_back(token);
    }

    LocalUrl localUrl;
    localUrl.filename = tokens[0];
    localUrl.address = std::strtoull(tokens[1].c_str(), nullptr, 16);
    localUrl.length =
      static_cast<size_t>(std::strtoull(tokens[2].c_str(), nullptr, 16));
    return localUrl;
  }
};

/** Returns a struct containing the XML name, address and length */
LocalUrl getNodeMapUrl(std::shared_ptr<gentlcpp::Port> port)
{
  return LocalUrl::fromString(
    port->getUrlInfo<std::string>(0, GenTL::URL_INFO_URL));
}

}

namespace geniranger {
 
NodeMap::NodeMap(std::shared_ptr<gentlcpp::Port> port)
: mPort(port)
{
  LocalUrl url = getNodeMapUrl(port);
  bool isXmlZipped = url.filename.find(".zip") != std::string::npos;
  if (isXmlZipped)
  {
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[url.length]);
    port->read(url.address, buffer.get(), url.length);
    mNodeMap._LoadXMLFromZIPData(buffer.get(), url.length);
  }
  else
  {
    std::string xml(url.length, ' ');
    port->read(url.address, &xml.front(), url.length);
    mNodeMap._LoadXMLFromString(xml.c_str());
  }

  std::string name = port->getName();
  mNodeMap._Connect(port->getGenApiPort(), name.c_str());
}

NodeMap::NodeMap(std::shared_ptr<gentlcpp::Port> port, const std::string & name)
	: mPort(port)
{
	LocalUrl url = getNodeMapUrl(mPort);
	bool isXmlZipped = url.filename.find(".zip") != std::string::npos;
	if (isXmlZipped)
	{
		std::unique_ptr<uint8_t[]> buffer(new uint8_t[url.length]);
		mPort->read(url.address, buffer.get(), url.length);
		mNodeMap._LoadXMLFromZIPData(buffer.get(), url.length);
	}
	else
	{
		std::string xml(url.length, ' ');
		mPort->read(url.address, &xml.front(), url.length);
		mNodeMap._LoadXMLFromString(xml.c_str());
	}

	//std::string name = port->getName();
	mNodeMap._Connect(mPort->getGenApiPort(), name.c_str());
}

GenApi::EAccessMode NodeMap::getAccessMode(const std::string& nodeName) const
{
  return geniranger::getAccessMode(mNodeMap, nodeName.c_str());
}

bool NodeMap::isCommand(const std::string& nodeName) const
{
  return geniranger::isCommand(mNodeMap, nodeName.c_str());
}

bool NodeMap::isBool(const std::string& nodeName) const
{
  return geniranger::isBool(mNodeMap, nodeName.c_str());
}

bool NodeMap::isFloat(const std::string& nodeName) const
{
  return geniranger::isFloat(mNodeMap, nodeName.c_str());
}

bool NodeMap::isInt(const std::string& nodeName) const
{
  return geniranger::isInt(mNodeMap, nodeName.c_str());
}

bool NodeMap::isEnum(const std::string& nodeName) const
{
  return geniranger::isEnum(mNodeMap, nodeName.c_str());
}

bool NodeMap::isString(const std::string& nodeName) const
{
  return geniranger::isString(mNodeMap, nodeName.c_str());
}

void NodeMap::executeCommand(const std::string& nodeName,
                             bool waitForCompletion)
{
  geniranger::executeCommand(mNodeMap, nodeName.c_str(), waitForCompletion);
}

bool NodeMap::getBool(const std::string& nodeName) const
{
  return geniranger::getBool(mNodeMap, nodeName.c_str());
}

float NodeMap::getFloat(const std::string& nodeName) const
{
  return geniranger::getFloat(mNodeMap, nodeName.c_str());
}

int64_t NodeMap::getInt(const std::string& nodeName) const
{
  return geniranger::getInt(mNodeMap, nodeName.c_str());
}

std::string NodeMap::getEnum(const std::string& nodeName) const
{
  return std::string(geniranger::getEnum(mNodeMap, nodeName.c_str()));
}

int64_t NodeMap::getEnumInt(const std::string& nodeName) const
{
  return geniranger::getEnumInt(mNodeMap, nodeName.c_str());
}

std::string NodeMap::getString(const std::string& nodeName) const
{
  return std::string(geniranger::getString(mNodeMap, nodeName.c_str()));
}

void NodeMap::setBool(const std::string& nodeName, bool value, bool verify)
{
  geniranger::setBoolExplicitVerification(
    mNodeMap, nodeName.c_str(), value, verify);
}

void NodeMap::setFloat(const std::string& nodeName, float value, bool verify)
{
  geniranger::setFloatExplicitVerification(
    mNodeMap, nodeName.c_str(), value, verify);
}

void NodeMap::setInt(const std::string& nodeName, int64_t value, bool verify)
{
  geniranger::setIntExplicitVerification(
    mNodeMap, nodeName.c_str(), value, verify);
}

void NodeMap::setEnum(const std::string& nodeName, const std::string& value)
{
  geniranger::setEnum(mNodeMap, nodeName.c_str(), value.c_str());
}

void NodeMap::setString(const std::string& nodeName,
                        const std::string& value,
                        bool verify)
{
  geniranger::setStringExplicitVerification(
    mNodeMap, nodeName.c_str(), value.c_str(), verify);
}

float NodeMap::getFloatMin(const std::string& nodeName) const
{
  return geniranger::getFloatMin(mNodeMap, nodeName.c_str());
}

float NodeMap::getFloatMax(const std::string& nodeName) const
{
  return geniranger::getFloatMax(mNodeMap, nodeName.c_str());
}

int64_t NodeMap::getIntMin(const std::string& nodeName) const
{
  return geniranger::getIntMin(mNodeMap, nodeName.c_str());
}

int64_t NodeMap::getIntMax(const std::string& nodeName) const
{
  return geniranger::getIntMax(mNodeMap, nodeName.c_str());
}

NodeMap::NodeMap(GenApi::CNodeMapRef nodeMap)
  : mNodeMap(nodeMap)
{
}

}
