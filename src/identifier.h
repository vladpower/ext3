#ifndef IDENTIFIER_H
#define IDENTIFIER_H
#include "utils/filemapping.h"
#include <string>

std::string identifyFile(FileMapping* imgDrive, int sectorNum);
void showPartitionTable(FileMapping* imgDrive);

#endif
