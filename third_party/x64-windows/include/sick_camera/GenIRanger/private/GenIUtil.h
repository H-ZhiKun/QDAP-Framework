// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace geniranger { namespace geniutil {

/**
 * Writes the message to std::out if LOG_ENABLED is defined. Does not flush.
 */
void log(std::string message);

/**
 * Throws a GenIRangerException is the provided message and writes it to
 * std::out if LOG_ENABLED is defined. Appends endl and flushes.
 */
void throwAndLog(std::string message);

/** Changes constness of pointed-to objects in a vector. */
template<class T>
inline const std::vector<std::shared_ptr<const T>>
constPointerVector(const std::vector<std::shared_ptr<T>>& v)
{
  std::vector<std::shared_ptr<const T>> result(v.size());
  for (int i = 0; i < v.size(); ++i)
  {
    result[i] = v[i];
  }
  return result;
}

}}
