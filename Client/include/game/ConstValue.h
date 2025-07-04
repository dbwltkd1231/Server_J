#pragma once
#include <string>
#include <atomic>

namespace Game
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
        std::string IP;
        int AuthServerPort;
        int LobbyServerPort;
        int OverlappedCountMax;
        std::string TestUID = "";
        std::atomic<int> CurrentClinetIndex;
        int TestClientCount = 0;
        int ThreadCount = 0;

    private:
        ConstValue() = default;
        ~ConstValue() = default;

        // 복사/이동 방지
        ConstValue(const ConstValue&) = delete;
        ConstValue& operator=(const ConstValue&) = delete;
    };
}