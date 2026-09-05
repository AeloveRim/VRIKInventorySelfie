#include "vrikinterface001.h"

namespace vrikPluginApi {
	// A message used to fetch VRIK's interface
	namespace {
		struct VrikMessage {
			enum { kMessage_GetInterface = 0xF2AFAEE6 }; 
			void* (*getApiFunction)(unsigned int revisionNumber) = nullptr;
		};

		constexpr auto VrikPluginName = "VRIK";
		// Stores the API after it has already been fetched

		IVrikInterface001* g_vrikInterface = nullptr;
	}
	// Fetches the interface to use from VRIK

	IVrikInterface001* getVrikInterface001() {
		// If the interface has already been fetched, rturn the same object

		if (g_vrikInterface) {
			return g_vrikInterface;
		}

		auto messaging = SKSE::GetMessagingInterface();
		if (!messaging) {
			return nullptr;
		}
		// Dispatch a message to get the plugin interface from VRIK

		VrikMessage vrikMessage;
		messaging->Dispatch(VrikMessage::kMessage_GetInterface, static_cast<void*>(&vrikMessage), sizeof(VrikMessage*), VrikPluginName);

		if (!vrikMessage.getApiFunction) {
			return nullptr;
		}
		// Fetch the API for this version of the VRIK interface

		g_vrikInterface = static_cast<IVrikInterface001*>(vrikMessage.getApiFunction(1));
		return g_vrikInterface;
	}

}
