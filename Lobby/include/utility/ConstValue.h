#pragma once
#include <string>

namespace Lobby
{
	namespace Utility
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
            std::string IP = "";
            int StartPort = 0;
            int StartCount = 0;

            int OverlappedCountMax = 0;
            int BuferSizeMax = 0;
            int SessionCountMax = 0;
            int ClientCapacity = 0;
            int ConnectReadyClientCountMax = 0;


        private:
            ConstValue() = default;
            ~ConstValue() = default;

            // 복사/이동 방지
            ConstValue(const ConstValue&) = delete;
            ConstValue& operator=(const ConstValue&) = delete;
        };
	}
}
/*
    std::string serverIP = config["NETWORK"]["IP"];
    int networkPort = config["NETWORK"]["Lobby_START_PORT"].get<int>();
    int socketOnetimePrepareCount = config["NETWORK"]["Lobby_START_COUNT"].get<int>();

    int clientActivateCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"].get<int>();
    int clientAcceptReadyCountMax = config["NETWORK"]["BUFFER_SIZE_MAX"].get<int>();
    int overlappedCountMax = config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"].get<int>();
    int bufferSizeMax = config["NETWORK"]["CLIENT_COUNT_MAX"].get<int>();

    int redisPort = config["REDIS"]["PORT"].get<int>();
*/