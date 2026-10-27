#include "nfs/audio/effects/sound_reverb.hpp"
#include "nfs/audio/codecs/sound_codecs.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <algorithm>
namespace nfs::sound::backend::effects {
void network_samples(void *, int, const float *, float *);
using Node = pending::Processor;
using Mix = void(__cdecl *)(int, float, const float *, float *);

int route_channel(int layout, int channels, int tag) {
    return constant<Byte>(7127613 + unsigned((layout * 6 + channels) * 6 + tag));
}

void destroy_graph() {
    int count = nfs::sound::read_bank_integer(at<Byte *>(8718704) + 4 + at<int>(8718720) * 4, 4);
    for (int i = 0; i < count; ++i) {
        auto node = at<void **>(8718708)[i];
        if (auto destroy = field<void(__cdecl *)(void *)>(node, 4))
            destroy(node);
        if (at<void **>(8718708)[i]) {
            release(at<void **>(8718708)[i]);
            at<void **>(8718708)[i] = nullptr;
        }
    }
    if (auto nodes = at<void **>(8718708)) {
        release(nodes);
        at<void *>(8718708) = nullptr;
    }
    auto *roots = at<void **>(8718724);
    for (unsigned i = 0; i < 6; ++i)
        if (roots[i])
            roots[i] = nullptr;
    if (roots) {
        release(roots);
        at<void *>(8718724) = nullptr;
    }
    at<Dword>(8718292) = at<Dword>(8718692) = at<Dword>(8718716) = at<Dword>(8718712) =
        at<Dword>(8718696) = 0;
}

void construct_graph(int rate) {
    at<void *>(8718704) = storage + 12;
    auto *roots = static_cast<void **>(allocate(28));
    at<void **>(8718724) = roots;
    zero_memory(roots, 28);
    int layout = at<int>(8718732);
    auto *cursor = at<Byte *>(8718292);
    at<Byte *>(8718712) = cursor;
    at<void *>(8718716) = at<void *>(8718692);
    int count = nfs::sound::read_bank_integer(at<Byte *>(8718704) + 4 + layout * 4, 4);
    auto *nodes = static_cast<void **>(allocate(Dword(count) * 4));
    at<void **>(8718708) = nodes;
    for (int i = 0; i < count; ++i) {
        void *node = allocate(constant<Dword>(7127616 + cursor[0] * 4));
        nodes[i] = node;
        int parameters[4];
        zero_memory(parameters, 16);
        switch (cursor[0]) {
        case 0:
            construct_identity(node);
            break;
        case 1:
            parameters[0] = int(Dword(nfs::sound::read_bank_integer(cursor + 4, 2)) << 8);
            parameters[1] = int(Dword(rate) << 8);
            parameters[2] = nfs::sound::read_bank_integer(cursor + 6, 2);
            pending::create_filter(node);
            pending::update_filter(node, parameters);
            break;
        case 2:
            parameters[0] = int(Dword(nfs::sound::read_bank_integer(cursor + 2, 2)) << 8);
            parameters[1] = int(Dword(rate) << 8);
            construct_highpass(node);
            set_highpass(node, parameters);
            break;
        case 3:
            parameters[0] = int(Dword(nfs::sound::read_bank_integer(cursor + 2, 2)) << 8);
            parameters[1] = int(Dword(rate) << 8);
            pending::create_effect(node);
            pending::change_effect(node, parameters);
            break;
        case 4:
            parameters[0] = int(Dword(nfs::sound::read_bank_integer(cursor + 4, 2)) << 8);
            parameters[1] = int(Dword(nfs::sound::read_bank_integer(cursor + 6, 2)) << 8);
            parameters[2] = int(Dword(rate) << 8);
            construct_bandpass(node);
            set_bandpass(node, parameters);
            break;
        case 5:
            parameters[0] = int(Dword(nfs::sound::read_bank_integer(cursor + 2, 2)) << 8);
            parameters[1] = int(Dword(rate) << 8);
            parameters[2] = int(Dword(nfs::sound::read_bank_integer(cursor + 4, 2)) << 8);
            parameters[3] = nfs::sound::read_bank_integer(cursor + 6, 2);
            construct_notch(node);
            set_notch(node, parameters);
            break;
        case 6:
            parameters[0] = nfs::sound::read_bank_integer(cursor + 2, 2);
            construct_gain(node);
            set_gain(node, parameters);
            break;
        case 7:
            parameters[0] = nfs::sound::read_bank_integer(cursor + 2, 2);
            parameters[1] = int(Dword(rate) << 8);
            parameters[2] = int(Dword(nfs::sound::read_bank_integer(cursor + 4, 2)) << 8);
            construct_delay(node);
            set_delay(node, parameters);
            break;
        case 8:
            construct_sum(node);
            break;
        case 9:
            construct_cache(node);
            break;
        }
        if (cursor[1])
            roots[route_channel(layout, pending::mix_modes, cursor[1])] = node;
        cursor += codecs::table<int>(7129168 + cursor[0] * 4);
    }
    cursor = at<Byte *>(8718716);
    for (int i = 0; i < nfs::sound::read_bank_integer(at<Byte *>(8718704) + 28 + layout * 4, 4); ++i, cursor += 12) {
        int destination = nfs::sound::read_bank_integer(cursor + 4, 4);
        int source = nfs::sound::read_bank_integer(cursor, 4);
        connect(nodes[source], nodes[destination], cursor[8], cursor[9]);
    }
}

void configure_graph(int rate, const void *descriptor) {
    at<Dword>(8718696) = 1;
    copy_memory(storage + 12, descriptor, 56);
    int layout = int(pending::mix_modes) - 1;
    auto *entry = storage + 20 + layout * 4;
    if (!nfs::sound::read_bank_integer(entry, 4)) {
        while (!nfs::sound::read_bank_integer(entry, 4)) {
            entry -= 4;
            --layout;
        }
        at<int>(8718720) = layout + 1;
    } else
        at<int>(8718720) = pending::mix_modes;
    at<int>(8718732) = at<int>(8718720);
    auto *cursor = static_cast<const Byte *>(descriptor) + 56;
    for (int i = 0; i < layout; ++i) {
        int nodes = nfs::sound::read_bank_integer(storage + 20 + i * 4, 4);
        while (nodes-- > 0)
            cursor += codecs::table<int>(7129168 + cursor[0] * 4);
        cursor += nfs::sound::read_bank_integer(storage + 44 + i * 4, 4) * 12;
    }
    at<const Byte *>(8718292) = cursor;
    int nodes = nfs::sound::read_bank_integer(storage + 20 + layout * 4, 4);
    while (nodes-- > 0)
        cursor += codecs::table<int>(7129168 + cursor[0] * 4);
    at<const Byte *>(8718692) = cursor;
    construct_graph(rate);
}

int preset_integer_root(int value) {
    int i = 0;
    for (;;) {
        int square = int(Dword(i) * Dword(i));
        ++i;
        if (square == value)
            return i - 1;
        if (square > value)
            return i - 2;
    }
}

int preset_delay_samples(int rate, int milliseconds) {
    int value = int(Dword(rate) * Dword(milliseconds)) / 1000;
    if (value < 3)
        value = 3;
    for (;;) {
        int limit = preset_integer_root(value) + 1;
        bool composite = false;
        for (int divisor = 2; divisor <= limit; ++divisor) {
            if (value % divisor == 0) {
                composite = true;
                break;
            }
            if (divisor == limit)
                return value;
        }
        if (composite)
            ++value;
    }
}

void configure(int rate, const void *descriptor) {
    reset();
    mixer<void(__cdecl *)(const void *)>(8717712) = reset_history;
    auto *bytes = static_cast<const Byte *>(descriptor);
    if (!bytes[2])
        mixer<void(__cdecl *)(int)>(8717716) = process_delays;
    else if (bytes[2] == 10) {
        configure_graph(rate, descriptor);
        mixer<void(__cdecl *)(int)>(8717716) = process_graph;
        mixer<Byte>(8717711) = 1;
        return;
    }


    int record = 0;
    for (unsigned group = 0; group < 3; ++group) {
        at<int>(8718224 + group * 4) = bytes[4 + group];
        for (int n = 0; n < bytes[4 + group]; ++n, ++record) {
            unsigned state = 8718296 + record * 36;
            auto gain = static_cast<long double>(static_cast<std::int8_t>(bytes[11 + record * 4])) *
                        network_scale;
            at<float>(state + 28) = float(gain);
            at<float>(state + 32) = float(static_cast<long double>(1) - gain);
            at<float>(state + 28) = float(
                (static_cast<long double>(bytes[7] + 100) * at<float>(state + 28)) * preset_scale);
            at<float>(state + 32) = float(
                (static_cast<long double>(bytes[7] + 100) * at<float>(state + 32)) * preset_scale);
            auto feedback =
                static_cast<long double>(static_cast<std::int8_t>(bytes[10 + record * 4])) *
                network_scale;
            at<float>(state + 16) = float(feedback);
            at<float>(state + 20) = float(-feedback);
            int samples = preset_delay_samples(rate, nfs::sound::read_bank_integer(bytes + 8 + record * 4, 2));
            auto buffer = allocate(Dword(samples) * 4);
            at<void *>(state + 12) = buffer;
            zero_memory(buffer, Dword(samples) * 4);
            at<Dword>(state) = 0;
            at<Dword>(state + 24) = 0;
            at<int>(state + 8) = samples;
            at<int>(state + 4) = samples;
        }
    }
    reset_history(descriptor);
    mixer<Byte>(8717711) = 2;
}

void reset() {
    mixer<Dword>(8717712) = mixer<Dword>(8717716) = 0;
    if (at<Dword>(8718696))
        destroy_graph();
    for (unsigned address = 8718308; address < 8718704; address += 36) {
        if (auto buffer = at<void *>(address)) {
            release(buffer);
            at<void *>(address) = nullptr;
        }
    }
    mixer<Byte>(8717711) = 0;
}
// The descriptor scan's saved stride is twenty times its table entry.
void reset_history(const void *descriptor) {
    at<Dword>(8721808) = 3000;
    zero_memory(storage + 512, 1024);
    if (field<Byte>(descriptor, 2) == 10) {
        at<Dword>(8718728) = 0;
        auto *cursor = at<Byte *>(8718712);
        while (*cursor) {
            cursor += codecs::table<int>(7129168 + *cursor * 4) * 20;
            ++at<Dword>(8718728);
        }
    }
}

void process_graph(int count) {
    void *input = mixer<void *>(8717992);
    auto *output = mixer<float *>(8717996);
    int visited[6] = {};
    ++at<Dword>(8721808);
    if (at<int>(8721808) >= 3000)
        return;
    if (nfs::sound::read_bank_integer(at<Byte *>(8718704) + 4 + at<int>(8718732) * 4, 4)) {
        bind_input(at<void **>(8718708)[at<int>(8718728)], storage + 512);
        for (unsigned channel = 0; channel < pending::mix_modes; ++channel) {
            if (!visited[channel]) {
                void *root = at<void **>(8718724)[channel + 1];
                if (!root)
                    continue;
                field<Node>(root, 0)(root, count, input, output,
                                     Byte(channel + 1));
                mixer<Mix>(8718204)(count, 1, output,
                                     mixer<float *>(8718016 + channel * 4));
                visited[channel] = 1;
                const auto *routes = tables + (7127872 - 7127332) +
                    unsigned((int(channel) +
                              (int(pending::mix_modes) - 7 + at<int>(8718732) * 6) * 6) * 6);
                for (unsigned other = 0; other < pending::mix_modes; ++other) {
                    const auto target = static_cast<std::int8_t>(routes[other]);
                    if (target >= 0) {
                        mixer<Mix>(8718204)(count, 1, output,
                                             mixer<float *>(8718016 + target * 4));
                        visited[target] = 1;
                    }
                }
            }
        }
    }
    zero_memory(storage + 512, Dword(count) * 4);
}

void process_delays(int count) {
    ++at<Dword>(8721808);
    if (at<int>(8721808) >= 3000)
        return;
    float *input = reinterpret_cast<float *>(storage + 512), *output = input;
    int record = 0;
    for (unsigned group = 0; group < 3; ++group)
        for (int n = 0; n < at<int>(8718224 + group * 4); ++n, ++record) {
            if (group == 1) {
                if (n == 0)
                    output = reinterpret_cast<float *>(storage + 1536);
                else if (n == 1)
                    input = reinterpret_cast<float *>(storage + 1536);
            } else if (group == 2) {
                if (n == 0) {
                    input = reinterpret_cast<float *>(storage + 512);
                    output = reinterpret_cast<float *>(storage + 2560);
                } else if (n == 1)
                    input = reinterpret_cast<float *>(storage + 2560);
            }
            void *state = storage + 72 + record * 36;
            int remaining = count;
            while (remaining > 0) {
                if (field<int>(state, 0) < 1)
                    field<int>(state, 0) = field<int>(state, 8);
                if (field<int>(state, 4) < 1)
                    field<int>(state, 4) = field<int>(state, 8);
                int samples =
                    std::min(remaining, std::min(field<int>(state, 0), field<int>(state, 4)));
                network_samples(state, samples, input + count - remaining,
                                output + count - remaining);
                remaining -= samples;
                field<int>(state, 0) -= samples;
                field<int>(state, 4) -= samples;
            }
        }
    for (unsigned channel = 0; channel < pending::mix_modes; ++channel)
        if (channel != 5) {
            unsigned address = (channel & 1) && at<int>(8718228) ? 8720784
                               : at<int>(8718232)                ? 8719760
                                                                  : 8718736;
            mixer<Mix>(8718204)(count, 1,
                                 reinterpret_cast<const float *>(storage + address - 8718224),
                                 mixer<float *>(8718016 + channel * 4));
        }
    zero_memory(storage + 512, Dword(count) * 4);
}
} // namespace nfs::sound::backend::effects
