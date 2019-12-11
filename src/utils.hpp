#pragma once
#include <cstdint>
#include <netinet/in.h>

constexpr char hexSym[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::size_t byteArrayToCharString(uint8_t* inArray, std::size_t arrayLength, char* outString, bool split = false, char delemiter = ' ');

#define ntohll_(x) ( ( (uint64_t)(ntohl( (uint32_t)((x << 32) >> 32) )) << 32) | ntohl( ((uint32_t)(x >> 32)) ) )

uint64_t ntohll(uint64_t v);

#define htonll(x) ntohll(x)

