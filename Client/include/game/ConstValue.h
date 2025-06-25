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

        // ��������
        std::string IP;
        int AuthServerPort;
        int OverlappedCountMax;
        std::string TestUID = "";
        std::atomic<int> CurrentClinetIndex;
        int TestClientCount = 0;
        int ThreadCount = 0;

    private:
        ConstValue() = default;
        ~ConstValue() = default;

        // ����/�̵� ����
        ConstValue(const ConstValue&) = delete;
        ConstValue& operator=(const ConstValue&) = delete;
    };
}