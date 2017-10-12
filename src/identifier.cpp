#include "identifier.h"
#include <iostream>
#define SECTOR_SIZE 512
using namespace std;


string identifyFile(FileMapping* imgDrive, int sectorNum)
{
    long secOffset = SECTOR_SIZE * sectorNum;
    if(fileMappingGetSize(imgDrive) < secOffset) {
        cerr << "Sector out of range!" << endl;
        throw 1;
    }
    string fRes;
    fRes = "/File/Name";


    unsigned char* data = fileMappingGetPointer(imgDrive);
    unsigned char* it = data + secOffset;
    for(int i = 0;i<SECTOR_SIZE;i++) {
        cout << (int)*(it++)<<' ';
    }

    return fRes;
}
