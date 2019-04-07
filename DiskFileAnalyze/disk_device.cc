
#include "disk_device.h"

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
		return true;
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
		return true;
	}
	return false;
}

bool DiskFile::read(uint64 offset, uint32 size, void* buf)
{
	if (INVALID_HANDLE_VALUE == this->hDiskFile)
		return false;
	//±ØÐëÉÈÇø¶ÔÆë

	return true;
}

bool DiskFile::readSectors(uint64 nStartSector, uint32 nSectors, void* buf)
{
	return true;
}

void DiskFile::close()
{
	CloseHandle(hDiskFile);
	hDiskFile = INVALID_HANDLE_VALUE;
}