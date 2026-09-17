#include "GPUWarmupManager.h"
#include "AssetDiscovery.h"
#include <fstream>
namespace tape {
GPUWarmupManager& GPUWarmupManager::Get() {
    // SKSE DLLs live until process exit. Do not destroy engine references during CRT teardown.
    static auto* instance = new GPUWarmupManager;
    return *instance;
}
void GPUWarmupManager::Initialize() {
    const auto* tasks = SKSE::GetTaskInterface();
    scheduler = std::jthread([this, tasks](std::stop_token stop) {
        while (!stop.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs.load()));
            if (!running.load()) continue;
            const auto token = generation.load();
            if (pending.exchange(true)) continue;
            // Only dispatch from this worker. All engine objects and containers stay on game thread.
            tasks->AddTask([this, token] {
                if (running.load() && generation.load() == token) ProcessBatch();
                pending.store(false);
            });
        }
    });
}
void GPUWarmupManager::Stop() {
    running.store(false);
    ++generation;
    if (!queue.empty() || !residentModels.empty())
        spdlog::info("Stopping: processed={} retained={} (releasing roots)", cursor, residentModels.size());
    queue.clear();
    residentModels.clear();
}
void GPUWarmupManager::Start() {
    Stop();
    config = Config::Load();
    if (!config.enabled || !config.startOnPostLoadGame) {
        spdlog::info("Automatic model loading disabled by configuration");
        return;
    }
    std::ifstream input("Data/SKSE/Plugins/SkGPUTapeModels.txt");
    if (!input) { spdlog::error("Cannot open SkGPUTapeModels.txt"); return; }
    auto list = ReadModelList(input, config.maxModels);
    if (input.bad()) { spdlog::error("Failed reading model list"); return; }
    spdlog::info("Discovered={} queued={} duplicates={} invalid={} excludedByLimit={}",
        list.discovered, list.paths.size(), list.duplicates, list.invalid, list.excludedByLimit);
    for (const auto line : list.invalidLines) spdlog::warn("Invalid model path at line {}", line);
    queue = std::move(list.paths);
    cursor = loaded = failed = batch = 0;
    started = lastReport = std::chrono::steady_clock::now();
    intervalMs.store(config.batchIntervalMs);
    spdlog::info("Mode=LoadOnly modelsPerBatch={} intervalMs={} retainRoots={}; rendered=unmeasured GPU residency=unverified",
        config.modelsPerBatch, config.batchIntervalMs, config.retainModels);
    if (queue.empty()) { spdlog::warn("No models queued; edit SkGPUTapeModels.txt"); return; }
    if (config.notifications) RE::DebugNotification("SkGPUTape: model loading started (no render submission)");
    running.store(true);
}
void GPUWarmupManager::ProcessBatch() {
    // Menus can pause gameplay; don't initiate expensive demands during them.
    if (auto* ui = RE::UI::GetSingleton(); ui && ui->GameIsPaused()) return;
    ++batch;
    const auto end = std::min(queue.size(), cursor + config.modelsPerBatch);
    spdlog::info("Batch {}: models {}..{} / {}", batch, cursor + 1, end, queue.size());
    for (; cursor < end; ++cursor) {
        const auto& path = queue[cursor];
        RE::NiPointer<RE::NiNode> root;
        const RE::BSModelDB::DBTraits::ArgsType args{};
        if (config.verbose) { spdlog::info("Demand begin: {}", path); spdlog::default_logger()->flush(); }
        const auto error = RE::BSModelDB::Demand(ModelDBPath(path).c_str(), root, args);
        if (error != RE::BSResource::ErrorCode::kNone || !root) {
            ++failed;
            spdlog::warn("Demand failed: {} error={} nullRoot={}", path, static_cast<int>(error), !root);
            continue;
        }
        ++loaded;
        if (config.verbose) spdlog::info("Loaded: {}", path);
        if (config.retainModels) residentModels.push_back(std::move(root));
    }
    const bool complete = cursor == queue.size();
    if (complete || std::chrono::steady_clock::now() - lastReport >= std::chrono::seconds(3)) Report(complete);
    if (complete) running.store(false);
}
void GPUWarmupManager::Report(bool complete) {
    lastReport = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration<double>(lastReport - started).count();
    spdlog::info("{} processed={}/{} loaded={} failed={} retainedRoots={} rendered=unmeasured elapsed={:.2f}s",
        complete ? "Load pass complete" : "Progress", cursor, queue.size(), loaded, failed, residentModels.size(), elapsed);
    if (config.notifications) {
        const auto text = fmt::format("SkGPUTape: {}/{} loaded, {} failed{}", loaded, queue.size(), failed, complete ? " (complete)" : "");
        RE::DebugNotification(text.c_str());
    }
}
RE::BSEventNotifyControl GPUWarmupManager::ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (event && event->opening && event->menuName == RE::MainMenu::MENU_NAME) Stop();
    return RE::BSEventNotifyControl::kContinue;
}
}
