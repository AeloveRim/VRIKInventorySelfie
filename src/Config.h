#pragma once

#include <string>

namespace VrikInventorySelfie {
	class Config {
	public:
		static Config* GetSingleton() {
			static Config singleton;
			return &singleton;
		}

		void LoadConfig(const std::string& iniPath);

		// Re-reads the ini from the same path last passed to LoadConfig. Used
		// when DebugMode is on so you can edit the ini and see the effect
		// without restarting the game.
		void Reload();

		float GetDistance() const { return distance; }
		float GetHeightOffset() const { return heightOffset; }
		float GetHorizontality() const { return horizontality; }
		float GetRotationOffset() const { return rotationOffset; }
		bool GetVerboseLogging() const { return verboseLogging; }
		bool GetDebugMode() const { return debugMode; }
		int GetUnlockDelayMs() const { return unlockDelayMs; }

		bool GetBodyVisibleInTween() const { return bodyVisibleInTween; }
		bool GetBodyVisibleInInventory() const { return bodyVisibleInInventory; }
		bool GetBodyVisibleInMagic() const { return bodyVisibleInMagic; }
		bool GetBodyVisibleInContainer() const { return bodyVisibleInContainer; }
		bool GetBodyVisibleInBarter() const { return bodyVisibleInBarter; }
		bool GetFixHdtSmpStretching() const { return fixHdtSmpStretching; }

		float distance = 90.0f;

		float heightOffset = 0.0f;

		float horizontality = 0.0f;

		float rotationOffset = 180.0f;

		bool verboseLogging = false;

		bool debugMode = false;

		int unlockDelayMs = 50;

		bool bodyVisibleInTween = true;
		bool bodyVisibleInInventory = true;
		bool bodyVisibleInMagic = false;
		bool bodyVisibleInContainer = false;
		bool bodyVisibleInBarter = false;
		bool fixHdtSmpStretching = true;

		// Path passed to the last LoadConfig() call, remembered so Reload()
		// knows where to re-read from.
		std::string lastLoadedPath;
	};
}
