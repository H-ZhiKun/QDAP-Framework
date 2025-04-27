#pragma once
#include <QMetaEnum>
#include <QString>
#include <string>
#include <vector>
#include <YAML-cpp/yaml.h>

namespace _Kits
{
class Utils
{
  public:
    /**
     * @brief 将字符串根据指定的分隔符拆分为子字符串的向量。
     *
     * 该函数将输入字符串按照指定的分隔符拆分成多个子字符串，并返回包含这些子字符串的向量。
     *
     * @param str 要拆分的输入字符串。
     * @param delimiter 用作拆分的分隔符字符串。
     * @return std::vector<std::string> 拆分后的子字符串向量。
     *
     * @note
     * 如果在输入字符串中未找到分隔符，则整个字符串将作为向量中的单个元素返回。
     * @note
     * 如果输入字符串以分隔符结尾，则结果向量的最后一个元素将是一个空字符串。
     *
     * @code
     * // 示例用法：
     * std::string text = "apple::banana::cherry";
     * std::string delimiter = "::";
     * auto result = split(text, delimiter);
     * // result = {"apple", "banana", "cherry"};
     * @endcode
     */
    static std::vector<std::string> split(const std::string &str,
                                          const std::string &delimiter);
    static int encryptAES(const unsigned char *plaintext,
                          int plaintext_len,
                          const unsigned char *key,
                          unsigned char *iv,
                          unsigned char *ciphertext);
    static int decryptAES(const unsigned char *ciphertext,
                          int ciphertext_len,
                          const unsigned char *key,
                          unsigned char *iv,
                          unsigned char *plaintext);
    
    /// @brief 读取配置文件中的PATH参数，返回config node
    /// @param originconfig 配置
    /// @param config 读取path返回的配置
    /// @param path 读取的path
    /// @return 
    static bool readConfigByPath(const YAML::Node &originconfig,
                                YAML::Node &config,
                                const std::string &path);
};
} // namespace _Kits