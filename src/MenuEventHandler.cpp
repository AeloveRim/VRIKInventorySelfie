#include "MenuEventHandler.h"
#include "vrikinterface001.h"
#include "Config.h"

#include <cmath>
#include <chrono>
#include <thread>

namespace VrikInventorySelfie {

	static const std::vector<std::string_view> g_pauseMenus = {
		"TweenMenu",
		"InventoryMenu",
		"Journal Menu",
		"ContainerMenu",
		"BarterMenu",
		"MagicMenu",
		"StatsMenu",
		"Crafting Menu",
		"Book Menu",
		"RaceSex Menu",
		"Lockpicking Menu",
		"Sleep/Wait Menu",
		"Quantity Menu",
		"Training Menu",
		"Tutorial Menu"
	};

	void MenuEventHandler::Register() {
		auto ui = RE::UI::GetSingleton();
		if (ui) {
			ui->AddEventSink(GetSingleton());
			SKSE::log::info("Registered MenuOpenCloseEvent sink with UI manager.");
		} else {
			SKSE::log::error("Failed to get UI singleton for menu event registration.");
		}
	}

	bool MenuEventHandler::IsPauseMenu(const std::string_view& menuName) const {
		for (const auto& name : g_pauseMenus) {
			if (name == menuName) {
				return true;
			}
		}
		return false;
	}

	bool MenuEventHandler::IsBodyVisibleMenu(const std::string_view& menuName) const {
		auto config = Config::GetSingleton();
		if (menuName == "TweenMenu") return config->GetBodyVisibleInTween();
		if (menuName == "InventoryMenu") return config->GetBodyVisibleInInventory();
		if (menuName == "MagicMenu") return config->GetBodyVisibleInMagic();
		if (menuName == "ContainerMenu") return config->GetBodyVisibleInContainer();
		if (menuName == "BarterMenu") return config->GetBodyVisibleInBarter();
		return false;
	}

	bool MenuEventHandler::AnyPauseMenuOpen() const {
		return activePauseMenuCount > 0;
	}

	RE::BSEventNotifyControl MenuEventHandler::ProcessEvent(
		const RE::MenuOpenCloseEvent* a_event,
		RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (!a_event) {
			return RE::BSEventNotifyControl::kContinue;
		}

		std::string_view menuName{ a_event->menuName.c_str() };
		auto vrik = vrikPluginApi::getVrikInterface001();
		bool verbose = Config::GetSingleton()->GetVerboseLogging();

		if (verbose) {
			SKSE::log::info("MenuOpenCloseEvent: name='{}' opening={} activePauseMenuCount={} wasLockedForMenu={}",
				menuName, a_event->opening, activePauseMenuCount, wasLockedForMenu);
		}

		if (a_event->opening) {
			if (IsPauseMenu(menuName)) {
				activePauseMenuCount++;

				// Calculate pre-pause transform on first entering a pause menu (e.g., TweenMenu)
				if (!wasLockedForMenu && vrik) {

					double existingLeftArm = vrik->getSettingDouble("enableLeftArm");
					double existingRightArm = vrik->getSettingDouble("enableRightArm");
					bool armsAlreadyDisabled = (existingLeftArm < 0.5) && (existingRightArm < 0.5);

					if (armsAlreadyDisabled) {
						if (verbose) {
							SKSE::log::info("enableLeftArm/enableRightArm already disabled (left={}, right={}) -- skipping selfie lock, something else appears to control VRIK right now.",
								existingLeftArm, existingRightArm);
						}
					} 
					else if ((menuName == "ContainerMenu" || menuName == "BarterMenu") && !IsBodyVisibleMenu(menuName))  {
						if (verbose) {
							SKSE::log::info("entering barter or container menu which does not display body -- skipping selfie lock entirely.");
						}
					}
					else if (menuName == "TweenMenu" && !IsBodyVisibleMenu(menuName) && !IsBodyVisibleMenu("InventoryMenu") && !IsBodyVisibleMenu("MagicMenu"))  {
						if (verbose) {
							SKSE::log::info("entering tween menu but no related menu displays body - skipping selfie lock entirely.");
						}
					else if (menuName != "TweenMenu" && menuName != "InventoryMenu" && menuName != "ContainerMenu" && menuName != "BarterMenu" && menuName != "MagicMenu")  {
						if (verbose) {
							SKSE::log::info("entering an unrelated menu - skipping selfie lock entirely.");
						}
					}
					else {
						auto player = RE::PlayerCharacter::GetSingleton();
						if (player) {
							RE::NiPoint3 pos = player->GetPosition();
							float px = pos.x;
							float py = pos.y;
							float pz = pos.z;
							float angleRad = player->data.angle.z;

							// Attempt to get exact HMD position/rotation if available
							auto hmdNode = player->GetNodeByName("VR HMD");
							if (hmdNode) {
								px = hmdNode->world.translate.x;
								py = hmdNode->world.translate.y;
								pz = hmdNode->world.translate.z;
							}

							auto config = Config::GetSingleton();

							if (config->GetDebugMode()) {
								config->Reload();
							}

							float dist = config->GetDistance();
							float heightOffset = config->GetHeightOffset();
							float horizontality = config->GetHorizontality();

							float targetX = px + dist * std::sin(angleRad) + horizontality * std::cos(angleRad);
							float targetY = py + dist * std::cos(angleRad) - horizontality * std::sin(angleRad);
							float targetZ = pz + heightOffset;

							float facingAngle = angleRad * (180.0f / 3.14159265358979323846f);
							facingAngle += config->GetRotationOffset();
							facingAngle = std::fmod(facingAngle, 360.0f);
							if (facingAngle < 0.0f) facingAngle += 360.0f;

							{
								wasLockedForMenu = true;
								++lockGeneration;

								vrik->setSettingDouble("enableLeftArm", 0.0);
								vrik->setSettingDouble("enableRightArm", 0.0);
								vrik->setSettingDouble("enableBody", 0.0);
								vrik->setSettingDouble("enableHead", 0.0);
								vrik->setSettingDouble("enablePosture", 0.0);

								// Lock VRIK position and rotation ahead of pause state
								vrik->setSettingDouble("lockPosition", 2.0);
								vrik->setSettingDouble("lockPositionX", targetX);
								vrik->setSettingDouble("lockPositionY", targetY);
								vrik->setSettingDouble("lockPositionZ", targetZ);
								vrik->setSettingDouble("lockRotation", 1.0);
								vrik->setSettingDouble("lockRotationAngle", facingAngle);
								vrik->setSettingDouble("hidePlayerHeadDistance", 1.0);

								if (verbose) {
								SKSE::log::info("Pre-locked VRIK body position: X={}, Y={}, Z={}, Rot={}",
									targetX, targetY, targetZ, facingAngle);

								SKSE::log::info("Readback: lockPosition={} lockPositionX={} lockPositionY={} lockPositionZ={} lockRotation={} lockRotationAngle={}",
									vrik->getSettingDouble("lockPosition"),
									vrik->getSettingDouble("lockPositionX"),
									vrik->getSettingDouble("lockPositionY"),
									vrik->getSettingDouble("lockPositionZ"),
									vrik->getSettingDouble("lockRotation"),
									vrik->getSettingDouble("lockRotationAngle"));
								}
						}
					}
				}
			}
			}

			if (IsBodyVisibleMenu(menuName) && vrik && wasLockedForMenu && !isBodyVisible) {
				vrik->setSettingDouble("showBodyInMenu", 1.0);
				isBodyVisible = true;
				if (verbose) {
					SKSE::log::info("Entered a body-visible menu ('{}') -> showBodyInMenu = 1.0", menuName);
				}
			}
		} else {
			if (IsPauseMenu(menuName)) {
				if (activePauseMenuCount > 0) {
					activePauseMenuCount--;
				}

				if (wasLockedForMenu && !AnyPauseMenuOpen()) {
					if (vrik) {
						auto config = Config::GetSingleton();
						auto player = RE::PlayerCharacter::GetSingleton();
						if (player) {
							RE::NiPoint3 pos = player->GetPosition();
							float px = pos.x;
							float py = pos.y;
							float pz = pos.z;
							float currentAngleDeg = player->data.angle.z * (180.0f / 3.14159265358979323846f);
							currentAngleDeg = std::fmod(currentAngleDeg, 360.0f);
							if (currentAngleDeg < 0.0f) currentAngleDeg += 360.0f;

							auto hmdNode = player->GetNodeByName("VR HMD");
							if (hmdNode) {
								px = hmdNode->world.translate.x;
								py = hmdNode->world.translate.y;
								pz = hmdNode->world.translate.z;
							}

							vrik->setSettingDouble("lockPositionX", px);
							vrik->setSettingDouble("lockPositionY", py);
							vrik->setSettingDouble("lockPositionZ", pz);
							vrik->setSettingDouble("lockRotationAngle", currentAngleDeg);

							if (verbose) {
								SKSE::log::info("Re-pointed VRIK lock at live position/angle before final unlock: X={}, Y={}, Z={}, Rot={}",
									px, py, pz, currentAngleDeg);
							}
						}

						int delayMs = config->GetUnlockDelayMs();
						if (delayMs <= 0) {
							vrik->restoreSettings();
							if (verbose) {
								SKSE::log::info("All pause menus exited -> VRIK settings restored immediately.");
							}
						} else {
							int expectedGeneration = lockGeneration.load();
							std::thread([this, vrik, delayMs, verbose, expectedGeneration]() {
								std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
								SKSE::GetTaskInterface()->AddTask([this, vrik, verbose, expectedGeneration]() {
									// If a new lock cycle started (e.g. you reopened the
									// inventory before this delay elapsed), skip restoring --
									// that would undo the newer lock instead of the one this
									// call was scheduled for.
									if (lockGeneration.load() != expectedGeneration) {
										return;
									}
									vrik->restoreSettings();
									if (verbose) {
										SKSE::log::info("All pause menus exited -> VRIK settings restored after unlock delay.");
									}
								});
							}).detach();
						}
					}
					wasLockedForMenu = false;
					isBodyVisible = false;
					activePauseMenuCount = 0;
				}
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}
