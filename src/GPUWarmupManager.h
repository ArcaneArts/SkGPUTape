#pragma once
#include "PCH.h"
#include "Config.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
namespace tape {
class GPUWarmupManager : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
    static GPUWarmupManager& Get();
    void Initialize();
    void Start();
    void Stop();
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent*, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
private:
    void ProcessBatch();
    void Report(bool complete);
    Config config;
    std::vector<std::string> queue;
    std::vector<RE::NiPointer<RE::NiNode>> residentModels;
    std::size_t cursor{}, loaded{}, failed{}, batch{};
    std::chrono::steady_clock::time_point started, lastReport;
    std::atomic_bool running{false}, pending{false};
    std::atomic_uint64_t generation{0};
    std::atomic_uint32_t intervalMs{100};
    std::jthread scheduler;
};
}
