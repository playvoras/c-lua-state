cmake_minimum_required(VERSION 3.28)
project(YoruBlox_Wind)

set(CMAKE_CXX_STANDARD 23)

add_executable(YoruBlox_Wind main.cpp
        BytecodeUtils.h
        offsets.hpp
        utils/base64.h
        utils/json.hpp
        utils/utils.h
        utils/xor.h
        Roblox/Bridge/Bridge.cpp
        Roblox/Bridge/Bridge.hpp
        Roblox/DataModel/DataModel.cpp
        Roblox/DataModel/DataModel.hpp
        Roblox/Instance/RobloxInstance.cpp
        Roblox/Instance/RobloxInstance.hpp
        "Roblox/Lua State/LuaState.cpp"
        "Roblox/Lua State/LuaState.hpp"
        Mapper/driver.h
        Mapper/intel_driver.cpp
        Mapper/intel_driver.hpp
        Mapper/intel_driver_resource.hpp
        Mapper/kdmapper.cpp
        Mapper/kdmapper.hpp
Mapper/nt.hpp Mapper/portable_executable.cpp Mapper/portable_executable.hpp
Mapper/service.cpp Mapper/service.hpp Mapper/utils.cpp Mapper/utils.hpp
driver/Driver.cpp driver/Driver.h)

target_include_directories(YoruBlox_Wind PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(YoruBlox_Wind PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies")
# Dependencies


find_package(zstd CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(xxhash CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(crow CONFIG REQUIRED)

# Curl For People
# FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
#         GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8)
# FetchContent_MakeAvailable(cpr)

#rcmp
# FetchContent_Declare(rcmp GIT_REPOSITORY https://github.com/Smertig/rcmp.git
#         GIT_TAG f5f75ae00a57c67fe41f79fd59c4b7f6997b999e)
# FetchContent_MakeAvailable(rcmp)

# Luau
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Luau")

# minhook
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Minhook")

# Imgui
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Imgui")

# oxorany
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/oxorany")

# IXWebSocket
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/IXWebSocket")

SET_TARGET_PROPERTIES(YoruBlox_Wind PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")

target_link_libraries(${PROJECT_NAME}
        PRIVATE

        # Luau
        Luau.Compiler
        Luau.Ast
        # Luau.Analysis
        Luau.VM
        Luau.VM.Internals

        # Imgui
        Imgui

        zstd::libzstd

        Crow::Crow

        asio::asio

        xxHash::xxhash

        cpr::cpr


        # oxorany
        oxorany
        ixwebsocket
)