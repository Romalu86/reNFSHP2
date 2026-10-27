#include "nfs/audio/speech/speech_interpreter.hpp"
#include "nfs/audio/speech/speech_queue.hpp"
#include "nfs/audio/speech/speech_rules.hpp"

namespace nfs::speech {
namespace {
template <class T> const T &field(const Byte *bytes, int offset) {
    return *reinterpret_cast<const T *>(bytes + offset);
}
int aligned_bindings(int count) {
    return ((count + 1) * 3) & ~3;
}
int presence_descriptor_offset(const Byte *rule) {
    const int encoded = static_cast<std::int8_t>(rule[7]);
    return 12 + ((rule[6] * 2 + 3) & ~3) + aligned_bindings(encoded & 15) +
           aligned_bindings(encoded >> 4);
}
} // namespace


int evaluate_rule_request(Dword *request) {
    Dword event = request[0];
    int voice;
    const Byte *rule = find_event_definition(&event);
    if (!rule || !read_event_voice(&event, &voice) || !rule_history_allows(rule, voice))
        return 0;
    int mode = state<int>(8767968 + voice * 4);
    if (state<int>(8768096) == 1) {
        mode = static_cast<int>(static_cast<Dword>(mode) + 1);
        if (mode > 2)
            mode = 2;
    }
    int threshold = 0;
    if ((rule[10] & 1) == 0 && mode == 1)
        mode = 0;
    else if (mode != 0)
        threshold = read_event_threshold(&event);
    Byte unavailable = 0;
    const Byte accepted = evaluate_rule_predicates(&event, rule, request, &unavailable);
    Byte order[100];
    order_rule_variants(rule, order);
    const int count = rule[6];
    if (count == 0)
        return -1;
    const bool check_presence = (rule[10] & 8) != 0;
    Dword presence = 0;
    if (check_presence) {
        const Dword descriptor = field<Dword>(rule, presence_descriptor_offset(rule));
        for (int i = 0; i < 20; ++i)
            if (request_presence_matches(descriptor, static_cast<int>(request[i + 1]), i))
                presence |= Dword{1} << i;
    }
    int selected = 0;
    for (int i = 0; i < count; ++i) {
        const unsigned index = order[i];
        if (index >= static_cast<unsigned>(count))
            return selected;
        const Byte *variant = rule + field<std::uint16_t>(rule, 12 + index * 2) * 4;
        if (!variant_probability_allows(variant) || !variant_request_allows(rule, variant, request))
            continue;
        if (check_presence &&
            field<std::uint16_t>(rule, presence_descriptor_offset(rule) + 4 + index * 2) !=
                static_cast<std::uint16_t>(presence))
            continue;
        RuleSelection selection;
        clear_rule_selection(&selection);
        if (gather_rule_candidates(&selection, variant, request, accepted, unavailable, mode) <= 0)
            continue;
        selected = select_rule_phrases(&selection, variant, mode, threshold);
        if (selected != 0) {
            append_selected_rule(&selection, voice, rule, variant, static_cast<Byte>(index),
                                 request);
            return selected;
        }
    }
    return selected;
}


int evaluate_queued_request(int slot) {
    auto *request = &state<Dword>(8768116 + slot * 96);
    int result = evaluate_rule_request(request);
    if (result == 0 && state<RequestRetry>(8767952)) {
        int attempt = 0;
        int retry;
        do {
            retry = state<RequestRetry>(8767952)(attempt++, request);
            if (retry >= 0)
                result = evaluate_rule_request(request);
        } while (result == 0 && retry > 0);
    }
    return result < 0 ? 0 : result;
}
} // namespace nfs::speech
