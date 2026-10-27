#pragma once
#include "car_initialization_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
namespace nfs::game::world::vehicle_setup {
const gui::String *parameter_filename(const ini::Reader *);
const gui::String *parameter_directory(const ini::Reader *);
void report_missing_parameter(const ini::Reader *, const char *, const char *);
Byte read_parameter_float(ini::Reader *, const char *, const char *, float *);
Byte read_parameter_string(ini::Reader *, const char *, const char *, char *, Dword);
bool read_basic_parameters(ini::Reader *, void *, const char *);
bool read_engine_parameters(ini::Reader *, void *, const char *);
bool read_transmission_parameters(ini::Reader *, void *, const char *, Byte);
bool read_brake_parameters(ini::Reader *, void *, const char *);
bool read_tire_attachment(ini::Reader *, void *, void *, const char *);
bool read_tire_parameters(ini::Reader *, void *, const char *, int);
bool load_physical_parameters(void *, const char *, Dword, Dword);
void set_engine_audio_tag(void *, const char *);
void clear_transmission_curve(void *);
Extended clamp_parameter(float, float, float);
void clear_tire_lateral_curve(void *);
void clear_tire_aligning_curve(void *);
void copy_tire_lateral_curve(void *, int, const void *, Dword);
void copy_tire_aligning_curve(void *, int, const void *, Dword);
extern const Dword tire_presets[754];
} // namespace nfs::game::world::vehicle_setup
