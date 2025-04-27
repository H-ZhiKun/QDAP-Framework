// Copyright 2016-2020 SICK AG. All rights reserved.

#include "NodeExporter.h"
#include "../public/Exceptions.h"
#include "GenIUtil.h"
#include "NodeUtil.h"

#include <sstream>

using namespace GenApi;
using namespace GenICam;

namespace geniranger {

NodeExporter::NodeExporter(ConfigWriter& formatter)
  : mFormat(formatter)
{
  // Empty
}

NodeExporter::~NodeExporter()
{
  // Empty
}

const std::vector<std::string>& NodeExporter::getErrors() const
{
  return mErrors;
}

void NodeExporter::enterCategory(CCategoryPtr node)
{
  mFormat.outputCategory(std::string(node->GetNode()->GetName()));
  mFormat.pushCategory();
}

void NodeExporter::leaveCategory(CCategoryPtr category)
{
  mFormat.popCategory();
}

void NodeExporter::enterSelector(CNodePtr selector)
{
  mFormat.pushSelector(selector);
}

void NodeExporter::leaveSelector(CNodePtr node)
{
  mFormat.popSelector();
}

void NodeExporter::onLeaf(CNodePtr node)
{
  std::string name(node->GetName().c_str());
  std::stringstream valueStream;
  try
  {
    if (nodeutil::isConfigNode(node))
    {
      valueStream << nodeutil::getValueAsString(node);
    }
    else
    {
      // Do nothing, the other types aren't interesting
    }
    mFormat.outputLeaf(name, valueStream.str());
  }
  catch (GenericException& e)
  {
    std::stringstream ss;
    ss << "Cannot read " << name
       << ". Library exception: " << e.GetDescription();
    mErrors.push_back(ss.str());
  }
  catch (std::exception& e)
  {
    std::stringstream ss;
    ss << "Cannot read " << name << ". " << e.what();
    mErrors.push_back(ss.str());
  }
}

////////////////////////////////////////////////////////////////

#pragma region VisionLab_Modified

NodeExporterVLB::NodeExporterVLB(ConfigWriterVLB& formatter)
	: mFormat(formatter)
{
	// Empty
}

NodeExporterVLB::~NodeExporterVLB()
{
	// Empty
}

const std::vector<std::string>& NodeExporterVLB::getErrors() const
{
	return mErrors;
}

void NodeExporterVLB::enterCategory(CCategoryPtr node)
{
	mFormat.outputCategory(std::string(node->GetNode()->GetName()));
	mFormat.pushCategory();
}

void NodeExporterVLB::leaveCategory(CCategoryPtr category)
{
	mFormat.popCategory();
}

void NodeExporterVLB::enterSelector(CNodePtr selector)
{
	mFormat.pushSelector(selector);
}

void NodeExporterVLB::leaveSelector(CNodePtr node)
{
	mFormat.popSelector();
}

void NodeExporterVLB::onLeaf(CNodePtr node)
{
	std::string name(node->GetName().c_str());
	std::stringstream valueStream;
	try
	{
		if (nodeutil::isConfigNode(node))
		{
			valueStream << nodeutil::getValueAsString(node);
		}
		else
		{
			// Do nothing, the other types aren't interesting
		}
		mFormat.outputLeaf(name, valueStream.str(), node); //// Modified by VLB, pass node to function.
	}
	catch (GenericException& e)
	{
		std::stringstream ss;
		ss << "Cannot read " << name
			<< ". Library exception: " << e.GetDescription();
		mErrors.push_back(ss.str());
	}
	catch (std::exception& e)
	{
		std::stringstream ss;
		ss << "Cannot read " << name << ". " << e.what();
		mErrors.push_back(ss.str());
	}
}

#pragma endregion



}
