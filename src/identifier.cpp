#include "identifier.h"
#include <iostream>
#include <string>
#include <map>

using namespace std;

vector<string> ext3FileNames;
map<uint,uint> sectorFile;

#define SECTOR_SIZE 512

string identifyFile(FileMapping* imgDrive, uint sectorNum)
{
    checkRangeSec(imgDrive, sectorNum);
    string fRes;
    uint blocksCount;
    uint blocksPerGroup;
    return getBlockGroup(imgDrive,getStartExt3Sec(imgDrive, sectorNum),blocksCount,blocksPerGroup, sectorNum);


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
#define INODES_PER_GROUP_OFFSET 40
#define BLOCK_GROUP_OFFSET    2048
#define INODE_TABLE_OFFSET       8
#define INODE_COUNT_OFFSET      16
#define INODE_SIZE             128
#define INODE_IBLOCK_OFFSET     40
#define DIRECTORY_NAMELEN_OFFSET 6
#define DIRECTORY_NAME_OFFSET    8

string getBlockGroup(FileMapping* imgDrive, uint secBeg, uint& blocksCount, uint& blocksPerGroup, uint sectorNum)
{
    unsigned char* data = fileMappingGetPointer(imgDrive);
    unsigned char* bootBlock = data + secBeg * SECTOR_SIZE;
    unsigned char* superBlock = bootBlock + SUPER_BLOCK_OFFSET;
    blocksCount = getIntNum(superBlock + BLOCKS_COUNT_OFFSET, 4);
    blocksPerGroup = getIntNum(superBlock + BLOCKS_PER_GROUP_OFFSET, 4);
    uint inodesPerGroup = getIntNum(superBlock + INODES_PER_GROUP_OFFSET, 4);
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
    cout << "inodes per group "<<inodesPerGroup<<endl;
    for(int i = 0; i < blockGroup; i++) {
        inodeTable[i] = getIntNum(groupDesc + INODE_TABLE_OFFSET, 4);
        inodeCount[i] = getIntNum(groupDesc+INODE_COUNT_OFFSET,2);
        iCount+=inodeCount[i];
        cout << "Inode Table " << inodeTable[i]<< endl;
        cout << "Used dirs " << inodeCount[i]<< endl;
        groupDesc+=32;
    }
    std::vector<uint> dirPointer(iCount);
    cout <<"Root pointer"<<endl;
    unsigned char* rootInode = bootBlock + inodeTable[0] * BLOCK_SIZE + INODE_SIZE;
    uint rootDirPointer = getIntNum(rootInode + INODE_IBLOCK_OFFSET,4);
    cout<<rootDirPointer<<endl;
    unsigned char* dir = bootBlock + rootDirPointer * BLOCK_SIZE;
    unsigned long iNumber = getIntNum(dir,4);
    unsigned char* fileOffset = dir;
    cout << "Inode number "<< iNumber << endl;
    depthSearch(iNumber, fileOffset, bootBlock,inodeTable,inodesPerGroup,0,"/");

    return ext3FileNames[sectorFile.find( (sectorNum - secBeg) / (BLOCK_SIZE / SECTOR_SIZE) )->second];

    // for(int i = 1; i < iCount; i++) {
    //     unsigned char* dir = bootBlock + (dirPointer[i]) * SECTOR_SIZE;
    //     unsigned long iNumber = getIntNum(dir,4);
    //     cout << "Inode number "<< iNumber << endl;
    // }
}

void depthSearch(uint iNumber,unsigned char* fileOffset, unsigned char* bootBlock,vector<uint>& inodeTable,uint inodesPerGroup, uint nFile,string pathFile)
{
    if(iNumber==0)
        return;
    uint nameLen = getIntNum(fileOffset + DIRECTORY_NAMELEN_OFFSET,1);
    unsigned char* fileName = fileOffset + DIRECTORY_NAME_OFFSET;

    if(nFile>1) {
        string strFileName(reinterpret_cast<char*>(fileName), nameLen);
        ext3FileNames.push_back(pathFile + strFileName);
        int nameNum = ext3FileNames.size()-1;


        uint iGroup = iNumber / inodesPerGroup;
        uint iReminder = iNumber % inodesPerGroup - 1;
        unsigned char* inode = bootBlock + inodeTable[iGroup] * BLOCK_SIZE + iReminder * INODE_SIZE;
        uint fileMode = getIntNum(inode,4);
        uint fileType = fileMode / 10000;

        cout <<" - "<<iGroup<<" - "<< getIntNum(inode,4) <<" - "<< iNumber <<endl;



        for(int i = 0; i<12; i++ ) {
            uint blockPointer = getIntNum(inode + INODE_IBLOCK_OFFSET + i*4,4);
            if(blockPointer) {
                cout<<"Block Pointer "<<blockPointer<<endl;
                sectorFile.insert(pair<uint,uint>(blockPointer,nameNum) );
            }

        }
        showBlockPointers(inode + INODE_IBLOCK_OFFSET + 12*4, bootBlock, nameNum, 0);
        showBlockPointers(inode + INODE_IBLOCK_OFFSET + 13*4, bootBlock, nameNum, 1);
        showBlockPointers(inode + INODE_IBLOCK_OFFSET + 14*4, bootBlock, nameNum, 2);

        uint dirPointer = getIntNum(inode + INODE_IBLOCK_OFFSET,4);
        unsigned char* dir = bootBlock + dirPointer * BLOCK_SIZE;
        unsigned long iNum = getIntNum(dir,4);

        if(fileType==1)
            depthSearch(iNum, dir, bootBlock,inodeTable,inodesPerGroup,0,pathFile + strFileName+'/');
    }


    fileOffset = fileName + ((nameLen-1)/4+1)*4;
    iNumber = getIntNum(fileOffset,4);
    depthSearch(iNumber, fileOffset, bootBlock,inodeTable,inodesPerGroup,nFile+1,pathFile);

}

void showBlockPointers(unsigned char* block, unsigned char* bootBlock, int nameNum, int depthIndirect)
{
    uint indierectBlockPointer = getIntNum(block,4);
    cout<<"Indierect Block Pointer "<<indierectBlockPointer<<endl;
    if(indierectBlockPointer) {
        unsigned char* block = bootBlock + indierectBlockPointer * BLOCK_SIZE;
        uint blockPointer = getIntNum(block,4);
        while(blockPointer) {
            if(depthIndirect>0) {
                showBlockPointers(block, bootBlock, nameNum, depthIndirect - 1);
            } else {
                sectorFile.insert(pair<uint,uint>(blockPointer,nameNum) );
            }
            block+=4;
            blockPointer = getIntNum(block,4);
        }

    }
}
