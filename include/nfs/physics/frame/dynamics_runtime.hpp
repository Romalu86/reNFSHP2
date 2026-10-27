#pragma once
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::game::runtime::physics {
float *copy_triplet(float *, const float *);
float *construct_triplet(float *, float, float, float);
float *zero_triplet(float *);
float *accumulate_triplet(float *, const float *);
float *sum_triplets(float *, const float *, const float *);
float *scale_triplet(float *, const float *, float);
float *divide_triplet(float *, const float *, float);
float *weighted_history(const float *, float *);
void subtract_triplets(const float *, const float *, float *);
int contact_friction(const float *, float, const float *, const float *, float *);
void contact_arm(void *, const float *, float *);
extern int contact_iterations;
extern const float pair_friction[2];
float *triangle_normal(float *, const float *);
int triangle_support(void *, const float *, float);
int surface_height(void *, float *);
namespace pending {
extern const Dword support_triangles[64 * 3];
extern const char shape_source_path[];
extern const char shape_error_message[];
extern const char *&error_source;
extern Dword &physics_assertion_line;
void prepare_error();
void report_runtime_error(const char *, ...);
} // namespace pending
} // namespace nfs::game::runtime::physics
namespace nfs::sound {
struct VoiceParameters {
    alignas(4) Byte bytes[24];
};
struct BankField {
    Byte *cursor;
    Dword tag, value;
    Byte *payload;
    Dword length;
};
int read_bank_integer(const Byte *, Dword);
int next_bank_field(BankField *);
int initialize_parameters(VoiceParameters *);
Dword play_bank(int, int, const VoiceParameters *);
int bank_status(int);
void *bank_entry(void *, int);
Byte next_voice_group();
int play_layers(void *, void *, int, int, const VoiceParameters *);
void dispatch_layer_callbacks(void *, Dword);
int random_variation(int);
namespace pending {
extern Dword &bank_limits;
extern Dword voice_group;
extern Dword audio_random_state[6];
extern const Dword default_envelope[2];
extern const std::uint16_t pan_lookup[128];
int decode_layers(void **, void *);
int create_layer(std::uint16_t, std::uint16_t, void *, const VoiceParameters *, void *,
                          int, int, int);
Dword next_audio_random_value();
} // namespace pending
} // namespace nfs::sound
namespace nfs::game::runtime::race_audio::pending {
extern Dword &race_audio_muted;
extern Dword &variation_index;
extern Byte *const bank_bindings;
} // namespace nfs::game::runtime::race_audio::pending
