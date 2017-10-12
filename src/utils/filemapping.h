#ifndef FILEMAPPING_H
#define FILEMAPPING_H

struct FileMapping;

FileMapping * fileMappingCreate(const char* fname);
unsigned char* fileMappingGetPointer(FileMapping * mapping);
unsigned int fileMappingGetSize(FileMapping * mapping);
void fileMappingClose(FileMapping * mapping);

#endif //FILEMAPPING_H
