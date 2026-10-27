#include "nfs/network/session/network_gui.hpp"
namespace nfs::network::ui {
// Partial CarSettings initialization; the other bytes survive.
void initialize_car_settings(CarSettings *settings) {
    auto *words = reinterpret_cast<Dword *>(settings);
    words[0] = words[1] = words[2] = words[3] = words[4] = 0;
    words[5] = 4294967295u;
    words[14] = 1065353216;
    words[15] = words[16] = words[17] = words[18] = words[19] = words[20] = 0;
    settings->bytes[24] = 0;
}

Message *construct_message(Message *message, const gui::String *text) {
    gui::string_construct(&message->text);
    gui::string_construct(&message->auxiliary);
    message->methods = message_methods;
    try {
        gui::string_copy(&message->text, text);
    } catch (...) {
        gui::string_destruct(&message->auxiliary);
        gui::string_destruct(&message->text);
        throw;
    }
    message->sender = 0;
    message->flags = 0;
    message->kind = 0;
    return message;
}

void destroy_message(Message *message) {
    gui::string_destruct(&message->auxiliary);
    gui::string_destruct(&message->text);
    message->methods = gui::base_methods;
}

void *__fastcall delete_message(Message *message, void *, unsigned flags) {
    destroy_message(message);
    if (flags & 1)
        gui::release(message);
    return message;
}
const gui::Destroy message_methods[]{reinterpret_cast<gui::Destroy>(delete_message)};
} // namespace nfs::network::ui
