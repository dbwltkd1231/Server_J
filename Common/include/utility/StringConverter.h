#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <nlohmann/json.hpp> 

namespace Utility
{
    class StringConverter
    {
    public:
        static std::string ConvertEUC_KRtoUTF8(const std::string& eucStr);
        static std::string ConvertUTF8toEUC_KR(const std::string& utf8Str);
        static std::string WstringToUTF8(const std::wstring& wstr);
        static std::string StringConvert(std::wstring ws);
        static  std::wstring ConvertToSQLWCHAR(const std::string& str);
    };

    std::string StringConverter::ConvertEUC_KRtoUTF8(const std::string& eucStr)
    {
        // EUC-KR → WideChar 변환
        int wide_size = MultiByteToWideChar(949, 0, eucStr.c_str(), -1, nullptr, 0);
        if (wide_size == 0)
        {
            // 변환 실패
            return std::string();
        }

        std::wstring wide_str(wide_size, 0);
        MultiByteToWideChar(949, 0, eucStr.c_str(), -1, &wide_str[0], wide_size);

        std::cout << std::endl;

        // WideChar → UTF-8 변환
        int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8_size == 0)
        {
            // 변환 실패
            return std::string();
        }
        std::string utf8_str(utf8_size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &utf8_str[0], utf8_size, nullptr, nullptr);

        return utf8_str;
    }

    std::string StringConverter::ConvertUTF8toEUC_KR(const std::string& utf8Str)
    {
        // UTF-8 → WideChar 변환
        int wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
        std::wstring wide_str(wide_size, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wide_str[0], wide_size);
        // WideChar → EUC-KR 변환
        int euc_kr_size = WideCharToMultiByte(949, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string euc_kr_str(euc_kr_size, 0);
        WideCharToMultiByte(949, 0, wide_str.c_str(), -1, &euc_kr_str[0], euc_kr_size, nullptr, nullptr);

        return euc_kr_str;
    }

    std::string StringConverter::WstringToUTF8(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();

        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
        std::string strTo(sizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], sizeNeeded, NULL, NULL);

        return strTo;
    }

    std::string StringConverter::StringConvert(std::wstring ws)
    {
        std::string result = std::string(ws.begin(), ws.end());;

        return result;
    }

    std::wstring StringConverter::ConvertToSQLWCHAR(const std::string& str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
        return wstr;
    }
}