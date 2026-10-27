#pragma once
#include "nfs/audio/backend/sound_ownership.hpp"
namespace nfs::sound::backend::effects {
extern Byte storage[3588];
extern const Byte tables[1836];
template <class T> T &at(unsigned va) {
    return field<T>(storage, va - 8718224);
}
template <class T> const T &constant(unsigned va) {
    return field<T>(tables, va - 7127332);
}
inline void copy_memory(void *destination, const void *source, Dword bytes) {
    for (Dword i = 0; i < bytes / 4; ++i)
        field<Dword>(destination, i * 4) = field<Dword>(source, i * 4);
    for (Dword i = bytes & ~Dword(3); i < bytes; ++i)
        field<Byte>(destination, i) = field<Byte>(source, i);
}
void reset();
void configure(int, const void *);
void configure_graph(int, const void *);
void construct_graph(int);
void destroy_graph();
void reset_history(const void *);
void process_graph(int);
void process_delays(int);
int construct_cache(void *);
int construct_sum(void *);
int construct_delay(void *);
void set_delay(void *, const int *);
int construct_gain(void *);
void set_gain(void *, const int *);
int construct_bandpass(void *);
void set_bandpass(void *, const int *);
int construct_notch(void *);
void set_notch(void *, const int *);
int construct_highpass(void *);
void set_highpass(void *, const int *);
int construct_identity(void *);
void bind_input(void *, void *);
int connect(void *, void *, int, int);
int delay_samples(int, int);
int integer_root(int);
extern const float root_coefficients[7], denormal_bias, network_bias, notch_limit, network_scale,
    preset_scale, delay_scale;
} // namespace nfs::sound::backend::effects
