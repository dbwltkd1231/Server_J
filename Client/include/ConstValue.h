#pragma once
#include <string>

namespace Client
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
        std::string IP = "";
        int Port = 0;
        std::string TestUID = "";
        int ClientCount = 0;
        int ThreadCount = 0;

    private:
        ConstValue() = default;
        ~ConstValue() = default;

        // ����/�̵� ����
        ConstValue(const ConstValue&) = delete;
        ConstValue& operator=(const ConstValue&) = delete;
    };
}