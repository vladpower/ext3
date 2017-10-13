#include "identifier.h"
#include <iostream>
#define SECTOR_SIZE 512
using namespace std;


string identifyFile(FileMapping* imgDrive, int sectorNum)
{
    long secOffset = SECTOR_SIZE * sectorNum;
    if(fileMappingGetSize(imgDrive) < secOffset + SECTOR_SIZE) {
        cerr << "Sector out of range!" << endl;
        throw 1;
    }
    string fRes;
    fRes = "/File/Name";




    return fRes;
}

void showPartitionTable(FileMapping* imgDrive) {
    unsigned char* data = fileMappingGetPointer(imgDrive);
    unsigned char* it = data + 446;
    int bootIndicator[4];
    int startCHS[4];
    int partionType[4];
    int endCHS[4];
    int startSector[4];
    int partionSize[4];
    for(int i = 0;i<4;i++) {
        bootIndicator[i] = *(it++);
        startCHS[i] = (it[2]<<16) + (it[1]<<8) + it[0];
        it+=3;
        partionType[i] = *(it++);
        endCHS[i] = (it[2]<<16) + (it[1]<<8) + it[0];
        it+=3;
        startSector[i] = (it[3]<<24) + (it[2]<<16) + (it[1]<<8) + it[0];
        it+=4;
        partionSize[i] = (it[3]<<24) + (it[2]<<16) + (it[1]<<8) + it[0];
        it+=4;
    }
    for(int i = 0;i<4;i++) {
        cout << "Table Entry for Primary Partition #" << i+1 << endl;
        cout << "Boot Indicator " <<hex<< bootIndicator[i] << endl;
        cout << "Starting CHS value " << startCHS[i] << endl;
        cout << "Partition-type Descriptor " << partionType[i] << endl;
        cout << "Ending CHS values " << endCHS[i] << endl;
        cout << "Starting Sector " << startSector[i] << endl;
        cout << "Partition Size (in sectors) " << partionSize[i] << endl;
    }
}
