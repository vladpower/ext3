#ifndef IDENTIFIER_H
#define IDENTIFIER_H
#include "utils/filemapping.h"
#include <string>
#include <vector>

std::string identifyFile(FileMapping* imgDrive, uint sectorNum);
unsigned long  getIntNum(unsigned char* it, int n);
uint getStartExt3Sec(FileMapping* imgDrive, uint sectorNum);
void checkRangeSec(FileMapping* imgDrive, uint sectorNum);
void showPartitionTable(FileMapping* imgDrive);
void showSector(FileMapping* imgDrive, uint sectorNum, bool isHex = false);
void depthSearch(uint iNumber,unsigned char* fileOffset, unsigned char* bootBlock,std::vector<uint>& inodeTable,uint inodesPerGroup, uint nFile, std::string pathFile);
void getBlockPointers(unsigned char* block, unsigned char* bootBlock, int nameNum, int depthIndirect);

#endif
