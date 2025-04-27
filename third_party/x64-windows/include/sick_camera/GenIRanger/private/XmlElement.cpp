// Copyright 2019-2020 SICK AG. All rights reserved.

#include "XmlElement.h"
#include <algorithm>
#include <assert.h>
#include <sstream>

namespace {

void insertSpaces(std::ostream& stream, size_t indentSize)
{
  for (size_t i = 0; i < indentSize; ++i)
  {
    stream << " ";
  }
}

}

namespace geniranger {

XmlElement::XmlElement(const std::string& name)
  : mName(name)
{
}

XmlElement& XmlElement::text(const std::string& text)
{
  // Cannot have both text and child elements
  assert(!hasChildren());
  mText = text;
  return *this;
}

const std::string& XmlElement::text() const
{
  return mText;
}

bool XmlElement::hasText() const
{
  return !mText.empty();
}

XmlElement& XmlElement::addChild(const XmlElement& child)
{
  // Cannot have both text and child elements
  assert(mText.empty());
  mChildren.push_back(child);
  return *this;
}

const std::string& XmlElement::attribute(const std::string& name) const
{
  auto it = mAttributes.find(name);
  if (it != mAttributes.end())
  {
    return it->second;
  }
  throw XmlAttributeException(std::string("No such attribute: ") + name);
}

XmlElement& XmlElement::attribute(const std::string& name,
                                  const std::string& value)
{
  mAttributes[name] = value;
  return *this;
}

bool XmlElement::hasAttribute(const std::string& name) const
{
  return mAttributes.find(name) != mAttributes.end();
}

const std::string& XmlElement::name() const
{
  return mName;
}

const std::map<std::string, std::string> XmlElement::attributes() const
{
  return mAttributes;
}

const std::vector<XmlElement>& XmlElement::children() const
{
  return mChildren;
}

bool XmlElement::hasChildren() const
{
  return !mChildren.empty();
}

bool XmlElement::operator==(const XmlElement& other) const
{
  return mName == other.mName && mAttributes == other.mAttributes
         && mChildren == other.mChildren;
}

void XmlElement::writeTo(std::ostream& xml, size_t indent) const
{
  // Opening tag
  insertSpaces(xml, indent);
  xml << "<" << mName;

  for (const auto& entry : mAttributes)
  {
    xml << " " << entry.first << "=\"" << entry.second << "\"";
  }
  xml << ">";

  if (hasChildren())
  {
    // Put children on new line
    xml << "\n";

    for (const auto& child : mChildren)
    {
      child.writeTo(xml, indent + 2);
    }
    // Indent before closing tag
    insertSpaces(xml, indent);
  }
  else if (!mText.empty())
  {
    xml << mText;
  }

  // Closing tag
  xml << "</" << mName << ">\n";
}


std::string XmlElement::str() const
{
  std::stringstream stream;
  this->writeTo(stream);
  return stream.str();
}

bool XmlElement::isAnyChildEqualTo(const XmlElement& other) const
{
  return *this == other
         || std::any_of(
           children().begin(), children().end(), [&](const XmlElement& child) {
             return child.isAnyChildEqualTo(other);
           });
}

bool XmlElement::isAnyChildSuperSetOf(const XmlElement& other) const
{
  return this->isSuperSetOf(other)
         || std::any_of(
           children().begin(), children().end(), [&](const XmlElement& child) {
             return child.isAnyChildSuperSetOf(other);
           });
}

bool XmlElement::isSuperSetOf(const XmlElement& other) const
{
  // Local name should be equal to other name
  if (mName != other.name())
  {
    return false;
  }

  // Local attributes should be a super set of other attributes
  if (!std::includes(mAttributes.begin(),
                     mAttributes.end(),
                     other.mAttributes.begin(),
                     other.mAttributes.end()))
  {
    return false;
  }

  // All of other children should individually and in order match a separate
  // local child
  auto localIt = mChildren.begin();
  auto localEnd = mChildren.end();
  return std::all_of(other.children().begin(),
                     other.children().end(),
                     [&](const XmlElement& otherChild) {
                       // Find local child that can be reduced to other child
                       for (; localIt != localEnd; localIt++)
                       {
                         if (localIt->isSuperSetOf(otherChild))
                         {
                           // Match found - "consume" local child.
                           localIt++;
                           return true;
                         }
                       }

                       // No local children to match against
                       return false;
                     });
}

XmlElement xml(const std::string& name)
{
  return XmlElement(name);
}

std::ostream& operator<<(std::ostream& os, const XmlElement& element)
{
  element.writeTo(os);
  return os;
}

std::string operator+(const std::string& s, const XmlElement& element)
{
  return s + element.str();
}

}
