#define _CRT_SECURE_NO_WARNINGS

#include "file.h"

bool readFile(const char* path, std::vector<uint8_t>& outBuffer) {
	FILE* file = fopen(path, "rb");

	if (file == nullptr)
	{
		printf("Failed to open file %s\n", path);
		return false;
	}

	// obtain file size:
	fseek(file, 0, SEEK_END);
	auto size = ftell(file);
	rewind(file);

	outBuffer.resize(size);

	// read the file
	auto sizeRead = fread(outBuffer.data(), sizeof(char), size, file);
	fclose(file);

	return sizeRead > 0u;
}

bool writeFile(const char* pFilename, const void* pData, size_t len) {
  FILE* pFile = nullptr;
#ifdef _WIN32
  fopen_s(&pFile, pFilename, "wb");
#else
  pFile = fopen(pFilename, "wb");
#endif
  if (!pFile)
    return false;

  if (len)
  {
    if (fwrite(pData, 1, len, pFile) != len)
    {
      fclose(pFile);
      return false;
    }
  }

  return fclose(pFile) != EOF;
}
