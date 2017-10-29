#include <iostream>
#include "utils/filemapping.h"
#include "identifier.h"
using namespace std;



int main(int argc, char* argv[])
{
    //input name of drive image file and offset from the beginning of the disk in sectors
    if(argc<3) {
        cerr<<"Too few arguments!"<<endl;
        return -1;
    }
    char* fname = argv[1];
    int sectorNum = atoi(argv[2]);
    FileMapping* imgDrive = fileMappingCreate(fname);
    if(imgDrive == nullptr)
        return -1;
    try {
        cout << "File: "<<identifyFile(imgDrive, sectorNum) << endl;
        showSector(imgDrive,sectorNum,true);
        cout << "Starting sector: " << getStartExt3Sec(imgDrive,sectorNum)<<endl;
    } catch(int e) {

    }



    fileMappingClose(imgDrive);
    return 0;
}
