// Copyright 2019-2020 SICK AG. All rights reserved.

#pragma once

#include "../public/Exceptions.h"
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace geniranger {

/**
 * An XmlElement can have either text contents or other XmlElements as children,
 * but not both at the same time.
 *
 * Many of the "setter" methods return *this to make chaining easy, when
 * constructing XML trees.
 */
class XmlElement
{
public:
  explicit XmlElement(const std::string& name);

  /**
   * Sets textual content of this element.
   *
   * \return this XmlElement for chaining
   */
  XmlElement& text(const std::string& text);

  /**
   * Sets textual content of this element, converted to string
   *
   * \return this XmlElement for chaining
   */
  template<typename T>
  XmlElement& text(T t)
  {
    return text(std::to_string(t));
  }

  template<>
  XmlElement& text(const char* t)
  {
    return text(std::string(t));
  }

  /** Gets textual content of this element. */
  const std::string& text() const;

  /** \return true if this element has textual content. */
  bool hasText() const;

  /**
   * Adds a copy of child element last.
   *
   * \return this XmlElement for chaining
   */
  XmlElement& addChild(const XmlElement& child);

  /**
   * Adds all child elements last
   *
   * \return this XmlElement for chaining
   */
  template<typename... Other>
  XmlElement& addChild(const XmlElement& child, Other... other)
  {
    addChild(child);
    return addChild(other...);
  }

  /**
   * Sets an attribute to the value
   *
   * \return this XmlElement for chaining
   */
  XmlElement& attribute(const std::string& name, const std::string& value);

  /** Sets an attribute to the value, converted to string */
  template<typename T>
  XmlElement& attribute(const std::string& name, T value)
  {
    return attribute(name, std::to_string(value));
  }

  template<>
  XmlElement& attribute(const std::string& name, const char* t)
  {
    return attribute(name, std::string(t));
  }

  /** \return the value of the attribute */
  const std::string& attribute(const std::string& name) const;

  bool hasAttribute(const std::string& name) const;

  /** \return the name of this element, a.k.a xml tag. */
  const std::string& name() const;

  /** \return all attributes of this element */
  const std::map<std::string, std::string> attributes() const;

  /** \return all children of this element */
  const std::vector<XmlElement>& children() const;

  bool hasChildren() const;

  /** Recursively compares this element to another */
  bool operator==(const XmlElement& other) const;

  bool operator!=(const XmlElement& other) const { return !(*this == other); }

  /** Serializes this XmlElement and all children to string representation. */
  void writeTo(std::ostream& xml, size_t indent = 0) const;

  /** Serializes this XmlElement and all children to string representation. */
  std::string str() const;

  /**
   * Traverses this tree and looks for subtree that equals given other tree.
   *
   * \return true if equal subtree is found.
   */
  bool isAnyChildEqualTo(const XmlElement& other) const;

  /**
   * Does the same thing as isSuperSetOf(), but with the addition that it
   * returns true if any element in this tree can be reduced to the other given
   * XML tree.
   */
  bool isAnyChildSuperSetOf(const XmlElement& other) const;

  /**
   * Validates that this XML tree is a super-set of the other given XML tree,
   * meaning that children and attributes can be removed from this tree in order
   * to form the other tree.
   *
   * Give two XML elements X and Y, X can be reduced to Y if all these criteria
   * are meet:
   * - X's name is the same as Y's name
   * - Y's attributes is a subset of X's attributes.
   * - Each of Y's children in order can be reduced to a child of X.
   *
   * \return true if this tree can be reduced to other given tree.
   */
  bool isSuperSetOf(const XmlElement& other) const;

private:
  std::string mName;
  std::map<std::string, std::string> mAttributes;
  std::string mText;
  std::vector<XmlElement> mChildren;
};

/** Short hand for creating an XML element. Useful for building XML trees. */
XmlElement xml(const std::string& name);

/** Idiomatic serialization of XmlElement to string representation */
std::ostream& operator<<(std::ostream& os, const XmlElement& element);

/** Idiomatic string concatenation of XmlElement. */
std::string operator+(const std::string& s, const XmlElement& element);
}
