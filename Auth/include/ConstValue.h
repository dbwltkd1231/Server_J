#pragma once
#include <string>

namespace Auth
{
    class ConstValue
    {
    public:
        static ConstValue& GetInstance()
        {
            static ConstValue instance;
            return instance;
        }

        // 설정값들
        std::string SecretKey = "";
        std::string IP = "";
        int ServerPort = 0;
        int RedisPort = 0;
        int SessionCountMax = 0;
        int OverlappedCountMax = 0;
        int ConnectReadyClientCountMax = 0;


    private:
        ConstValue() = default;
        ~ConstValue() = default;

        // 복사/이동 방지
        ConstValue(const ConstValue&) = delete;
        ConstValue& operator=(const ConstValue&) = delete;
    };
}
