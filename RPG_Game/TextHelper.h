#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class TextHelper
{
public:
    // Chuyển std::string UTF-8 sang sf::String (SFML hỗ trợ Unicode)
    static sf::String toSFString(const std::string& utf8String)
    {
        return sf::String::fromUtf8(utf8String.begin(), utf8String.end());
    }

    // Hoặc có thể dùng wide string
    static sf::String toSFStringW(const std::wstring& wideString)
    {
        return sf::String(wideString);
    }
};
