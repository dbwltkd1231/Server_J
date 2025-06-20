#pragma once
#include <string>

#include "../utility/ConfigCreator.h"
#include "../Client/include/ConstValue.h"


//#define SettingMode  


int main()
{

#if defined(SettingMode)
	Utility::CreateClientSettingFiles();
#else

	auto config = Utility::LoadSettingFiles();
	if (config == NULL)
	{
		return 0;
	}

	Client::ConstValue::GetInstance().IP = config["CLIENT"]["IP"].get<std::string>();
	Client::ConstValue::GetInstance().Port = config["CLIENT"]["PORT"];
	Client::ConstValue::GetInstance().TestUID = config["CLIENT"]["CLIENT_TEST_UID"].get<std::string>();
	Client::ConstValue::GetInstance().ClientCount = config["CLIENT"]["TEST_CLIENT_COUNT"];
	Client::ConstValue::GetInstance().ThreadCount = config["CLIENT"]["TEST_THREAD_COUNT"];

	while (true)
	{

	}

#endif

	return 0;
}