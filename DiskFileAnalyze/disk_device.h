
#ifndef __DISK_DEVICE_H
#define __DISK_DEVICE_H

#include "common/cctypes.h"
#include <windows.h>

#define SECTOR_SIZE 0x200
#define SECTOR_MASK (~(SECTOR_SIZE - 1))

#define SECTOR_RIGHT_ALIGN(x, a) ((x + (a - 1)) & ~(a - 1))
#define SECTOR_LEFT_ALIGN(x, a) (x & ~(a - 1))
#define SECTOR_OFFSET(x, a) (x & (a - 1))

class DiskFile
{
public:
	DiskFile();
	virtual ~DiskFile();
	bool open(const char* name);
	bool read(uint64 offset, uint32 size, void* buf);
	bool readSectors(uint64 nStartSector, uint32 nSectors, void* buf);
	void close();
private:
	bool readDiskGeometry();
	bool readFully(uint64 offset, uint32 size, void* buf);
private:
	HANDLE hDiskFile;
	uint32 nBytesPerSector;
	uint64 nDiskBytes;
};

class DiskDevice : public DiskFile
{
public:
	DiskDevice();
	~DiskDevice();
protected:
private:
};

#endif