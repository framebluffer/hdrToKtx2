#ifndef __ARGUMENTS_H
#define __ARGUMENTS_H

#include "glTF-IBL-Sampler/GltfIblSampler.h"
#include "glTF-IBL-Sampler/ResultType.h"
#include "tonemap.h"
#include <stdint.h>
#include <vector>
#include <cstdlib>
#include <string>
#include <stdexcept>

#define STRINGBUF_SIZE 1024

// Arguments
extern char inputHDR[STRINGBUF_SIZE];

extern char outputIrradiance[STRINGBUF_SIZE];
extern char outputRadiance[STRINGBUF_SIZE];
extern char outputLUT[STRINGBUF_SIZE];
extern char outputSkybox[STRINGBUF_SIZE];

extern IBLLib::Distribution irradianceMapDistributionFunction;
extern IBLLib::Distribution radianceMapDistributionFunction;

extern uint32_t irradianceMapResolution;
extern uint32_t radianceMapResolution;
extern uint32_t LUTResolution;
extern uint32_t skyboxResolution;

extern uint32_t radianceMapMipCount;

extern uint32_t irradianceMapSampleCount;
extern uint32_t radianceMapSampleCount;
extern uint32_t LUTSampleCount;
extern uint32_t skyboxSampleCount;

extern IBLLib::OutputFormat irradianceMapIntermediaryFormat;
extern IBLLib::OutputFormat radianceMapIntermediaryFormat;
extern IBLLib::OutputFormat skyboxFormat;
extern TonemapType skyboxTonemapper;

extern float radianceMapLODBias;

extern bool outputUASTC;
extern bool outputETC1S;
extern bool compress;

bool getArguments(int argc, char **argv);
bool readFile(const char* path, std::vector<uint8_t>& outBuffer);

#endif // __ARGUMENTS_H