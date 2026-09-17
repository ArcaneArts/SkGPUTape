#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "Config.h"
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <string>
namespace tape {
Config Config::Load() {
    const auto path = std::filesystem::absolute("Data/SKSE/Plugins/SkGPUTape.ini").string();
    auto read = [&](const char* section, const char* key, const char* fallback) {
        char buffer[64]{};
        GetPrivateProfileStringA(section, key, fallback, buffer, sizeof(buffer), path.c_str());
        return std::string(buffer);
    };
    auto boolean = [&](const char* section, const char* key, bool fallback) {
        auto value = read(section, key, fallback ? "true" : "false");
        for (auto& c : value) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        if (value == "true" || value == "1") return true;
        if (value == "false" || value == "0") return false;
        return fallback;
    };
    auto number = [&](const char* key, unsigned fallback, unsigned minimum, unsigned maximum) {
        auto value = read("Warmup", key, std::to_string(fallback).c_str());
        unsigned result{};
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
        if (error != std::errc{} || end != value.data() + value.size()) return fallback;
        return std::clamp(result, minimum, maximum);
    };
    Config c;
    c.enabled = boolean("General", "Enabled", c.enabled);
    c.startOnPostLoadGame = boolean("General", "StartOnPostLoadGame", c.startOnPostLoadGame);
    c.notifications = boolean("General", "ShowNotifications", c.notifications);
    c.retainModels = boolean("Warmup", "RetainModels", c.retainModels);
    c.verbose = boolean("Logging", "Verbose", c.verbose);
    c.modelsPerBatch = number("ModelsPerBatch", 1, 1, 250);
    c.batchIntervalMs = number("BatchIntervalMs", 100, 16, 10000);
    c.maxModels = number("MaxModels", 50, 0, 1000000);
    return c;
}
}
