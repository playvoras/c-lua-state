//
// Created by Yoru on 5/8/2024.
//

#include "LuaState.hpp"
#include <iostream>
#include "../Instance/RobloxInstance.hpp"

LuaState* LuaState::g_Singleton = nullptr;

const auto pDriver{ Driver::get_singleton() };

LuaState* LuaState::get_singleton() noexcept {
	if (g_Singleton == nullptr)
		g_Singleton = new LuaState();
	return g_Singleton;
}

void LuaState::initialize(std::uint64_t lua_state) {
    this->LS = lua_state;
}

void LuaState::set_identity(int identityy) {
    this->identity = identityy;


    auto LS_UserData = pDriver->read<std::uint64_t>(this->LS + 0x78);


    //pDriver->write<unsigned long>(LS_UserData + 0x48, 0x3FFFF00); // capabilities
}