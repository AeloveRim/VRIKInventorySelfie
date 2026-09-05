#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <string>
#include <vector>
#include <atomic>

namespace VrikInventorySelfie {
	class MenuEventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
	public:
		static MenuEventHandler* GetSingleton() {
			static MenuEventHandler singleton;
			return &singleton;
		}

		static void Register();

		virtual RE::BSEventNotifyControl ProcessEvent(
			const RE::MenuOpenCloseEvent* a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;

	private:
		MenuEventHandler() = default;
		~MenuEventHandler() override = default;

		bool IsPauseMenu(const std::string_view& menuName) const;
		bool IsBodyVisibleMenu(const std::string_view& menuName) const;
		bool AnyPauseMenuOpen() const;

		bool wasLockedForMenu = false;
		bool isBodyVisible = false;
		int activePauseMenuCount = 0;

		// Incremented every time a new lock cycle starts (see ProcessEvent).
		// The delayed restoreSettings() call on the unlock path captures this
		// value and only actually restores if it still matches -- if you
		// reopen the inventory again before the delay elapses, a newer lock
		// cycle has started and the stale delayed restore is skipped instead
		// of undoing it. std::atomic because it's read/written from the
		// background delay thread as well as the main thread.
		std::atomic<int> lockGeneration{ 0 };
	};
}
