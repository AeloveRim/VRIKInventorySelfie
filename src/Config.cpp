#include "Config.h"
#include <SKSE/SKSE.h>
#include <Windows.h>
#include <fstream>

namespace VrikInventorySelfie {
	namespace {
		bool ReadBool(const char* section, const char* key, bool defaultValue, const std::string& iniPath) {
			char buffer[256];
			if (GetPrivateProfileStringA(section, key, defaultValue ? "true" : "false", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
				std::string valStr = buffer;
				return (valStr == "true" || valStr == "1" || valStr == "TRUE");
			}
			return defaultValue;
		}
	}

	void Config::LoadConfig(const std::string& iniPath) {
		lastLoadedPath = iniPath;

		std::ifstream file(iniPath);
		if (!file.is_open()) {
			SKSE::log::info("Config file not found at {}, using defaults.", iniPath.c_str());
			return;
		}
		file.close();

		char buffer[256];

		if (GetPrivateProfileStringA("General", "Distance", "90.0", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
			try {
				distance = std::stof(buffer);
			} catch (...) {}
		}

		if (GetPrivateProfileStringA("General", "HeightOffset", "0.0", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
			try {
				heightOffset = std::stof(buffer);
			} catch (...) {}
		}

		if (GetPrivateProfileStringA("General", "Horizontality", "0.0", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
			try {
				horizontality = std::stof(buffer);
			} catch (...) {}
		}

		if (GetPrivateProfileStringA("General", "RotationOffset", "180.0", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
			try {
				rotationOffset = std::stof(buffer);
			} catch (...) {}
		}

		if (GetPrivateProfileStringA("General", "VerboseLogging", "false", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
			std::string valStr = buffer;
			verboseLogging = (valStr == "true" || valStr == "1" || valStr == "TRUE");
		}

		if (GetPrivateProfileStringA("General", "DebugMode", "false", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
			std::string valStr = buffer;
			debugMode = (valStr == "true" || valStr == "1" || valStr == "TRUE");
		}

		if (GetPrivateProfileStringA("General", "UnlockDelayMs", "50", buffer, sizeof(buffer), iniPath.c_str()) > 0) {
			try {
				unlockDelayMs = std::stoi(buffer);
			} catch (...) {}
		}

		bodyVisibleInTween = ReadBool("BodyVisibility", "Tween", true, iniPath);
		bodyVisibleInInventory = ReadBool("BodyVisibility", "Inventory", true, iniPath);
		bodyVisibleInMagic = ReadBool("BodyVisibility", "Magic", false, iniPath);
		bodyVisibleInContainer = ReadBool("BodyVisibility", "Container", false, iniPath);
		bodyVisibleInBarter = ReadBool("BodyVisibility", "Barter", false, iniPath);

		fixHdtSmpStretching = ReadBool("General", "FixHdtSmpStretching", true, iniPath);

		SKSE::log::info("Loaded Config: Distance = {}, HeightOffset = {}, Horizontality = {}, RotationOffset = {}, VerboseLogging = {}, DebugMode = {}, UnlockDelayMs = {}, FixHdtSmpStretching = {}",
			distance, heightOffset, horizontality, rotationOffset, verboseLogging ? 1 : 0, debugMode ? 1 : 0, unlockDelayMs, fixHdtSmpStretching ? 1 : 0);
		SKSE::log::info("Loaded Config: BodyVisibility Tween={} Inventory={} Magic={} Container={} Barter={}",
			bodyVisibleInTween ? 1 : 0, bodyVisibleInInventory ? 1 : 0, bodyVisibleInMagic ? 1 : 0,
			bodyVisibleInContainer ? 1 : 0, bodyVisibleInBarter ? 1 : 0);
	}

	void Config::Reload() {
		if (!lastLoadedPath.empty()) {
			LoadConfig(lastLoadedPath);
		}
	}
}
