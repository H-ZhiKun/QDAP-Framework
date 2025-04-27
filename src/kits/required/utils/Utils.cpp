#include "Utils.h"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <filesystem>
#include "kits/required/log/CRossLogger.h"
using namespace _Kits;
std::vector<std::string> Utils::split(const std::string &str,
                                      const std::string &delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
    {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    tokens.push_back(str.substr(start));
    return tokens;
}

int Utils::encryptAES(const unsigned char *plaintext,
                      int plaintext_len,
                      const unsigned char *key,
                      unsigned char *iv,
                      unsigned char *ciphertext)
{
    // EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    // if (ctx == nullptr)
    // {
    //     return -1;
    // }

    // // 初始化加密上下文
    // if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv))
    // {
    //     return -1;
    // }

    // int len;
    // int ciphertext_len;

    // // 执行加密
    // if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    // {
    //     EVP_CIPHER_CTX_free(ctx);
    //     return -1;
    // }
    // ciphertext_len = len;

    // // 完成加密
    // if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    // {
    //     EVP_CIPHER_CTX_free(ctx);
    //     return -1;
    // }
    // ciphertext_len += len;

    // EVP_CIPHER_CTX_free(ctx);
    // return ciphertext_len;
    return 0;
}

int Utils::decryptAES(const unsigned char *ciphertext,
                      int ciphertext_len,
                      const unsigned char *key,
                      unsigned char *iv,
                      unsigned char *plaintext)
{
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     if (ctx == nullptr)
//     {
//         return -1;
//     }

//     // 初始化解密上下文
//     if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv))
//     {
//         EVP_CIPHER_CTX_free(ctx);
//         return -1;
//     }

//     int len;
//     int plaintext_len;

//     // 执行解密
//     if (1 !=
//         EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
//     {
//         EVP_CIPHER_CTX_free(ctx);
//         return -1;
//     }
//     plaintext_len = len;

//     // 完成解密
//     if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
//     {
//         EVP_CIPHER_CTX_free(ctx);
//         return -1;
//     }
//     plaintext_len += len;

//     EVP_CIPHER_CTX_free(ctx);
//     return plaintext_len;
    return 0;
 }

 bool _Kits::Utils::readConfigByPath(const YAML::Node &originconfig,
                                     YAML::Node &config,
                                     const std::string &path)
 {
    auto str_config_path = originconfig[path].as<std::string>();

    if (str_config_path.empty())
    {
        return false;
    }

    try
    {
        config = YAML::LoadFile(
            std::filesystem::current_path().string() + str_config_path);
    }
    catch(const std::exception& e)
    {
        LogError("Failed to load config file: {%s}", e.what());
        
        return false;
    }
    
    return true;
 }
