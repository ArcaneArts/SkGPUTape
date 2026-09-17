#pragma once
#include <cstdint>
namespace tape {
struct Config {
    bool enabled{true};
    bool startOnPostLoadGame{true};
    bool notifications{true};
    bool retainModels{true};
    bool verbose{true};
    std::uint32_t modelsPerBatch{1};
    std::uint32_t batchIntervalMs{100};
    std::uint32_t maxModels{50};
    static Config Load();
};
}
