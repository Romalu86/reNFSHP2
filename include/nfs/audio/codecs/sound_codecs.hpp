#pragma once
#include "nfs/audio/backend/sound_ownership.hpp"
namespace nfs::sound::backend::codecs {
extern const Byte saved_tables[2264];
extern Byte predictor_features;
extern const float speech_growth, speech_bias, speech_outer, speech_middle, speech_inner,
    speech_history_scale;
template <class T> const T &table(unsigned va) {
    return field<T>(saved_tables, va - 7129168);
}
Dword read_bits(void *, int);
void skip_bits(void *, int);
void decode_excitation(void *, int, float *);
void initialize_speech(const Byte *, void *);
void decode_speech_block(void *);
void speech_interpolate(float *);
void speech_filter(void *, int, int);
void reflection_coefficients(const float *, float *);
void decode_predictor_scalar(void *);
void decode_predictor_streaming(void *);
void decode_predictor_packed(void *);
void *__fastcall create_predictor(void *, void *);
int __fastcall bind_predictor(void *, void *, const void *, Dword, int);
int __fastcall read_predictor(void *, void *, float **, int);
void *__fastcall save_predictor(void *, void *, void *);
void __fastcall load_predictor(void *, void *, const void *);
} // namespace nfs::sound::backend::codecs
