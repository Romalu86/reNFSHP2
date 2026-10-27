#include "mad_codec.hpp"
namespace nfs::movies::mad::codec {
// Shared MAD decoder state.
alignas(8) Byte decoder_storage[6692]{};
#include "mad_transform_data.inc"
#include "mad_codewords.inc"
Dword &bit_window = *reinterpret_cast<Dword *>(decoder_storage);
int &available_bits = *reinterpret_cast<int *>(decoder_storage + 1816);
Byte *&bit_cursor = *reinterpret_cast<Byte **>(decoder_storage + 264);
Dword &predicted = *reinterpret_cast<Dword *>(decoder_storage + 6688);
Dword &tables_initialized = *reinterpret_cast<Dword *>(decoder_storage + 784);
} // namespace nfs::movies::mad::codec
