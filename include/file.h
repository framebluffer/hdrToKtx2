#ifndef __FILE_H__
#define __FILE_H__

#include <vector>

bool readFile(const char* path, std::vector<uint8_t>& outBuffer);
bool writeFile(const char* pFilename, const void* pData, size_t len);

#endif // __FILE_H__
