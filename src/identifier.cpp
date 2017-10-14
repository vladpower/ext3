#include "identifier.h"
#include <iostream>
#include <vector>
#define SECTOR_SIZE 512
using namespace std;


string identifyFile(FileMapping* imgDrive, uint sectorNum)
{
    checkRangeSec(imgDrive, sectorNum);
    string fRes;
    fRes = "/File/Name";
    uint blocksCount;
    uint blocksPerGroup;
    getBlockGroup(imgDrive,getStartExt3Sec(imgDrive, sectorNum),blocksCount,blocksPerGroup);


    return fRes;
}

inline unsigned long getIntNum(unsigned char* it, int n)
{
    //n = 1..4
    unsigned long num = *it;
    for(int i=1;i<n;i++) {
        num += it[i] << static_cast<unsigned long>( 8 * i);
    }

    return num;
}

#define PARTITION_TABLE_OFFSET 446

void showPartitionTable(FileMapping* imgDrive)
{
    //MBR
    unsigned char* data = fileMappingGetPointer(imgDrive);
    unsigned char* it = data + PARTITION_TABLE_OFFSET;
    int bootIndicator[4];
    unsigned long  startCHS[4];
    int partionType[4];
    unsigned long  endCHS[4];
    unsigned long startSector[4];
    unsigned long partionSize[4];
    for(int i = 0;i<4;i++) {
        bootIndicator[i] = *(it++); // 0
        startCHS[i] = getIntNum(it, 3); // 1 - 3
        it+=3;
        partionType[i] = *(it++); // 4
        endCHS[i] = getIntNum(it, 3); // 5 - 7
        it+=3;
        startSector[i] = getIntNum(it, 4); // 8 - 11
        it+=4;
        partionSize[i] = getIntNum(it, 4); // 12 - 15
        it+=4;
    }
    for(int i = 0;i<4;i++) {
        cout << "Table Entry for Primary Partition #" << i+1 << endl;
        cout << "Boot Indicator " << bootIndicator[i] << endl;
        cout << "Starting CHS value " << startCHS[i] << endl;
        cout << "Partition-type Descriptor " << partionType[i] << endl;
        cout << "Ending CHS values " << endCHS[i] << endl;
        cout << "Starting Sector " << startSector[i] << endl;
        cout << "Partition Size (in sectors) " << partionSize[i] << endl;
    }
}

void showSector(FileMapping* imgDrive, uint sectorNum, bool isHex)
{
    checkRangeSec(imgDrive, sectorNum);
    long secOffset = SECTOR_SIZE * sectorNum;
    unsigned char* data = fileMappingGetPointer(imgDrive);
    unsigned char* it = data + secOffset;
    if(isHex)
        cout<<hex;
    for(int i = 0;i<SECTOR_SIZE;i++) {
        cout << (int)*(it++)<<' ';
    }
    cout<<dec<<endl;

}

inline void checkRangeSec(FileMapping* imgDrive, uint sectorNum)
{
    if(fileMappingGetSize(imgDrive) < SECTOR_SIZE * (sectorNum + 1) ) {
        cerr << "Sector out of range!" << endl;
        throw 1;
    }
}

#define PARTION_TYPE_OFFSET  4
#define START_SECTOR_OFFSET  8
#define PARTION_SIZE_OFFSET 12
#define PARTION_ENTRY_SIZE  16
#define EXT3_PARTION_TYPE 0x83

uint getStartExt3Sec(FileMapping* imgDrive, uint sectorNum)
{
    //MBR
    checkRangeSec(imgDrive, sectorNum);
    int partionType[4];
    uint startSector[4];
    uint partionSize[4];
    unsigned char* data = fileMappingGetPointer(imgDrive);
    unsigned char* partEntry = data + PARTITION_TABLE_OFFSET;
    unsigned char* it;
    for(int i = 0; i<4; i++) {
        it = partEntry + PARTION_TYPE_OFFSET;
        partionType[i] = *it;
        it = partEntry + START_SECTOR_OFFSET;
        startSector[i] = getIntNum(it, 4);
        it = partEntry + PARTION_SIZE_OFFSET;
        partionSize[i] = getIntNum(it, 4);
        partEntry += PARTION_ENTRY_SIZE;
    }

    for(int i = 0; i<4; i++) {
        if(startSector[i] == 0)
            continue;
        uint endSector = startSector[i] + partionSize[i] - 1;
        if(startSector[i] <= sectorNum && sectorNum <= endSector) {
            // !!!need to check that the FS is not other linux FS!!!
            if(partionType[i] == EXT3_PARTION_TYPE) {
                return startSector[i];
            } else {
                cerr << "Partition " << startSector[i] << ' ' <<  endSector << endl;
                cerr << "Partion type is not ext3" << endl;
                throw 2;
            }
        }
    }
    cerr << "Sector does not belong to any partition" << endl;
    throw 3;

}

#define BLOCK_SIZE            1024 // !!s_log_block_size!!
#define SUPER_BLOCK_OFFSET    1024
#define BLOCKS_COUNT_OFFSET      4
#define BLOCKS_PER_GROUP_OFFSET 32
#define BLOCK_GROUP_OFFSET    2048
#define INODE_TABLE_OFFSET       8
#define INODE_COUNT_OFFSET       16
#define INODE_SIZE               128
#define INODE_IBLOCK_OFFSET      40

uint getBlockGroup(FileMapping* imgDrive, uint secBeg, uint& blocksCount, uint& blocksPerGroup)
{
    unsigned char* data = fileMappingGetPointer(imgDrive);
    unsigned char* bootBlock = data + secBeg * SECTOR_SIZE;
    unsigned char* superBlock = bootBlock + SUPER_BLOCK_OFFSET;
    blocksCount = getIntNum(superBlock + BLOCKS_COUNT_OFFSET, 4);
    blocksPerGroup = getIntNum(superBlock + BLOCKS_PER_GROUP_OFFSET, 4);
    uint blockGroup = (blocksCount + blocksPerGroup - 1) / blocksPerGroup; //round up
    unsigned char* groupDesc = bootBlock + BLOCK_GROUP_OFFSET;
    vector<uint> inodeTable(blockGroup);
    vector<uint> inodeCount(blockGroup);
    int iCount = 0;
    int reservedInods = getIntNum(groupDesc, 4);
    cout << "inodes "<<getIntNum(groupDesc, 4)<<endl;
    cout << "first data block "<<getIntNum(groupDesc, 20)<<endl;
    cout << "blocks num "<<blocksCount<<endl;
    cout << "blocks per group num "<<blocksPerGroup<<endl;
    cout << "block group "<<blockGroup<<endl;
    for(int i = 0; i < blockGroup; i++) {
        inodeTable[i] = getIntNum(groupDesc + INODE_TABLE_OFFSET, 4);
        inodeCount[i] = getIntNum(groupDesc+INODE_COUNT_OFFSET,2);
        iCount+=inodeCount[i];
        cout << "Inode Table " << inodeTable[i]<< endl;
        cout << "Used dirs " << inodeCount[i]<< endl;
        groupDesc+=32;
    }
    std::vector<uint> dirPointer(iCount);
    int k = 0;
    cout <<"Directory pointers"<<endl;
    for(int i = 0; i < blockGroup; i++) {
        unsigned char* inode = bootBlock + inodeTable[i] * BLOCK_SIZE;
        for(int j = 0; j < inodeCount[i]; j++) {
            dirPointer[k] = getIntNum(inode + INODE_IBLOCK_OFFSET,4);
            cout << dirPointer[k]<<endl;
            inode += INODE_SIZE;
            k++;
        }
    }

    for(int i = 1; i < iCount; i++) {
        unsigned char* dir = bootBlock + (dirPointer[i]) * SECTOR_SIZE;
        unsigned long iNumber = getIntNum(dir,4);
        cout << "Inode number "<< iNumber << endl;
    }





}
