// Copyright 2016-2020 SICK AG. All rights reserved.

#include "ConfigWriter.h"

#include "../public/Exceptions.h"
#include "GenICam.h"
#include "GenIUtil.h"
#include "NodeUtil.h"

#include <algorithm>

using namespace GenApi;
using namespace GenICam;

namespace geniranger {

ConfigWriter::ConfigWriter(std::ostream& outputStream)
  : mOutputStream(outputStream)
  , mIndent(0)
{
  mOutputStream << "#Version,1" << std::endl;
}

void ConfigWriter::outputCategory(const std::string& name)
{
  if (name.empty())
  {
    return;
  }

  // Output category
  geniutil::log(std::string(mIndent, ' ').append("#" + name + '\n'));
  mOutputStream << "#" << name << std::endl;
}

static bool compare(CNodePtr lh, CNodePtr rh)
{
  return lh->GetName() < rh->GetName();
}

void ConfigWriter::outputLeaf(const std::string& name, const std::string& value)
{
  if (name.empty() || value.empty())
  {
    return;
  }

  // Output node name
  geniutil::log(std::string(mIndent, ' ').append(name));
  mOutputStream << name;

  // If the value is dependent on selectors, output the selectors and their
  // values. This will be used as the key value when importing the node value.
  if (mSelectors.size() > 0)
  {
    std::stringstream ss;
    // Copy selector list so that it can be sorted.
    // The original is treated like a stack and should not be altered.
    auto sorted = mSelectors;
    std::sort(sorted.begin(), sorted.end(), compare);
    for (auto it = sorted.begin(); it != sorted.end(); it++)
    {
      auto selector = *it;
      auto selectorName = selector->GetName();
      try
      {
        ss << "_";
        if (nodeutil::isInteger(selector))
        {
          CIntegerPtr intSelector = static_cast<CIntegerPtr>(selector);
          ss << selectorName << "_" << intSelector->GetValue();
        }
        else if (nodeutil::isEnumeration(selector))
        {
          CEnumerationPtr enumSelector = static_cast<CEnumerationPtr>(selector);
          ss << selectorName << "_"
             << enumSelector->GetCurrentEntry()->GetSymbolic();
        }
        else
        {
          std::stringstream message;
          message << "Unsupported selector: " << selectorName;
          geniutil::throwAndLog(message.str());
        }
      }
      catch (GenericException& e)
      {
        std::stringstream message;
        message << "Cannot read " << selectorName
                << ". Library exception: " << e.GetDescription();
        geniutil::throwAndLog(message.str());
      }
    }
    geniutil::log(ss.str());
    mOutputStream << ss.str();
  }
  // Lastly output the node value
  geniutil::log("," + value + '\n');
  mOutputStream << "," << value << '\n';
}

void ConfigWriter::indent()
{
  mIndent = mIndent + 2;
}

void ConfigWriter::unindent()
{
  mIndent = mIndent - 2;
  // TODO test that this shouldn't happen
  mIndent = mIndent < 0 ? 0 : mIndent;
}

void ConfigWriter::pushCategory()
{
  indent();
}

void ConfigWriter::popCategory()
{
  unindent();
}

void ConfigWriter::pushSelector(const CNodePtr& node)
{
  mSelectors.push_back(node);
}

void ConfigWriter::popSelector()
{
  mSelectors.pop_back();
}

////////////////////////////////////////////////////////////////

#pragma region VisionLab_Modified

ConfigWriterVLB::ConfigWriterVLB(std::ostream& outputStream)
	: mOutputStream(outputStream)
	, mIndent(0)
{
	mOutputStream << "#Version,1" << std::endl;
}

void ConfigWriterVLB::outputCategory(const std::string& name)
{
	if (name.empty())
	{
		return;
	}

	// Output category
	geniutil::log(std::string(mIndent, ' ').append("#" + name + '\n'));
	mOutputStream << "#" << name << std::endl;
}

static bool compareVLB(CNodePtr lh, CNodePtr rh)
{
	return lh->GetName() < rh->GetName();
}

void ConfigWriterVLB::outputLeaf(const std::string& name, const std::string& value, const GenApi::CNodePtr& node)
{
	if (name.empty() || value.empty())
	{
		return;
	}

	// Output node name
	geniutil::log(std::string(mIndent, ' ').append(name));
	mOutputStream << name;

	// Add optional value bu Vision Lab
	// Store all optional values
	StringList_t sysbolics;

	// If the value is dependent on selectors, output the selectors and their
	// values. This will be used as the key value when importing the node value.
	if (mSelectors.size() > 0)
	{
		std::stringstream ss;
		// Copy selector list so that it can be sorted.
		// The original is treated like a stack and should not be altered.
		auto sorted = mSelectors;
		std::sort(sorted.begin(), sorted.end(), compareVLB);
		for (auto it = sorted.begin(); it != sorted.end(); it++)
		{
			auto selector = *it;
			auto selectorName = selector->GetName();
			try
			{
				ss << "_";
				if (nodeutil::isInteger(selector))
				{
					CIntegerPtr intSelector = static_cast<CIntegerPtr>(selector);
					ss << selectorName << "_" << intSelector->GetValue();
				}
				else if (nodeutil::isEnumeration(selector))
				{
					CEnumerationPtr enumSelector = static_cast<CEnumerationPtr>(selector);
					ss << selectorName << "_"
						<< enumSelector->GetCurrentEntry()->GetSymbolic();
				}
				else
				{
					std::stringstream message;
					message << "Unsupported selector: " << selectorName;
					geniutil::throwAndLog(message.str());
				}
			}
			catch (GenericException& e)
			{
				std::stringstream message;
				message << "Cannot read " << selectorName
					<< ". Library exception: " << e.GetDescription();
				geniutil::throwAndLog(message.str());
			}
		}
		geniutil::log(ss.str());
		mOutputStream << ss.str();
	}
	// Lastly output the node value
	geniutil::log("," + value + '\n');

#pragma region VisionLab_Modified
	mOutputStream << "," << value; // do not add '\n' here

	// Add optional values by Vision Lab
	std::stringstream ss;
	if (nodeutil::isEnumeration(node))
	{
		CEnumerationPtr enumSelector = static_cast<CEnumerationPtr>(node);
		enumSelector->GetSymbolics(sysbolics);
		std::vector<std::string> snames;
		for (auto sub : sysbolics)
			ss << sub << " ";
	}
	mOutputStream << "," << ss.str();

	// Add ifReadOnly by Vision Lab
	mOutputStream << (nodeutil::isConfigNode(node) ? ",RW" : ",R");
	mOutputStream << '\n';
#pragma endregion

}

void ConfigWriterVLB::indent()
{
	mIndent = mIndent + 2;
}

void ConfigWriterVLB::unindent()
{
	mIndent = mIndent - 2;
	// TODO test that this shouldn't happen
	mIndent = mIndent < 0 ? 0 : mIndent;
}

void ConfigWriterVLB::pushCategory()
{
	indent();
}

void ConfigWriterVLB::popCategory()
{
	unindent();
}

void ConfigWriterVLB::pushSelector(const CNodePtr& node)
{
	mSelectors.push_back(node);
}

void ConfigWriterVLB::popSelector()
{
	mSelectors.pop_back();
}

#pragma endregion



}
