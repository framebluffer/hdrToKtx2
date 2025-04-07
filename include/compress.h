#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <stdint.h>
#include "basis_universal/transcoder/basisu_transcoder.h"
#include "basis_universal/encoder/basisu_comp.h"
#include "glTF-IBL-Sampler/GltfIblSampler.h"

bool initMipLevelInfos(const basist::ktx2_header &ktx2Header, const void *ktx2Data, size_t dataSize, uint32_t mipLevels, std::vector<basist::ktx2_level_index> &outMipLevelInfos);
bool createBasisCompressedHDRTexture(std::vector<uint8_t> &data, const char *outputPath, IBLLib::OutputFormat format);

#endif // __COMPRESS_H__
