#include "basis_universal/transcoder/basisu_transcoder.h"

#include "tonemap.h"
#include "compress.h"

// ACES luminance-only
//
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
glm::vec3 tonemapPixelAcesLuminance(glm::vec3 x) {
  const float a = 2.51f;
  const float b = 0.03f;
  const float c = 2.43f;
  const float d = 0.59f;
  const float e = 0.14f;
  const float W = 0.4f * 11.2f;  // White point
  const float s = 1.0f/((W*(a*W + b))/(W*(c*W + d) + e));

  x = 0.4f*x;  // Roughly match other curves
  return s * (x*(a*x + b))/(x*(c*x + d) + e);
}

// Filmic tonemapper
// http://filmicworlds.com/blog/filmic-tonemapping-operators/
glm::vec3 tonemapPixelFilmic(glm::vec3 x) {
  const float A = 0.15f;
  const float B = 0.50f;
  const float C = 0.10f;
  const float D = 0.20f;
  const float E = 0.02f;
  const float F = 0.30f;
  const float W = 3.0f * 11.2f;  // White point
  const float s = 1.0f/( ((W*(A*W + C*B) + D*E) / (W*(A*W + B) + D*F)) - E/F );

  x = 3.0f*x;  // Roughly match other curves
  //return glm::vec3(1.0f, 0.0f, 0.0f);
  return s*( ((x*(A*x + C*B) + D*E) / (x*(A*x + B) + D*F)) - E/F );
}

// Khronos PBR Neutral tonemapper
// https://github.com/KhronosGroup/ToneMapping/tree/main/PBR_Neutral
glm::vec3 tonemapPixelPBRNeutral(glm::vec3 c) {
  const float F = 0.04f;
  const float Ks = 0.8f - F;
  const float Kd = 0.15f;

  const float x = glm::min(c.r, glm::min(c.g, c.b));
  const float f = (x > (2.0f * F)) ? (F) : (x - ((x*x)/(4.0f*F)));
  const float p = glm::max(c.r - f, glm::max(c.g - f, c.b - f));
  const float Pn = 1.0f - (((1.0f-Ks)*(1.0f-Ks))/((p+1.0f)-(2.0f*Ks)));
  const float g = (1.0f)/(Kd*(p-Pn)+1.0f);

  glm::vec3 Cf = glm::vec3(c.r - f, c.g - f, c.b - f);

  if(p > Ks) {
    return (Cf * ((Pn/p)*g) + glm::vec3(Pn, Pn, Pn) * (1.0f-g));
  } else {
    return Cf;
  }
}

glm::vec3 tonemapNone(glm::vec3 x) {
  return glm::vec3();
}

glm::vec3 (*tonemapFunctions[])(glm::vec3) = {
  tonemapNone,
  tonemapPixelAcesLuminance,
  tonemapPixelFilmic,
  tonemapPixelPBRNeutral
};

void tonemapKtx2File(void *ktx2Blob, size_t size, IBLLib::OutputFormat format, TonemapType tonemapType) {
  basist::ktx2_header ktx2Header = {};
  memcpy(&ktx2Header, ktx2Blob, sizeof(ktx2Header));

  std::vector<basist::ktx2_level_index> mipLevelInfos;
  initMipLevelInfos(ktx2Header, ktx2Blob, size, ktx2Header.m_level_count, mipLevelInfos);

  size_t mipImageSize = mipLevelInfos[0].m_byte_length;

  size_t bpp = 0;

  switch(format) {
  case(IBLLib::OutputFormat::R16G16B16A16_SFLOAT): {
    bpp = 8;
    break;
  }
  case(IBLLib::OutputFormat::R32G32B32A32_SFLOAT): {
    bpp = 16;
    break;
  }
  }

  size_t textureWidth = ktx2Header.m_pixel_width;
  size_t textureHeight = ktx2Header.m_pixel_height;
  size_t textureCubeFaces = ktx2Header.m_face_count;
  size_t textureArrayLayers = ktx2Header.m_layer_count;
  size_t textureMipLevels = ktx2Header.m_level_count;

  if(textureArrayLayers == 0) {
    textureArrayLayers = 1;
  }

  for(uint32_t layerIndex = 0; layerIndex < textureArrayLayers; layerIndex++) {
    for(uint32_t faceIndex = 0; faceIndex < textureCubeFaces; faceIndex++) {
      for(uint32_t mipIndex = 0; mipIndex < textureMipLevels; mipIndex++) {
        size_t width = textureWidth >> mipIndex;
        size_t height = textureHeight >> mipIndex;

        mipImageSize = width * height * bpp;

        size_t mipImageSizePixels = width * height;

        glm::vec4 *src = 
          reinterpret_cast<glm::vec4 *>(
            static_cast<uint8_t *>(ktx2Blob) +
            (
              mipLevelInfos[mipIndex].m_byte_offset +
              (layerIndex * textureCubeFaces * mipImageSize) +
              (faceIndex * mipImageSize)
            )
          );

        for(size_t i = 0; i < mipImageSizePixels; i++) {
          glm::vec3 color = tonemapFunctions[tonemapType](src[i].xyz());
          src[i] = glm::vec4(color, 1.0f);
        }
      }
    }
  }
}
