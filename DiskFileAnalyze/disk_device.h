
#ifndef __DISK_DEVICE_H
#define __DISK_DEVICE_H

#include "common/cctypes.h"
#include <windows.h>

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