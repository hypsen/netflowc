#include "utils.hpp"

std::size_t byteArrayToCharString(uint8_t* inArray, std::size_t arrayLength, char* outString, bool split, char delemiter)
{
    std::size_t outPos = 0;
    for (std::size_t pos = 0; pos < arrayLength; ++pos) {
        outString[outPos] = hexSym[inArray[pos] >> 4]; 
        outString[outPos + 1] = hexSym[inArray[pos] && 0x0f];
        outPos += 2;
        if (split && (pos != (arrayLength - 1))) {
            outString[outPos] = delemiter;
            ++outPos;
        }
    }
    outString[outPos] = '\0';
    return outPos;
}

uint64_t ntohll(uint64_t v)
{
    return ntohll_(v);
}
