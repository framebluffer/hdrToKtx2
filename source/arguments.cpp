#define _CRT_SECURE_NO_WARNINGS

#include "arguments.h"

// Arguments
char inputHDR[STRINGBUF_SIZE] = {};

char outputIrradiance[STRINGBUF_SIZE] = {};
char outputRadiance[STRINGBUF_SIZE] = {};
char outputLUT[STRINGBUF_SIZE] = {};
char outputSkybox[STRINGBUF_SIZE] = {};

IBLLib::Distribution irradianceMapDistributionFunction;
IBLLib::Distribution radianceMapDistributionFunction;

uint32_t irradianceMapResolution;
uint32_t radianceMapResolution;
uint32_t LUTResolution;
uint32_t skyboxResolution;

uint32_t radianceMapMipCount;

uint32_t irradianceMapSampleCount;
uint32_t radianceMapSampleCount;
uint32_t LUTSampleCount;
uint32_t skyboxSampleCount;

IBLLib::OutputFormat irradianceMapIntermediaryFormat;
IBLLib::OutputFormat radianceMapIntermediaryFormat;
IBLLib::OutputFormat skyboxFormat;
TonemapType skyboxTonemapper;

float radianceMapLODBias;

bool outputUASTC = false;
bool outputETC1S = false;
bool compress = false;

bool findArgument(const char *argument, int *argPos, int argc, char **argv) {
  bool found = false;
  int i = 1;
  for(; i < argc; i++) {
    if(!strcmp(argument, argv[i])) {
      found = true;
      break;
    }
  }

  if(argPos != nullptr) {
    *argPos = i;
  }

  return found;
}

bool getArgument(const char *argument, int argc, char **argv, bool required) {
  int i = 0;
  bool found = findArgument(argument, &i, argc, argv);

  if(!found && required) {
    return false;
  }

  if(!strcmp(argument, "--inputHDR")) {
    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }
    strcpy(inputHDR, argv[i+1]);
  } else if(!strcmp(argument, "--outputIrradiance")) {
    if(!found) {
      return true;
    }
    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }
    strcpy(outputIrradiance, argv[i+1]);
  } else if(!strcmp(argument, "--outputRadiance")) {
    if(!found) {
      return true;
    }
    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }
    strcpy(outputRadiance, argv[i+1]);
  } else if(!strcmp(argument, "--outputLUT")) {
    if(!found) {
      return true;
    }
    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }
    strcpy(outputLUT, argv[i+1]);
  } else if(!strcmp(argument, "--outputSkybox")) {
    if(!found) {
      return true;
    }
    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }
    strcpy(outputSkybox, argv[i+1]);
  } else if(!strcmp(argument, "--irradianceDistribution")) {
    if(!found) {
      irradianceMapDistributionFunction = IBLLib::Distribution::Lambertian;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    if(!strcmp(argv[i+1], "Lambertian")) {
      irradianceMapDistributionFunction = IBLLib::Distribution::Lambertian;
    } else if(!strcmp(argv[i+1], "Charlie")) {
      irradianceMapDistributionFunction = IBLLib::Distribution::Charlie;
    } else if(!strcmp(argv[i+1], "GGX")) {
      irradianceMapDistributionFunction = IBLLib::Distribution::GGX;
    } else {
      return false;
    }
  } else if(!strcmp(argument, "--radianceDistribution")) {
    if(!found) {
      radianceMapDistributionFunction = IBLLib::Distribution::GGX;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    if(!strcmp(argv[i+1], "Lambertian")) {
      radianceMapDistributionFunction = IBLLib::Distribution::Lambertian;
    } else if(!strcmp(argv[i+1], "Charlie")) {
      radianceMapDistributionFunction = IBLLib::Distribution::Charlie;
    } else if(!strcmp(argv[i+1], "GGX")) {
      radianceMapDistributionFunction = IBLLib::Distribution::GGX;
    } else {
      return false;
    }
  } else if(!strcmp(argument, "--irradianceResolution")) {
    if(!found) {
      irradianceMapResolution = 64;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      irradianceMapResolution = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--radianceResolution")) {
    if(!found) {
      radianceMapResolution = 256;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      radianceMapResolution = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--LUTResolution")) {
    if(!found) {
      LUTResolution = 64;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      LUTResolution = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--skyboxResolution")) {
    if(!found) {
      skyboxResolution = 512;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      skyboxResolution = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--irradianceSampleCount")) {
    if(!found) {
      irradianceMapSampleCount = 1024;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      irradianceMapSampleCount = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--radianceSampleCount")) {
    if(!found) {
      radianceMapSampleCount = 1024;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      radianceMapSampleCount = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--LUTSampleCount")) {
    if(!found) {
      LUTSampleCount = 1024;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      LUTSampleCount = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--skyboxSampleCount")) {
    if(!found) {
      skyboxSampleCount = 1024;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      skyboxSampleCount = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--radianceLODBias")) {
    if(!found) {
      radianceMapLODBias = 0.0f;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      radianceMapLODBias = std::stof(argv[i+1]);
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--radianceMipLevels")) {
    if(!found) {
      radianceMapMipCount = 4;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    try {
      radianceMapMipCount = static_cast<uint32_t>(std::stoul(argv[i+1]));
    } catch(std::invalid_argument) {
      return false;
    }
  } else if(!strcmp(argument, "--irradianceIntermediaryFormat")) {
    if(!found) {
      irradianceMapIntermediaryFormat = IBLLib::OutputFormat::R16G16B16A16_SFLOAT;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    if(!strcmp(argv[i+1], "R16G16B16A16_SFLOAT")) {
      irradianceMapIntermediaryFormat = IBLLib::OutputFormat::R16G16B16A16_SFLOAT;
    } else if(!strcmp(argv[i+1], "R8G8B8A8_UNORM")) {
      irradianceMapIntermediaryFormat = IBLLib::OutputFormat::R8G8B8A8_UNORM;
    } else if(!strcmp(argv[i+1], "R32G32B32A32_SFLOAT")) {
      irradianceMapIntermediaryFormat = IBLLib::OutputFormat::R32G32B32A32_SFLOAT;
    } else {
      return false;
    }
  } else if(!strcmp(argument, "--radianceIntermediaryFormat")) {
    if(!found) {
      radianceMapIntermediaryFormat = IBLLib::OutputFormat::R16G16B16A16_SFLOAT;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    if(!strcmp(argv[i+1], "R16G16B16A16_SFLOAT")) {
      radianceMapIntermediaryFormat = IBLLib::OutputFormat::R16G16B16A16_SFLOAT;
    } else if(!strcmp(argv[i+1], "R8G8B8A8_UNORM")) {
      radianceMapIntermediaryFormat = IBLLib::OutputFormat::R8G8B8A8_UNORM;
    } else if(!strcmp(argv[i+1], "R32G32B32A32_SFLOAT")) {
      radianceMapIntermediaryFormat = IBLLib::OutputFormat::R32G32B32A32_SFLOAT;
    } else {
      return false;
    }
  } else if(!strcmp(argument, "--skyboxFormat")) {
    if(!found) {
      skyboxFormat = IBLLib::OutputFormat::R32G32B32A32_SFLOAT;
      return true;
    }

    if(((i+1) == argc) || argv[i+1][0] == '-') {
      return false;
    }

    if(!strcmp(argv[i+1], "R32G32B32A32_SFLOAT")) {
      skyboxFormat = IBLLib::OutputFormat::R32G32B32A32_SFLOAT;
    } else {
      return false;
    }
  } else if(!strcmp(argument, "--compress")) {
    if(!found) {
      compress = false;
      return true;
    } else {
      compress = true;
      return true;
    }
  } else if(!strcmp(argument, "--skyboxTonemapper")) {
    if(!found) {
      skyboxTonemapper = NONE;
      return true;
    } else {
      if(!strcmp(argv[i+1], "NONE")) {
        skyboxTonemapper = NONE;
      } else if(!strcmp(argv[i+1], "ACES")) {
        skyboxTonemapper = ACESLUMINANCE;
      } else if(!strcmp(argv[i+1], "FILMIC")) {
        skyboxTonemapper = FILMIC;
      } else if(!strcmp(argv[i+1], "PBR")) {
        skyboxTonemapper = PBRNEUTRAL;
      }
    }
  }

  return true;
}

bool getArguments(int argc, char **argv) {

  if(!getArgument("--inputHDR", argc, argv, true)) {
    return false;
  }

  if(
    !findArgument("--outputIrradiance", nullptr, argc, argv) &&
    !findArgument("--outputRadiance", nullptr, argc, argv) &&
    !findArgument("--outputLUT", nullptr, argc, argv) &&
    !findArgument("--outputSkybox", nullptr, argc, argv)
    ) {
    return false;
  }

  if(!getArgument("--outputIrradiance", argc, argv, false)) {
    return false;
  }
  if(!getArgument("--outputRadiance", argc, argv, false)) {
    return false;
  }
  if(!getArgument("--outputLUT", argc, argv, false)) {
    return false;
  }
  if(!getArgument("--outputSkybox", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--irradianceDistribution", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--radianceDistribution", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--irradianceResolution", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--radianceResolution", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--LUTResolution", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--skyboxResolution", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--irradianceSampleCount", argc, argv, false)) {
    return false;
  }

  if (!getArgument("--skyboxSampleCount", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--radianceSampleCount", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--LUTSampleCount", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--skyboxSampleCount", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--radianceLODBias", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--radianceMipLevels", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--irradianceIntermediaryFormat", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--radianceIntermediaryFormat", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--skyboxFormat", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--skyboxTonemapper", argc, argv, false)) {
    return false;
  }

  if(!getArgument("--compress", argc, argv, false)) {
    return false;
  }

  return true;
}

