#pragma once

#include <string>
#include <vector>

class Base64 {
public:
    Base64() = delete;

    static std::string Encode(const std::vector<uint8_t>& bytes);

    static std::string Decode(std::string const& encoded_string);

private:
    static const std::string kBase64Chars;

    static bool IsBase64(uint8_t c);
};

