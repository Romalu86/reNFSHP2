#include "nfs/audio/speech/speech_game.hpp"
namespace nfs::lifecycle::construction {
kernel::Dword speech_handles[10800]{};
}
namespace nfs::speech::game {
const char *languages[7] = {"english", "french",  "german", "italian",
                            "spanish", "english", "korean"};
Dword voice_masks[6] = {1, 2, 4, 8, 16, 32}, paired_voice_masks[6] = {1, 2, 4, 8, 16, 32},
     dispatch_masks[4] = {2, 8, 4, 32};
} // namespace nfs::speech::game
