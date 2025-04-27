#pragma once
#include "kits/serialization/StructSerialization.h"
#include <JSON/json.h>
#include <cstdint>
#include <string>
#include <vector>

namespace TIS_Info
{
    struct ImageBuffer
    {
        std::string name;
        std::vector<uint8_t> data;
        size_t width = 0;
        size_t height = 0;
        uint64_t timestamp = 0;
        std::string pixFormat;

        ImageBuffer(size_t w, size_t h) : width(w), height(h)
        {
            data.resize(w * h); // 简化，按需调整
        }
        void reset()
        {
            // 无需清理数据，直接覆盖写
        }
    };
    struct TestStruct
    {
        std::string name;
        size_t width = 0;
        size_t height = 0;
        uint64_t timestamp = 0;
    };

} // namespace TIS_Info
STRUCT_SERIALIZATION(TIS_Info::TestStruct, name, width, height, timestamp)