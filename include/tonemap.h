#ifndef __TONEMAP_H__
#define __TONEMAP_H__

#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp"
#include "glTF-IBL-Sampler/GltfIblSampler.h"

enum TonemapType {
  NONE,
  ACESLUMINANCE,
  FILMIC,
  PBRNEUTRAL
};

glm::vec3 tonemapPixelAcesLuminance(glm::vec3 x);
glm::vec3 tonemapPixelFilmic(glm::vec3 x);
glm::vec3 tonemapPixelPBRNeutral(glm::vec3 c);

void tonemapKtx2File(void *ktx2Blob, size_t size, IBLLib::OutputFormat format, TonemapType tonemapType);

#endif // __TONEMAP_H__
