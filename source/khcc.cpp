// TODO(Markus): FIx multiple env maps argument bug

#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>

#define BASISU_SUPPORT_SSE 1
#define BASISU_SUPPORT_OPENCL 1

const bool BASISU_USE_OPENCL = false;

#include "glTF-IBL-Sampler/GltfIblSampler.h"
#include "basis_universal/transcoder/basisu_transcoder.h"
#include "glTF-IBL-Sampler/ResultType.h"
#include "ktx/ktx.h"
#include "ktx/vkformat_enum.h"

#define STB_IMPLEMENTATION
#include "stb/stb_image.h"

#include "arguments.h"
#include "file.h"
#include "compress.h"
#include "tonemap.h"

#define ENABLE_IBL_SAMPLER_DEBUG_OUTPUT 0

void printUsage();

int main(int argc, char **argv) {
  if (!getArguments(argc, argv)) {
    printUsage();
    return 1;
  }

  bool basisuInitialized = false;

  const char *nonBasisuString = "__KHCCEXE__TMP__NON_BASISU_";

  // IRRADIANCE MAP
  if(outputIrradiance[0] != '\0') {
    std::string strOutputIrradiance(outputIrradiance);

    size_t irradiancePathEnd = 0;
    irradiancePathEnd = strOutputIrradiance.rfind("/");
    if(irradiancePathEnd == std::string::npos) {
      irradiancePathEnd = strOutputIrradiance.rfind("\\");
      if(irradiancePathEnd != std::string::npos) {
        irradiancePathEnd++;
      } else {
        irradiancePathEnd = 0;
      }
    }

    strOutputIrradiance.insert(irradiancePathEnd, nonBasisuString);
    const char *outputIrradianceNonBasisu = strOutputIrradiance.c_str();

    IBLLib::Result irradianceMapSampleResult = {};
    irradianceMapSampleResult = IBLLib::sample(
      inputHDR,
      outputIrradianceNonBasisu,
      nullptr, // LUT path
      IBLLib::Distribution::Lambertian,
      irradianceMapResolution,
      1, // mip count
      irradianceMapSampleCount,
      irradianceMapIntermediaryFormat,
      0.0f, // LOD bias
      ENABLE_IBL_SAMPLER_DEBUG_OUTPUT);

    if (irradianceMapSampleResult != IBLLib::Result::Success) {
      printf("glTF-IBL-Sampler Failed to create intermediary irradiance map\n");
      return 1;
    }

    if(compress) {
      printf("Read non basis irradiance texture data to buffer\n");
      std::vector<uint8_t> intermediaryIrradianceBlob;
      if (!readFile(outputIrradianceNonBasisu, intermediaryIrradianceBlob)) {
        printf("Failed to read intermediary irradiance file into buffer\n");
        return 1;
      }

      std::remove(outputIrradianceNonBasisu);

      if (!basisuInitialized) {
        basisu::basisu_encoder_init(BASISU_USE_OPENCL, false);
        basisuInitialized = true;
      }

      if (!createBasisCompressedHDRTexture(intermediaryIrradianceBlob, outputIrradiance, irradianceMapIntermediaryFormat)) {
        printf("Failed to create basis compressed irradiance texture\n");
        return 1;
      }
    }
  }

  // RADIANCE MAP
  if(outputRadiance[0] != '\0') {
    std::string strOutputRadiance(outputRadiance);

    size_t radiancePathEnd = 0;
    radiancePathEnd = strOutputRadiance.rfind("/");
    if(radiancePathEnd == std::string::npos) {
      radiancePathEnd = strOutputRadiance.rfind("\\");
      if(radiancePathEnd != std::string::npos) {
        radiancePathEnd++;
      } else {
        radiancePathEnd = 0;
      }
    }

    strOutputRadiance.insert(radiancePathEnd, nonBasisuString);
    const char *outputRadianceNonBasisu = strOutputRadiance.c_str();

    IBLLib::Result radianceMapSampleResult = {};
    radianceMapSampleResult = IBLLib::sample(
      inputHDR,
      outputRadianceNonBasisu,
      nullptr, // LUT path
      IBLLib::Distribution::GGX,
      radianceMapResolution,
      radianceMapMipCount,
      radianceMapSampleCount,
      radianceMapIntermediaryFormat,
      radianceMapLODBias,
      ENABLE_IBL_SAMPLER_DEBUG_OUTPUT);

    if (radianceMapSampleResult != IBLLib::Result::Success) {
      printf("glTF-IBL-Sampler Failed to create intermediary radiance map\n");
      return 1;
    }

    if(compress) {
      printf("Read non basis radiance texture data to buffer\n");
      std::vector<uint8_t> intermediaryRadianceBlob;
      if (!readFile(outputRadianceNonBasisu, intermediaryRadianceBlob)) {
        printf("Failed to read intermediary radiance file into buffer\n");
        return 1;
      }

      std::remove(outputRadianceNonBasisu);

      if (!basisuInitialized) {
        basisu::basisu_encoder_init(BASISU_USE_OPENCL, false);
        basisuInitialized = true;
      }

      if (!createBasisCompressedHDRTexture(intermediaryRadianceBlob, outputRadiance, radianceMapIntermediaryFormat)) {
        printf("Failed to create basis compressed radiance texture\n");
        return 1;
      }
    }
  }

  // LUT
  if(outputLUT[0] != '\0') {
    IBLLib::Result LUTSampleResult = {};
    LUTSampleResult = IBLLib::sample(
      inputHDR,
      "__KHCCEXE__TMP__throwaway_radiance.ktx2", // Env map path
      outputLUT, // LUT path
      IBLLib::Distribution::GGX,
      LUTResolution,
      1, // mip count
      LUTSampleCount,
      radianceMapIntermediaryFormat,
      0.0f, // LOD bias
      ENABLE_IBL_SAMPLER_DEBUG_OUTPUT);

    if (LUTSampleResult != IBLLib::Result::Success) {
      printf("glTF-IBL-Sampler Failed to create intermediary radiance map\n");
      return 1;
    }

    std::remove("__KHCCEXE__TMP__throwaway_radiance.ktx2");

    int lutWidth, lutHeight, lutComponents;
    uint8_t *lutData = stbi_load(outputLUT, &lutWidth, &lutHeight, &lutComponents, STBI_rgb);
    if(lutData == nullptr) {
      printf("Failed to read LUT from file\n");
      return 1;
    }

    ktxTexture2 *lutTexture;
    ktxTextureCreateInfo lutCreateInfo;
    KTX_error_code lutResult;

    lutCreateInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
    lutCreateInfo.baseWidth = LUTResolution;
    lutCreateInfo.baseHeight = LUTResolution;
    lutCreateInfo.baseDepth = 1;
    lutCreateInfo.numDimensions = 2;
    lutCreateInfo.numLevels = 1;
    lutCreateInfo.numLayers = 1;
    lutCreateInfo.numFaces = 1;
    lutCreateInfo.isArray = KTX_FALSE;
    lutCreateInfo.generateMipmaps = KTX_FALSE;

    lutResult = ktxTexture2_Create(&lutCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &lutTexture);
    if(lutResult != KTX_SUCCESS) {
      printf("Failed to create KTX2 texture for LUT\n");
      printf("%d\n", lutResult);
      return 1;
    }

    lutResult = ktxTexture_SetImageFromMemory(ktxTexture(lutTexture), 0, 0, 0, lutData, lutWidth * lutHeight * lutComponents);
    if(lutResult != KTX_SUCCESS) {
      printf("Failed to create ktx2 texture for LUT\n");
      return 1;
    }
    ktxTexture_WriteToNamedFile(ktxTexture(lutTexture), outputLUT);
    ktxTexture_Destroy(ktxTexture(lutTexture));
  }

  if(outputSkybox[0] != '\0') {
    // TODO(Markus): IBLSampler not able to produce sane result with GGX sampling if only one miplevel???
    // TODO(Markus): skybox texture contains one extra miplevel that is unneccessary.
    // Not computed correctly for use as skybox.
    // This can be fixed in the future.
    // SKYBOX
    IBLLib::Result skyboxSampleResult = {};
    skyboxSampleResult = IBLLib::sample(
      inputHDR,
      outputSkybox,
      nullptr, // LUT
      IBLLib::Distribution::GGX,
      skyboxResolution,
      2, // mip count
      skyboxSampleCount,
      skyboxFormat, // NOTE(Markus): Apparently this bein R32G32B32A32_SFLOAT is a problem when passing to basisu compressor.
      0.0f, // LOD bias
      ENABLE_IBL_SAMPLER_DEBUG_OUTPUT);

    if (skyboxSampleResult != IBLLib::Result::Success) {
      printf("glTF-IBL-Sampler Failed to create skybox\n");
      return 1;
    }

    if(compress || skyboxTonemapper != NONE) {
      std::vector<uint8_t> skyboxBlob;
      if(!readFile(outputSkybox, skyboxBlob)) {
        printf("Failed to read skybox file into buffer\n");
        return 1;
      }

      std::remove(outputSkybox);
        
      if(skyboxTonemapper != NONE) {
        tonemapKtx2File(skyboxBlob.data(), skyboxBlob.size(), skyboxFormat, skyboxTonemapper);
      }

      if(compress) {
        if(!basisuInitialized) {
          basisu::basisu_encoder_init(BASISU_USE_OPENCL, false);
          basisuInitialized = true;
        }

        if(!createBasisCompressedHDRTexture(skyboxBlob, outputSkybox, skyboxFormat)) {
          printf("Failed to create basis compressed skybox texture\n");
          return 1;
        }
      } else {
        writeFile(outputSkybox, skyboxBlob.data(), skyboxBlob.size());
      }
    }
  }

  std::remove("pipeline.cache");

  printf("success\n");
  return 0;
}

void printUsage() {
  printf("khcc Generates supercompressed UASTC HDR Irradiance, Radiance + Lookup table - environment maps in .KTX2 format from an input Equirectangular latlong .HDR image\n\n");
  printf("Usage:\n\n");
  printf("At least --inputHDR, and one of --outputIrradiance | --outputRadiance | --outputLUT | --outputSkybox must be specified.\n");
  printf("khcc.exe --inputHDR <inputHDRPath.hdr> --outputIrradiance <outputIrradiancePath.ktx2> --outputRadiance <outputRadiancePath.ktx2> --outputLUT <outputLUTPath.ktx2> --outputSkybox <outputSkyboxPath.ktx2> [--<argument> <argumentValue> ...]\n");
  printf("\n");
  printf("Arguments:\n\n");
  //printf("%-30s\t%60s\n", "--irradianceDistribution", "Lambertian | Charlie | GGX");
  //printf("%-30s\t%60s\n", "--radianceDistribution", "Lambertian | Charlie | GGX");
  printf("%-30s\t%60s\n", "--irradianceResolution", "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--radianceResolution", "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--LUTResolution", "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--skyboxResolution", "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--irradianceSampleCount", "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--radianceSampleCount",  "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--LUTSampleCount",  "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--skyboxSampleCount",  "<Unsigned Integer>");
  //printf("%-30s\t%60s\n", "--irradianceLODBias", "<Float>");
  printf("%-30s\t%60s\n", "--radianceLODBias", "<Float>");
  //printf("%-30s\t%60s\n", "--irradianceMipLevels", "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--radianceMipLevels", "<Unsigned Integer>");
  printf("%-30s\t%60s\n", "--irradianceIntermediaryFormat", "R8G8B8A8_UNORM | R16G16B16A16_SFLOAT | R32G32B32A32_SFLOAT");
  printf("%-30s\t%60s\n", "--radianceIntermediaryFormat", "R8G8B8A8_UNORM | R16G16B16A16_SFLOAT | R32G32B32A32_SFLOAT");
  printf("%-30s\t%60s\n", "--skyboxFormat", "R32G32B32A32_SFLOAT");
  printf("%-30s\t%60s\n", "--skyboxTonemapper", "NONE | ACES | FILMIC | PBR");
  printf("%-30s\t%60s\n", "--compress", "if specified, uses basis compression");
}
