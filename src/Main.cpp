#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>

#include "Config.h"
#include "MenuEventHandler.h"
#include "vrikinterface001.h"

namespace {
	void InitializeLog() {
		auto path = SKSE::log::log_directory();
		if (path) {
			*path /= "VrikInventorySelfie.log";
			auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
			auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
			log->set_level(spdlog::level::info);
			log->flush_on(spdlog::level::info);
			spdlog::set_default_logger(std::move(log));
			spdlog::set_pattern("[%H:%M:%S:%e] [%l] %v");
		}
	}

	void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
		if (!a_msg) return;

		if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
			VrikInventorySelfie::MenuEventHandler::Register();
			SKSE::log::info("Registered MenuEventHandler on DataLoaded.");
		} else if (a_msg->type == SKSE::MessagingInterface::kPostLoad) {
			auto vrik = vrikPluginApi::getVrikInterface001();
			if (vrik) {
				SKSE::log::info("Successfully acquired VRIK interface (build {}).", vrik->getBuildNumber());
			} else {
				SKSE::log::warn("VRIK interface not found during PostLoad. Is VRIK installed and loaded?");
			}
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
	InitializeLog();
	SKSE::log::info("VrikInventorySelfie plugin loading...");

	SKSE::Init(a_skse);

	VrikInventorySelfie::Config::GetSingleton()->LoadConfig("Data\\SKSE\\Plugins\\VrikInventorySelfie.ini");

	auto messaging = SKSE::GetMessagingInterface();
	if (messaging) {
		messaging->RegisterListener(MessageHandler);
	}

	SKSE::log::info("VrikInventorySelfie loaded successfully.");
	return true;
}
