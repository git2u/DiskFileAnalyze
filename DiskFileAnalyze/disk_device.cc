
#include "disk_device.h"
#include <vector>
using std::vector;

DiskFile::DiskFile()
	: hDiskFile(INVALID_HANDLE_VALUE)
	, nBytesPerSector(0)
	, nDiskBytes(0)
{

}

DiskFile::~DiskFile()
{

}

bool DiskFile::open(const char* name)
{
	if (NULL == name)
		return false;
	HANDLE hDev = ::CreateFileA(name,
		FILE_READ_DATA | FILE_WRITE_DATA,
		(FILE_SHARE_WRITE | FILE_SHARE_READ),
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (INVALID_HANDLE_VALUE == hDev) {
		hDev = CreateFileA(name,
			FILE_READ_DATA,
			(FILE_SHARE_WRITE | FILE_SHARE_READ),
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
	}
	if (INVALID_HANDLE_VALUE == hDev)
		return false;
	this->hDiskFile = hDev;
	if (!this->readDiskGeometry()) {
		this->hDiskFile = INVALID_HANDLE_VALUE;
		return false;
	}
	return true;
}

bool DiskFile::readDiskGeometry()
{
	if (INVALID_HANDLE_VALUE == this->hDiskFile)
		return false;
	DWORD dwReadBytes = 0;
	DISK_GEOMETRY_EX geomEx = { 0 };
	BOOL bres = ::DeviceIoControl(this->hDiskFile,
		IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
		NULL,
		0,
		&geomEx,
		sizeof(geomEx),
		&dwReadBytes,
		NULL);
	if (bres) {
		this->nBytesPerSector = geomEx.Geometry.BytesPerSector;
		this->nDiskBytes = geomEx.DiskSize.QuadPart;
		if (0 != this->nBytesPerSector && 0 != this->nDiskBytes) {
			return true;
		}
	}

	DISK_GEOMETRY geom;
	GET_LENGTH_INFORMATION geom_disksize;
	bres = ::DeviceIoControl(this->hDiskFile,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL,
		0,
		&geom,
		sizeof(geom),
		&dwReadBytes,
		NULL);
	if (bres) {
		bres = ::DeviceIoControl(this->hDiskFile,
			IOCTL_DISK_GET_LENGTH_INFO,
			NULL,
			0,
			&geom_disksize,
			sizeof(geom_disksize),
			&dwReadBytes,
			NULL);
	}
	if (bres) {
		this->nBytesPerSector = geom.BytesPerSector;
		this->nDiskBytes = geom_disksize.Length.QuadPart;
		if (0 != this->nBytesPerSector && 0 != this->nDiskBytes) {
			return true;
		}
	}
	return false;
}

bool DiskFile::readFully(uint64 offset, uint32 size, void* buf)
{
	//¶¨Î»ÓÎ±ê
	if (this->hDiskFile == INVALID_HANDLE_VALUE)
		return false;
	LARGE_INTEGER liDistanceToMove;
	liDistanceToMove.QuadPart = offset;
	BOOL bResult = ::SetFilePointerEx(this->hDiskFile, liDistanceToMove, NULL, FILE_BEGIN);
	if (!bResult)
		return false;
	//Ñ­»·¶ÁÈ¡
	bool bres = true;
	uint32 bytes = size;
	DWORD dwReadBytes = 0;
	while (bytes > 0) {
		bResult = ::ReadFile(hDiskFile, (char*)(buf) + size - bytes, bytes, &dwReadBytes, NULL);
		if (!bResult) {
			bres = false;
			break;
		}
		bytes -= dwReadBytes;
	}
	return bres;
}

bool DiskFile::read(uint64 offset, uint32 size, void* buf)
{
	if (0 == size || !buf)
		return false;
	if (INVALID_HANDLE_VALUE == this->hDiskFile)
		return false;
	/* ±ØÐëÉÈÇø¶ÔÆë
	   ×ó±ßÎ´¶ÔÆë ÓÒ±ßÎ´¶ÔÆë 1
	   ×ó±ßÎ´¶ÔÆë ÓÒ±ß¶ÔÆë 2
	   ×ó±ß¶ÔÆë ÓÒ±ßÎ´¶ÔÆë 3
	   ×ó±ß¶ÔÆë ÓÒ±ß¶ÔÆë 4
	*/
	int state = 0;
	uint64 endoffset = offset + size;
	int leftoffset = SECTOR_OFFSET(offset, this->nBytesPerSector);
	int rightoffset = SECTOR_OFFSET(endoffset, this->nBytesPerSector);
	if (0 == leftoffset) {
		if (0 == rightoffset) state = 4; else state = 3;
	}
	else {
		if (0 == rightoffset) state = 2; else state = 1;
	}
	bool bres = false;
	switch (state) {
	case 1: {
		uint64 startOffsetAlignRight = SECTOR_RIGHT_ALIGN(offset, this->nBytesPerSector);
		uint64 endOffsetAlignLeft = SECTOR_LEFT_ALIGN(endoffset, this->nBytesPerSector);
		bres = this->readFully(startOffsetAlignRight, size - leftoffset - rightoffset, (char*)(buf) + leftoffset);
		if (!bres)
			break;
		vector<char> tempBuffer;
		tempBuffer.reserve(this->nBytesPerSector);
		uint64 startOffsetAlignLeft = SECTOR_LEFT_ALIGN(offset, this->nBytesPerSector);
		bres = this->readFully(startOffsetAlignLeft, this->nBytesPerSector, &tempBuffer[0]);
		if (!bres)
			break;
		memcpy_s(buf, leftoffset, &tempBuffer[0], leftoffset);
		uint64 endOffsetAlignRight = SECTOR_RIGHT_ALIGN(endoffset, this->nBytesPerSector);
		bres = this->readFully(endOffsetAlignRight, this->nBytesPerSector, &tempBuffer[0]);
		if (!bres)
			break;
		memcpy_s((char*)(buf)+size - rightoffset, rightoffset, &tempBuffer[0], rightoffset);
		break;
	}
	case 2: {
		uint64 startOffsetAlignRight = SECTOR_RIGHT_ALIGN(offset, this->nBytesPerSector);
		DWORD dwReadBytes = size - leftoffset;
		bres = this->readFully(startOffsetAlignRight, dwReadBytes, (char*)(buf) + leftoffset);
		if (!bres)
			break;
		vector<char> tempBuffer;
		tempBuffer.reserve(this->nBytesPerSector);
		uint64 startOffsetAlignLeft = SECTOR_LEFT_ALIGN(offset, this->nBytesPerSector);
		bres = this->readFully(startOffsetAlignLeft, this->nBytesPerSector, &tempBuffer[0]);
		if (!bres)
			bres;
		memcpy_s(buf, leftoffset, &tempBuffer[0], leftoffset);
		break;
	}
	case 3:{
		DWORD dwReadBytes = size - rightoffset;
		bres = this->readFully(offset, dwReadBytes, buf);
		if (!bres)
			break;
		vector<char> tempBuffer;
		tempBuffer.reserve(this->nBytesPerSector);
		uint64 u64EndOffsetAlignLeft = SECTOR_LEFT_ALIGN(endoffset, this->nBytesPerSector);
		bres = this->readFully(u64EndOffsetAlignLeft, this->nBytesPerSector, &tempBuffer[0]);
		if (!bres)
			break;
		memcpy_s((char*)(buf) + dwReadBytes, rightoffset, &tempBuffer[0], rightoffset);
		break;
	}
	case 4:
		bres = this->readFully(offset, size, buf);
		break;
	default:
		break;
	}
	return bres;
}

bool DiskFile::readSectors(uint64 nStartSector, uint32 nSectors, void* buf)
{
	uint64 offset = nStartSector * this->nBytesPerSector;
	uint32 size = nSectors * this->nBytesPerSector;
	return this->read(offset, size, buf);
}

void DiskFile::close()
{
	CloseHandle(hDiskFile);
	hDiskFile = INVALID_HANDLE_VALUE;
}