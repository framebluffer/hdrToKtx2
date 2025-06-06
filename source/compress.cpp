#include "compress.h"
#include "glTF-IBL-Sampler/GltfIblSampler.h"

bool initMipLevelInfos(const basist::ktx2_header &ktx2Header, const void *ktx2Data, size_t dataSize, uint32_t mipLevels, std::vector<basist::ktx2_level_index> &outMipLevelInfos) {
  if(mipLevels == 0) {
    mipLevels = 1; // NOTE(Markus): Here we just set it to 1 in order for the memcpy below to work.
  }
  const uint32_t totalMipLevelIndicesSize = mipLevels * sizeof(basist::ktx2_level_index);
  if((sizeof(ktx2Header) + totalMipLevelIndicesSize) > dataSize)
  {
    printf("Ktx2 file too small to contain mip level data\n");
    return false;
  }

  outMipLevelInfos.resize(mipLevels);

  memcpy(outMipLevelInfos.data(), static_cast<const uint8_t *>(ktx2Data) + sizeof(ktx2Header), totalMipLevelIndicesSize);

  return true;
}

bool createBasisCompressedHDRTexture(std::vector<uint8_t> &data, const char *outputPath, IBLLib::OutputFormat format) {
  printf("Setting up KTX2 texture params\n");

  basist::ktx2_header ktx2Header = {};
  memcpy(&ktx2Header, data.data(), sizeof(ktx2Header));

  uint8_t *texturePointer = data.data();

  uint32_t textureMipLevels = ktx2Header.m_level_count;
  uint32_t textureArrayLayers = ktx2Header.m_layer_count ? static_cast<uint32_t>(ktx2Header.m_layer_count) : 1;
  uint32_t textureCubeFaces = ktx2Header.m_face_count;
  uint32_t textureWidth = ktx2Header.m_pixel_width;
  uint32_t textureHeight = ktx2Header.m_pixel_height;
  uint32_t componentSize = ktx2Header.m_type_size;

  std::vector<basist::ktx2_level_index> mipLevelInfos;
  initMipLevelInfos(ktx2Header, texturePointer, data.size(), textureMipLevels, mipLevelInfos);

  size_t mipImageSize = mipLevelInfos[0].m_byte_length;

  // TODO(Markus): Option for non tonemapped uastc hdr, and tonemapped etc1s srgb

  basisu::basis_compressor_params compParams;
  compParams.m_hdr = true;
	compParams.m_uastc_hdr_4x4_options.set_quality_level(3); // TODO(Markus): For skybox etc1s tonemapped
	compParams.m_status_output = true;
	compParams.m_compute_stats = true;
	compParams.m_create_ktx2_file = true;
	compParams.m_write_output_basis_or_ktx2_files = true;
	compParams.m_perceptual = true;
  compParams.m_tex_type = basist::cBASISTexTypeCubemapArray;
  compParams.m_debug = true;
  compParams.m_mip_smallest_dimension = 16;

  size_t blockWidth = 1;
  size_t blockHeight = 1;
  size_t blockSize = 0;

  basisu::hdr_image_type imageType;

  switch(format) {
  case(IBLLib::OutputFormat::R16G16B16A16_SFLOAT): {
    imageType = basisu::hdr_image_type::cHITRGBAHalfFloat;
    blockSize = 8;
    break;
  }
  case(IBLLib::OutputFormat::R32G32B32A32_SFLOAT): {
    imageType = basisu::hdr_image_type::cHITRGBAFloat;
    blockSize = 16;
    break;
  }
  default: {
    printf("createBasisCompressedHDRTexture: format must be either R16G16B16A16_SFLOAT or R32G32B32A32_SFLOAT\n");
    return false;
    break;
  }
  }

  for(uint32_t layerIndex = 0; layerIndex < textureArrayLayers; layerIndex++) {
    for(uint32_t faceIndex = 0; faceIndex < textureCubeFaces; faceIndex++) {
      basisu::vector<basisu::imagef> mipImages;

      for(uint32_t mipIndex = 0; mipIndex < textureMipLevels; mipIndex++) {

        size_t width = textureWidth >> mipIndex;
        size_t height = textureHeight >> mipIndex;
        size_t blockColumns = (width + blockWidth - 1) / blockWidth;
        size_t blockRows = (height + blockHeight - 1) / blockHeight;

        mipImageSize = blockSize * blockColumns * blockRows;

        uint8_t *src = texturePointer + (mipLevelInfos[mipIndex].m_byte_offset + (layerIndex * textureCubeFaces * mipImageSize) + (faceIndex * mipImageSize));

        basisu::imagef image(width, height);
        load_image_hdr(src, mipImageSize, image, blockColumns, blockRows, imageType, false);

        if(mipIndex == 0) {
          compParams.m_source_images_hdr.push_back(image);
        } else {
          mipImages.push_back(image);
        }

      }

      if(mipImages.size()) {
        compParams.m_source_mipmap_images_hdr.push_back(mipImages);
      }
    }
  }

	compParams.m_out_filename = outputPath;

  const uint32_t nThreads = 1;
  basisu::job_pool jobPool(nThreads);
  compParams.m_pJob_pool = &jobPool;
  compParams.m_multithreading = false;

  basisu::enable_debug_printf(true);

  basisu::basis_compressor comp;
  if(!comp.init(compParams)) {
    printf("Failed to init basis compressor\n");
    return false;
  }

  basisu::basis_compressor::error_code ec = comp.process();
  if(ec != basisu::basis_compressor::cECSuccess) {
    printf("basisu failed to compress texture to UASTC\n");
    return false;
  }

  return true;
}
