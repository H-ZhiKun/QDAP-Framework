#pragma once
#include <cstdint>
namespace _Kits
{
class SourceLocation
{
  public:
    constexpr SourceLocation(const char *fileName = __builtin_FILE(),
                             const char *funcName = __builtin_FUNCTION(),
                             uint32_t lineNum = __builtin_LINE()) noexcept
        : _fileName(fileName), _funcName(funcName), _lineNum(lineNum)
    {
    }

    [[nodiscard]] constexpr const char *FileName() const noexcept
    {
        return _fileName;
    }

    [[nodiscard]] constexpr const char *FuncName() const noexcept
    {
        return _funcName;
    }

    [[nodiscard]] constexpr uint32_t LineNum() const noexcept
    {
        return _lineNum;
    }

  private:
    const char *_fileName;
    const char *_funcName;
    const uint32_t _lineNum;
};
} // namespace _Kits