#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

// Minimal helper for calling an already-registered Papyrus global function
// (like DynamicHDT's ResetPhysics) directly from native code, without going
// through a Papyrus script of our own. Adapted from the DispatchStaticCall
// wrapper pattern used in SexLabPP's Script.h.
//
// This matters specifically because we need the call to land at the exact
// same synchronous point we already lock VRIK's position -- before the game
// pauses -- rather than relying on a Papyrus script's own OnMenuOpen event,
// which is subject to the Papyrus VM's own queuing latency and isn't
// guaranteed to fire before the freeze (this is very likely why the
// original Papyrus VRIK prototype couldn't move the body once paused).
// Dispatching directly from our C++ event sink sidesteps that VM latency
// entirely, since we're not waiting for Papyrus to notice anything.
namespace PapyrusDispatch {
	using VM = RE::BSScript::Internal::VirtualMachine;
	using CallbackPtr = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>;
	using Args = RE::BSScript::IFunctionArguments;

	// Fires a static/global Papyrus function call (e.g. class "DynamicHDT",
	// function "ResetPhysics") with no callback for the return value --
	// fire-and-forget, matching how we use this (we don't need SMP's
	// return value, just to trigger the reset).
	//
	// Returns false if dispatch failed outright (e.g. the class/function
	// isn't registered -- which happens harmlessly if the other mod, like
	// HDT SMP, isn't installed/loaded). This does not throw or crash on a
	// missing class/function; it's a normal, checked failure path.
	template <class... TArgs>
	inline bool DispatchStaticCall(const RE::BSFixedString& a_class, const RE::BSFixedString& a_fnName, TArgs&&... a_args) {
		auto vm = VM::GetSingleton();
		if (!vm) {
			return false;
		}

		CallbackPtr nullCallback;  // fire-and-forget: no return-value handling
		auto* args = RE::MakeFunctionArguments(std::forward<TArgs>(a_args)...);
		return vm->DispatchStaticCall(a_class, a_fnName, args, nullCallback);
	}
}
