#pragma once
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::network::ui {
struct CarSettings {
    alignas(4) std::uint8_t bytes[84];
};
struct PlayerRecord {
    Dword identifier;
    char name[24];
    std::int16_t slot;
    std::uint16_t slot_high;
    CarSettings car;
    std::int16_t phase;
    std::uint8_t trailing[18];
};
struct ConsoleText {
    const gui::Destroy *methods;
    char text[256];
};
struct Message {
    const gui::Destroy *methods;
    gui::String text;
    int sender;
    Dword flags;
    gui::String auxiliary;
    Dword kind;
};
static_assert(sizeof(CarSettings) == 84 && offsetof(PlayerRecord, car) == 32 &&
              sizeof(PlayerRecord) == 136);
static_assert(offsetof(PlayerRecord, phase) == 116);
static_assert(sizeof(ConsoleText) == 260 && sizeof(Message) == 56 &&
              offsetof(Message, auxiliary) == 32);
extern const gui::Destroy console_methods[], console_base_methods[], message_methods[];
void initialize_car_settings(CarSettings *);
Message *construct_message(Message *, const gui::String *);
void destroy_message(Message *);
void __fastcall notice(Dword *, void *, const gui::String *, std::int16_t, int, int, int);
Dword console_available();
Dword console_print(const char *);
void append_console_unlocked(void *, const ConsoleText *);
void append_console(void *, const ConsoleText *);
ConsoleText *copy_console_text(ConsoleText *, const ConsoleText *);
void destroy_console_text(ConsoleText *);
} // namespace nfs::network::ui
