#pragma once

namespace nfs::game::audio::vehicle {
struct Progress;

namespace progress_math {
void weight_native80(const Progress *progress, void *output);
void fraction_native80(const Progress *progress, void *output);
void combined_native80(void *output);
float loading_completion_f32();
}
}
