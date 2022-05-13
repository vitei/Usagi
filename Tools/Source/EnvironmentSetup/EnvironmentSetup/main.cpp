#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <direct.h>
#include <fstream>
#include <filesystem>
#include <assert.h>

struct RegistryEnvironmentPair
{
	const char* RegistryKey;
	const char* RegistryValue;
	const char* EnvironmentValue;
	const char* SubDirectory;
};

int main(int argc, const char* argv[])
{
	printf("Usagi Registry tool. Please be sure to run as administrator\n");
	const int EntryCounts = 2;
	RegistryEnvironmentPair RegistryPairs[EntryCounts] =
	{
		{
			"SOFTWARE\\Microsoft\\MSBuild\\ToolsVersions\\4.0",
			"MSBuildToolsPath",
			"MSBUILD_DIR",
			""
		},
		{
			"SOFTWARE\\Autodesk FBX SDK 2020.3.1",
			"Install_Dir",
			"FBXSDK_DIR",
			""
		},

	};

	char currPath[1024];
	char regValue[256];
	DWORD cbValueLength = sizeof(regValue);

	for (int i = 0; i < EntryCounts; i++)
	{
		HKEY keyHandle = nullptr;
		long n = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegistryPairs[i].RegistryKey, 0, KEY_READ, &keyHandle);
		
		if (n!= ERROR_SUCCESS)
		{
			printf("No registry key %s found, unable to set %s\n", RegistryPairs[i].RegistryKey, RegistryPairs[i].EnvironmentValue);
			continue;
		}
		
		if (RegQueryValueEx(keyHandle, RegistryPairs[i].RegistryValue, NULL, NULL, reinterpret_cast<LPBYTE>(&regValue), &cbValueLength) != ERROR_SUCCESS)
		{
			printf("No registry value %s found, unable to set %s\n", RegistryPairs[i].RegistryValue, RegistryPairs[i].EnvironmentValue);
			continue;
		}
		sprintf_s(currPath, sizeof(currPath), "%s%s", regValue, RegistryPairs[i].SubDirectory);

		n = RegOpenKeyEx(HKEY_CURRENT_USER, "Environment", 0, KEY_ALL_ACCESS, &keyHandle);

		if (n != ERROR_SUCCESS)
		{
			printf("Failed to open environment variables for writing\n");
		}

		if (RegSetValueEx(keyHandle, RegistryPairs[i].EnvironmentValue, 0, REG_SZ, (LPBYTE)currPath, (DWORD)strlen(currPath) + 1) != ERROR_SUCCESS)
		{
			printf("Unable to set enviroment variable %s\n", RegistryPairs[i].EnvironmentValue);
			continue;
		}
		else
		{
			printf("Setting %s to %s\n", RegistryPairs[i].EnvironmentValue, currPath);
		}

	}

	_getcwd(regValue, 256);
	if (!GetCurrentDirectory(sizeof(currPath), currPath))
	{
		printf("Failed to determine current directory, can't set USAGI_DIR\n");
		getchar();
		return -2;
	}

	HKEY keyHandle = nullptr;
	long n = RegOpenKeyEx(HKEY_CURRENT_USER, "Environment", 0, KEY_ALL_ACCESS, &keyHandle);

	if (n != ERROR_SUCCESS)
	{
		printf("Failed to open environment variables for writing\n");
	}

	if (std::filesystem::exists("..\\..\\OculusSDK"))
	{
		std::string s1(currPath);
		s1 = s1.substr(0, s1.find_last_of("\\"));
		s1 = s1.substr(0, s1.find_last_of("\\"));
		s1 += "\\OculusSDK";

		if (RegSetValueEx(keyHandle, "OCULUS_SDK_DIR", 0, REG_SZ, (LPBYTE)s1.c_str(), (DWORD)s1.length() + 1) != ERROR_SUCCESS)
		{
			printf("Failed to set OCULUS_SDK_DIR variable\n");
		}
		else
		{
			printf("OCULUS_SDK_DIR set\n");
		}
	}
	else
	{
		printf("OculusSDK not found in ../../OculusSdk, need to manually set OCULUS_SDK_DIR\n");
	}

	if (std::filesystem::exists("Engine\\ThirdParty\\PhysX\\physx"))
	{
		std::string s1(currPath);
		s1 += "\\Engine\\ThirdParty\\PhysX\\physx";

		if (RegSetValueEx(keyHandle, "PHYSX_DIR", 0, REG_SZ, (LPBYTE)s1.c_str(), (DWORD)s1.length() + 1) != ERROR_SUCCESS)
		{
			printf("Failed to set PHYSX_DIR variable\n");
		}
		else
		{
			printf("PHYSX_DIR set\n");
		}
	}
	else
	{
		printf("PhysX not found in ../../PhysX, need to manually set PHYSX_DIR\n");
	}

	if (RegSetValueEx(keyHandle, "USAGI_DIR", 0, REG_SZ, (LPBYTE)currPath, strlen(currPath) + 1) != ERROR_SUCCESS)
	{
		printf("Unable to set usagi directory");
	}
	else
	{
		printf("Setting USAGI_DIR to %s\n", currPath);
	}

	// Stop the application from quitting
	getchar();

	return 0;
}