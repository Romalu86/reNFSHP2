#pragma once
#include "../../ai/driving/ai_car_behavior.hpp"
namespace nfs::game::runtime::events {
void deliver(Dword *, void *);
void enqueue(Dword *, const void *);
} // namespace nfs::game::runtime::events
namespace nfs::services::input_events {
void emit_action(void *, nfs::kernel::Dword, nfs::kernel::Dword, std::uint8_t);
void emit_car_actions(const nfs::kernel::Dword *);
void dispatch_car_actions(void *, void *, const nfs::kernel::Dword *);
} // namespace nfs::services::input_events
