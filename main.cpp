#include <iostream>
#include "driver/Driver.h"
#include "Mapper/kdmapper.hpp"
#include "Mapper/utils.hpp"
#include "Mapper/driver.h"
#include <thread>
#include "Roblox/DataModel/DataModel.hpp"
#include "Roblox/Instance/RobloxInstance.hpp"
#include "Roblox/Bridge/Bridge.hpp"
#include "utils/utils.h"
#include "BytecodeUtils.h"
#include "Roblox/Lua State/LuaState.hpp"
#include <zstd.h>
#include <xxhash.h>
#include "utils/xor.h"

HANDLE iqvw64e_device_handle;
using namespace kdmapper;
using namespace intel_driver;

std::string random_string()
{
    srand((unsigned int)time((time_t*)0));
    std::string str = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";
    std::string newstr;
    int pos;
    while (newstr.size() != 32)
    {
        pos = ((rand() % (str.size() + 1)));
        newstr += str.substr(pos, 1);
    }
    return newstr;
}

void TitleThread()
{
    while (true)
    {
        SetConsoleTitleA(random_string().c_str());
    }
}

std::thread Title(TitleThread);


int main()
{

    const auto pDatamodel{ DataModel::get_singleton() };
    const auto pBridge{ Bridge::get_singleton() };
    const auto pLuaState{ LuaState::get_singleton() };
    std::cout << xor_a("[~] Initializing...") << std::endl;

    HANDLE device_handler = Load();

    if (!device_handler || device_handler == INVALID_HANDLE_VALUE)
    {
        std::cout << xor_a("[-] Failed to initialize.") << std::endl;
        std::cin.get();
        return -1;
    }



    MapDriverBytes(device_handler, RawData);

    Unload(device_handler);

    std::cout << xor_a("[+] Initialized") << std::endl;


    std::cout << xor_a("[~] Finding Roblox...") << std::endl;
    const auto pDriver{ Driver::get_singleton() };
    

    std::wstring target = L"RobloxPlayerBeta.exe";
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (strcmp(pe32.szExeFile, xor_a("RobloxPlayerBeta.exe")) == 0) {
                    pid = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    if (pid == 0) {
        std::cout << xor_a("[-] Couldn't find Roblox") << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << xor_a("[+] Found Roblox") << std::endl;

    pDriver->initialize(xor_w(L"\\\\.\\{f751dd83-fcc5-43b5-aa0d-398fe67bc306}"), pid);

    uintptr_t target_process_base_address = pDriver->get_base_address(xor_w(L"RobloxPlayerBeta.exe"));

    if (target_process_base_address == 0) {
        std::cout << xor_a("[-] Failed to get base address of roblox. This may be because of the driver not being loaded") << std::endl;
    }


    // TODO: Add Ingame injection

    auto target_process_hwnd = utils::get_hwnd_of_process_id(pid);
    auto thread_id = GetWindowThreadProcessId(target_process_hwnd, 0);
    ;

    pDatamodel->main_thread_id = thread_id;

    std::uint64_t datamodel = pDatamodel->get_datamodel();
    RobloxInstance game = static_cast<RobloxInstance>(datamodel);

    auto coregui = game.FindFirstChildOfClass(xor_a("CoreGui"));


    auto robloxgui = coregui.find_first_child(xor_a("RobloxGui"));

    auto Modules = robloxgui.find_first_child(xor_a("Modules"));

    auto Common = Modules.find_first_child("Common");


    auto policyservice = Modules.find_first_child(xor_a("CoreUtility"));


    auto script_context = game.FindFirstChildOfClass("ScriptContext");

    std::cout << "[~] Script Context: 0x" << std::hex << script_context.self << std::endl;

    //std::cout << "[~] Getting LuaState" << std::endl;

    //auto idk = pDriver->read<std::uint64_t>(script_context.self + 0x240);

    //auto lua_state = idk + 8;

    //std::cout << "[+] Lua State: 0x" << std::hex << lua_state << std::endl;

    //pLuaState->initialize(lua_state);


    //auto lS_userdata = pDriver->read<std::uint64_t>(lua_state + 0x78);

    //int g = 0x0;


    std::cout << xor_a("[~] Attaching...") << std::endl;


    if (pDatamodel->ingame == false) {
        policyservice.SetBytecode(init_script_bytecode, init_script_size);
    }
    else if(false) {
        auto Playerlist = Modules.find_first_child("PlayerList");
        auto ScriptHook = Playerlist.find_first_child("PlayerListManager");

        auto CorePackages = game.FindFirstChildOfClass("CorePackages");
        auto ScriptLoader = CorePackages.find_first_child("JestGlobals");



        ScriptLoader.SetModuleBypass(); // www
        ScriptHook.spoof(ScriptLoader);

        ScriptLoader.SetBytecode(init_script_bytecode, init_script_size);

        HWND boblox = FindWindowA(NULL, "Roblox");
        SetForegroundWindow(boblox);

        INPUT input[2] = {};

        while (GetForegroundWindow() == boblox) {
            keybd_event(0, MapVirtualKey(VK_ESCAPE, 0), KEYEVENTF_SCANCODE, 0);
            keybd_event(0, MapVirtualKey(VK_ESCAPE, 0), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0);

            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));

        ScriptHook.spoof(ScriptHook);

        ScriptLoader.SetBytecode(jestglobals_bytecode, jestglobals_size);
        // 12:50
    }

    //pLuaState->set_identity(8);

    //std::cout << "[+] Identity set to 8" << std::endl;

    std::cout << xor_a("[+] Attached!") << std::endl;

    //std::cout << "[~] Thanks for using Wind!" << std::endl;

    pBridge->initialize();


    pBridge->start();
}
