#include "../session/race_runtime.hpp"
namespace nfs::application::pending {

void initialize_race_rewards(void *rewards) {
    reset_race_rewards(rewards);
}

void load_race_reward_rules(void *rewards, const void *source) {
    kernel::memory_copy(rewards, source, 12);
}

void read_race_reward_rules(void *rules, void *settings, void *rewards) {
    for (unsigned i = 0; i < 18; ++i)
        at<Dword>(settings, i * 4) = at<Dword>(rules, 102924 + i * 4);
    kernel::memory_copy(rewards, bytes(rules) + 102996, 12);
}

void *active_event_progress(void *rules) {
    const Dword mode = at<Dword>(rules, 148);
    if (mode == 2 || mode == 4) {
        void *entry =
            bytes(rules) + (mode == 2 ? 10668 : 31260) + at<Dword>(rules, 102920) * 624;
        if (entry)
            return bytes(entry) + 4;
    }
    return bytes(rules) + 10048;
}

void mark_event_entered(void *progress) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = ((((flags & 4294967280u) - 1) ^ flags) & 240) ^ flags | 1;
}

Dword collision_reward(void *rewards, int severity, int difficulty) {
    if (difficulty < 0)
        difficulty = 0;
    else if (difficulty > 2)
        difficulty = 2;
    if (severity < 1)
        severity = 1;
    else if (severity > 8)
        severity = 8;
    return at<Dword>(rewards, 296 + (Dword(severity) + Dword(difficulty) * 8) * 4);
}

Dword takedown_reward(void *rewards, int difficulty) {
    if (difficulty < 0)
        return at<Dword>(rewards, 396);
    if (difficulty > 2)
        difficulty = 2;
    return at<Dword>(rewards, 396 + Dword(difficulty) * 4);
}

void add_participant_reward(void *record, Dword kind, Dword value) {
    at<Dword>(record, 948 + kind * 4) += value;
    ++at<Dword>(record, 968 + kind * 4);
}
} // namespace nfs::application::pending

namespace nfs::application::pending {
// Reward defaults; other words remain untouched.
void reset_race_rewards(void *rewards) {
    at<Dword>(rewards, 24) = 300;
    at<Dword>(rewards, 124) = 300;
    at<Dword>(rewards, 132) = 0;
    at<Dword>(rewards, 136) = 0;
    at<Dword>(rewards, 224) = 0;
    at<Dword>(rewards, 228) = 0;
    at<Dword>(rewards, 232) = 0;
    at<Dword>(rewards, 32) = 100;
    at<Dword>(rewards, 412) = 75;
    at<Dword>(rewards, 416) = 100;
    at<Dword>(rewards, 400) = 75;
    at<Dword>(rewards, 404) = 100;
    at<Dword>(rewards, 300) = 100;
    at<Dword>(rewards, 28) = 200;
    at<Dword>(rewards, 128) = 200;
    at<Dword>(rewards, 316) = 200;
    at<Dword>(rewards, 332) = 200;
    at<Dword>(rewards, 16) = 750;
    at<Dword>(rewards, 116) = 750;
    at<Dword>(rewards, 324) = 250;
    at<Dword>(rewards, 336) = 250;
    at<Dword>(rewards, 356) = 500;
    at<Dword>(rewards, 368) = 500;
    at<Dword>(rewards, 12) = 1000;
    at<Dword>(rewards, 112) = 1000;
    at<Dword>(rewards, 212) = 1000;
    at<Dword>(rewards, 388) = 1000;
    at<Dword>(rewards, 20) = 500;
    at<Dword>(rewards, 36) = 50;
    at<Dword>(rewards, 40) = 10;
    at<Dword>(rewards, 108) = 1500;
    at<Dword>(rewards, 120) = 400;
    at<Dword>(rewards, 204) = 2000;
    at<Dword>(rewards, 208) = 1500;
    at<Dword>(rewards, 216) = 600;
    at<Dword>(rewards, 220) = 400;
    at<Dword>(rewards, 408) = 50;
    at<Dword>(rewards, 396) = 50;
    at<Dword>(rewards, 304) = 125;
    at<Dword>(rewards, 308) = 150;
    at<Dword>(rewards, 312) = 175;
    at<Dword>(rewards, 320) = 225;
    at<Dword>(rewards, 328) = 275;
    at<Dword>(rewards, 340) = 300;
    at<Dword>(rewards, 344) = 350;
    at<Dword>(rewards, 348) = 400;
    at<Dword>(rewards, 352) = 450;
    at<Dword>(rewards, 360) = 600;
    at<Dword>(rewards, 364) = 400;
    at<Dword>(rewards, 372) = 600;
    at<Dword>(rewards, 376) = 700;
    at<Dword>(rewards, 380) = 800;
    at<Dword>(rewards, 384) = 900;
    at<Dword>(rewards, 392) = 1200;
}
} // namespace nfs::application::pending
