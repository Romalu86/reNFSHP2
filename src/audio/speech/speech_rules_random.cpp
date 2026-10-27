#include "nfs/audio/speech/speech_rules.hpp"

namespace nfs::speech {

Byte *speech_configuration() {
    return &state<Byte>(8767920);
}


Dword current_speech_tick() {
    const auto callback = state<TickCallback>(8767928);
    return callback ? callback() : 0;
}


Dword next_speech_random() {
    Dword &high = state<Dword>(7146812);
    Dword &fifth = state<Dword>(7146816);
    Dword &fourth = state<Dword>(7146820);
    Dword &third = state<Dword>(7146824);
    Dword &second = state<Dword>(7146828);
    Dword &low = state<Dword>(7146832);
    const Dword sum = second + low;
    const Dword carry = sum < low || sum < second;
    second = sum;
    Dword next = sum + (carry + third);
    Dword add = (next < third) + fourth;
    third = next;
    next += add;
    add = (next < fourth) + fifth;
    fourth = next;
    next += add;
    add = next + (next < fifth);
    fifth = next;
    high += add;
    if (++low == 0 && ++second == 0 && ++third == 0 && ++fourth == 0 && ++fifth == 0)
        ++high;
    return high;
}


void initialize_speech_random(Dword seed) {
    for (unsigned index = 0; index < 32; ++index) {
        state<std::int16_t>(8767776 + index * 4) = -1;
        state<std::int16_t>(8767778 + index * 4) = -1;
    }
    state<int>(8739168) = 0;
    seed_speech_random(seed * 65537u);
}


void seed_speech_random(Dword seed) {
    state<Dword>(7146812) = seed + 4063039062u;
    state<Dword>(7146816) = seed + 2284922601u;
    state<Dword>(7146820) = seed + 3324304687u;
    state<Dword>(7146824) = seed + 117621916;
    state<Dword>(7146828) = seed + 2654289789u;
    state<Dword>(7146832) = seed + 1876900708;
}


std::uint16_t choose_phrase_index(int count, Dword tag) {
    int window = count / 2;
    if (window > 10)
        window = 10;
    Dword best = ((next_speech_random() >> 16) * static_cast<Dword>(count)) >> 16;
    Dword candidate;
    int best_age = -1;
    for (int attempt = 0; attempt < 32; ++attempt) {
        candidate = ((next_speech_random() >> 16) * static_cast<Dword>(count)) >> 16;
        const int age = recent_phrase_distance(static_cast<std::int16_t>(candidate), window,
                                               static_cast<std::int16_t>(tag));
        if (age == -1) {
            best = candidate;
            break;
        }
        if (best_age < age) {
            best = candidate;
            best_age = age;
        }
    }
    remember_phrase(static_cast<std::int16_t>(best), static_cast<std::int16_t>(tag));
    // Record the best candidate, but return the final candidate as a WORD.
    return static_cast<std::uint16_t>(candidate);
}


int recent_phrase_distance(std::int16_t phrase, int count, std::int16_t tag) {
    if (count > 32)
        count = 32;
    int cursor = state<int>(8739168);
    int matching_tags = 0;
    for (int scanned = 0; scanned < 33; ++scanned) {
        if (matching_tags >= count)
            break;
        if (state<std::int16_t>(8767778 + cursor * 4) == tag) {
            if (state<std::int16_t>(8767776 + cursor * 4) == phrase)
                return matching_tags;
            ++matching_tags;
        }
        if (--cursor < 0)
            cursor += 32;
    }
    return -1;
}


void remember_phrase(std::int16_t phrase, std::int16_t tag) {
    int &cursor = state<int>(8739168);
    cursor = static_cast<int>(static_cast<Dword>(cursor) + 1);
    if (cursor >= 32)
        cursor = 0;
    state<std::int16_t>(8767776 + cursor * 4) = phrase;
    state<std::int16_t>(8767778 + cursor * 4) = tag;
}


int samples_to_centiseconds(int samples) {
    const int rate = state<int>(8767916);
    return rate ? static_cast<int>(static_cast<Dword>(samples) * 100) / rate : 0;
}
} // namespace nfs::speech
