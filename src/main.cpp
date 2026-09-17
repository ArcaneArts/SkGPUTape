#include "PCH.h"
#include "GPUWarmupManager.h"
#include <spdlog/sinks/basic_file_sink.h>
namespace {
void OnMessage(SKSE::MessagingInterface::Message* message) {
    auto& manager = tape::GPUWarmupManager::Get();
    switch (message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        if (auto* ui = RE::UI::GetSingleton()) ui->AddEventSink<RE::MenuOpenCloseEvent>(&manager);
        break;
    case SKSE::MessagingInterface::kPreLoadGame:
    case SKSE::MessagingInterface::kNewGame:
        manager.Stop();
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
        // SKSE encodes the success boolean in the pointer value, not at its address.
        spdlog::info("PostLoadGame success={}", message->data != nullptr);
        if (message->data) manager.Start();
        else manager.Stop();
        break;
    default: break;
    }
}
}
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    const auto directory = SKSE::log::log_directory();
    if (!directory) return false;
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>((*directory / "SkGPUTape.log").string(), true);
    auto logger = std::make_shared<spdlog::logger>("SkGPUTape", std::move(sink));
    spdlog::set_default_logger(std::move(logger));
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%l] %v");
    spdlog::flush_on(spdlog::level::info);
    spdlog::info("SkGPUTape 0.1.0 initialized; runtime={}; mode=LoadOnly", REL::Module::get().version().string());
    if (!SKSE::GetMessagingInterface()->RegisterListener(OnMessage)) return false;
    tape::GPUWarmupManager::Get().Initialize();
    return true;
}
