//
// Created by Yoru on 5/8/2024.
//

#include "Bridge.hpp"
#include <iostream>
#include <crow.h>
#include <cpr/cpr.h>
#include <string>
#include "../../utils/json.hpp"
#include "../Instance/RobloxInstance.hpp"
#include <fstream>
#include "../DataModel/DataModel.hpp"
#include "../../utils/base64.h"
#include "../../utils/utils.h"
#include "Luau/BytecodeBuilder.h"
#include "Luau/BytecodeUtils.h"
#include "Luau/Compiler.h"
#include "zstd.h"
#include "xxhash.h"
#include "utils/xor.h"

using namespace crow;

Bridge* Bridge::g_Singleton = nullptr;

Bridge* Bridge::get_singleton() noexcept {
    if (g_Singleton == nullptr)
        g_Singleton = new Bridge();
    return g_Singleton;
}

SimpleApp app;

class bytecode_encoder_t : public Luau::BytecodeEncoder {
    inline void encode(uint32_t* data, size_t count) override {
        // Loop through the instructions.
        for (auto i = 0u; i < count;) {
            // Get the opcode (which is the first byte of the instruction).
            auto& opcode = *reinterpret_cast<uint8_t*>(data + i);

            // Add the instruction length (which could be multiple 32-bit integers).
            i += Luau::getOpLength(LuauOpcode(opcode));

            // Encode the opcode.
            opcode *= 227;
        }
    }
};

std::string compress_bytecode(std::string_view bytecode) {
    // Create a buffer.
    const auto data_size = bytecode.size();
    const auto max_size = ZSTD_compressBound(data_size);
    auto buffer = std::vector<char>(max_size + 8);

    // Copy RSB1 and data size into the buffer.
    strcpy_s(&buffer[0], buffer.capacity(), xor_a("RSB1"));
    memcpy_s(&buffer[4], buffer.capacity(), &data_size, sizeof(data_size));

    // Copy compressed bytecode into the buffer.
    const auto compressed_size = ZSTD_compress(&buffer[8], max_size, bytecode.data(), data_size, ZSTD_maxCLevel());
    if (ZSTD_isError(compressed_size))
        throw std::runtime_error(xor_a("Failed to compress the bytecode."));

    // Encrypt the buffer.
    const auto size = compressed_size + 8;
    const auto key = XXH32(buffer.data(), size, 42u);
    const auto bytes = reinterpret_cast<const uint8_t*>(&key);

    for (auto i = 0u; i < size; ++i)
        buffer[i] ^= bytes[i % 4] + i * 41u;

    // Create and return output.
    return std::string(buffer.data(), size);
}


void Bridge::initialize() {
    CROW_ROUTE(app, "/bridge")
            .methods("POST"_method)
                    ([](const request& req, response& res) {
                        auto json_body = nlohmann::json::parse(req.body);

                        if (json_body.contains(xor_a("action")) && json_body.contains(xor_a("script"))) {
                            auto action = json_body["action"].get<std::string>();

                            if (action == xor_a("loadstring")) {
                                nlohmann::json response_json;

                                RobloxInstance real_game = static_cast<RobloxInstance>(DataModel::get_singleton()->get_datamodel());
                                auto coregui = real_game.FindFirstChildOfClass("CoreGui");

                                auto module_script = coregui.find_first_child("YoruBlox");

                                if (!module_script.self)
                                    return;

                                module_script.SetModuleBypass();

                                static auto encoder = bytecode_encoder_t();

                                auto bytecode = Luau::compile("return function(...) " + base64_decode(json_body["script"].get<std::string>()) + "\nend", {}, {}, &encoder);
                                auto compressed = compress_bytecode(bytecode);

                                std::vector<uint8_t> sigma_rizz(compressed.begin(), compressed.end());


                                module_script.SetBytecode(sigma_rizz, compressed.size());
                                response_json["status"] = "success";
                                response_json["message"] = "loaded bytecode into script";
                                res.set_header("Content-Type", "application/json");
                                res.write(response_json.dump());
                                res.end();

                            } else {
                                res.code = 400;
                                res.write("Invalid action");
                            }
                        } else {
                            res.code = 400;
                            res.write("Missing 'action' or 'script' field");
                        }
                        res.end();
                    });

    CROW_ROUTE(app, "/bridge")
            .methods("GET"_method)
                    ([](const request& req, response& res) {
                        auto action = req.url_params.get("action");
                        auto arg1 = req.url_params.get("arg1");
                        auto arg2 = req.url_params.get("arg2");

                        nlohmann::json response_json;

                        if (std::string(action) == "HttpGet") {
                            auto result = cpr::Get(cpr::Url{ arg1 });
                            response_json["status"] = "success";
                            response_json["response"] = base64_encode(result.text);
                            response_json["message"] = "Done";
                        } else if (std::string(action) == "isfolder") {
                            if (std::filesystem::is_directory("workspace\\" + std::string(arg1))) {
                                response_json["status"] = "success";
                                response_json["message"] = "True";
                            } else {
                                response_json["status"] = "success";
                                response_json["message"] = "False";
                            }
                        } else if (std::string(action) == "fix_require") {
                            RobloxInstance real_game = static_cast<RobloxInstance>(DataModel::get_singleton()->get_datamodel());
                            auto coregui = real_game.FindFirstChildOfClass("CoreGui");
                            auto g = coregui.find_first_child("moduleholder_yorublox").get_object_value();

                            if (!g.self) {
                                response_json["status"] = "error";
                                response_json["message"] = "Important files are missing";
                            } else {
                                g.SetModuleBypass();
                                response_json["status"] = "success";
                                response_json["message"] = "Bypassed";
                            }
                        } else if (std::string(action) == "writefile") {
                            std::ofstream outfile("workspace\\" + std::string(arg1));

                            if (outfile.is_open()) {
                                outfile << arg2;
                                outfile.close();
                                response_json["status"] = "success";
                                response_json["message"] = "Wrote content to file";
                            } else {
                                response_json["status"] = "error";
                                response_json["message"] = "Failed to write to file";
                            }
                        } else if (std::string(action) == "readfile") {
                            std::ifstream inputf("workspace\\" + std::string(arg1));

                            if (inputf.is_open()) {
                                std::string content;
                                while (std::getline(inputf, content)) {}
                                inputf.close();
                                response_json["status"] = "success";
                                response_json["message"] = content;
                            } else {
                                response_json["status"] = "error";
                                response_json["message"] = "Failed to read file";
                            }
                        } else if (std::string(action) == "makefolder") {
                            bool result = std::filesystem::create_directories("workspace\\" + std::string(arg1));

                            if (result) {
                                response_json["status"] = "success";
                                response_json["message"] = "Done easily";
                            } else {
                                response_json["status"] = "error";
                                response_json["message"] = "Failed to create folders";
                            }
                        } else if (std::string(action) == "isfile") {
                            if (std::filesystem::is_regular_file("workspace\\" + std::string(arg1))) {
                                response_json["status"] = "success";
                                response_json["message"] = "True";
                            } else {
                                response_json["status"] = "success";
                                response_json["message"] = "False";
                            }
                        } else if (std::string(action) == "delfile") {
                            bool result = std::filesystem::remove("workspace\\" + std::string(arg1));

                            if (result) {
                                response_json["status"] = "success";
                                response_json["message"] = "Removed file";
                            } else {
                                response_json["status"] = "error";
                                response_json["message"] = "Failed to remove file";
                            }
                        } else if (std::string(action) == "delfolder") {
                            bool result = std::filesystem::remove_all("workspace\\" + std::string(arg1));

                            if (result) {
                                response_json["status"] = "success";
                                response_json["message"] = "Removed folder";
                            } else {
                                response_json["status"] = "error";
                                response_json["message"] = "Folder doesn't exist";
                            }
                        } else if (std::string(action) == "appendfile") {
                            std::ofstream outfile("workspace\\" + std::string(arg1), std::ios_base::app);

                            if (outfile.is_open()) {
                                outfile.write(arg2, std::strlen(arg2));
                                outfile.close();
                                response_json["status"] = "success";
                                response_json["message"] = "Appended content to file";
                            } else {
                                response_json["status"] = "error";
                                response_json["message"] = "Failed to append to file";
                            }
                        } else if (std::string(action) == "listfiles") {
                            std::vector<std::string> files_paths = {};

                            if (std::filesystem::is_directory("workspace\\" + std::string(arg1))) {
                                for (const auto& entry : std::filesystem::directory_iterator("workspace\\" + std::string(arg1))) {
                                    auto file_path = entry.path().string();
                                    file_path = utils::replace(file_path, "workspace\\", "");
                                    files_paths.push_back(file_path.c_str());
                                }
                                response_json["status"] = "success";
                                response_json["message"] = nlohmann::json(files_paths);
                            } else {
                                response_json["status"] = "error";
                                response_json["message"] = "Failed to list files";
                            }
                        } else if (std::string(action) == "getscriptbytecode") {
                            // Not working as of right now
                            // TODO: FIX IT

                            RobloxInstance real_game = static_cast<RobloxInstance>(DataModel::get_singleton()->get_datamodel());
                            auto coregui = real_game.FindFirstChildOfClass("CoreGui");

                            auto holder = coregui.find_first_child("getscriptbytecode");
                            auto script = holder.get_object_value();

                            std::cout << std::hex << "[+] Target Script -> " << script.self << std::endl;

                            if (!script.self) {
                                response_json["status"] = "error";
                                response_json["message"] = "Failed to obtain script";
                            } else {
                                response_json["status"] = "success";
                                response_json["message"] = base64_encode("ehrh");
                            }
                        }
                        res.set_header("Content-Type", "application/json");
                        res.write(response_json.dump());
                        res.end();
                    });
}

void Bridge::start() {
    crow::logger::setLogLevel(crow::LogLevel::Critical); // Disables logging
    app.port(8000).multithreaded().run();
}