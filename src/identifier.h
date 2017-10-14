#ifndef IDENTIFIER_H
#define IDENTIFIER_H
#include "utils/filemapping.h"
#include <string>

std::string identifyFile(FileMapping* imgDrive, uint sectorNum);
unsigned long  getIntNum(unsigned char* it, int n);
uint getStartExt3Sec(FileMapping* imgDrive, uint sectorNum);
uint getBlockGroup(FileMapping* imgDrive, uint secBG, uint& blocksCount, uint& blocksPerGroup);
void checkRangeSec(FileMapping* imgDrive, uint sectorNum);
void showPartitionTable(FileMapping* imgDrive);
void showSector(FileMapping* imgDrive, uint sectorNum, bool isHex = false);

#endif
