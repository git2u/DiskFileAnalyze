
#ifndef __DISK_CCTYPES_H
#define __DISK_CCTYPES_H

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef unsigned long long	uint64;
typedef unsigned long		ulong;

typedef char		int8;
typedef short		int16;
typedef int			int32;
typedef long long	int64;
typedef long		ilong;

#if (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
#	define cpu_to_le16(v) (v)
#	define cpu_to_le32(v) (v)
#	define cpu_to_le64(v) (v)
#	define le16_to_cpu(v) (v)
#	define le32_to_cpu(v) (v)
#	define le64_to_cpu(v) (v)
#else
#	define cpu_to_le16(v) _byteswap_ushort(v)
#	define cpu_to_le32(v) _byteswap_ulong(v)
#	define cpu_to_le64(v) _byteswap_uint64(v)

#	define le16_to_cpu(v) _byteswap_ushort(v)
#	define le32_to_cpu(v) _byteswap_ulong(v)
#	define le64_to_cpu(v) _byteswap_uint64(v)
#endif
#endif

/* Big-endian handling */
static inline uint16 hb_be_uint16(const uint16 v)
{
	const uint8 *V = (const uint8 *)&v;
	return (V[0] << 8) | V[1];
}

static inline uint16 hb_uint16_swap(const uint16 v)
{
	return (v >> 8) | (v << 8);
}

static inline uint32 hb_uint32_swap(const uint32 v)
{
	return (hb_uint16_swap(v) << 16) | hb_uint16_swap(v >> 16);
}

#endif