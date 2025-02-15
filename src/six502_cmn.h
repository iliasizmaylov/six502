#ifndef _SIX502_CMN_H_
#define _SIX502_CMN_H_

#include <cstdint>
#include <string>
#include <algorithm>
#include <cstring>

namespace SIX502 {

#define MEM_SIZES_xxx	\
	MEM_SIZE(_256B, 8)	\
	MEM_SIZE(_512B, 9)	\
	MEM_SIZE(_1KB, 10)	\
	MEM_SIZE(_2KB, 11)	\
	MEM_SIZE(_4KB, 12)	\
	MEM_SIZE(_8KB, 13)	\
	MEM_SIZE(_16KB, 14) \
	MEM_SIZE(_32KB, 15) \
	MEM_SIZE(_64KB, 16) \

#define MEM_SIZE(postfix, size) MEM_MAX##postfix = (1ULL << size),
enum MEM_SIZES_MAX {
MEM_SIZES_xxx
NR_MEM_MAX
};
#undef MEM_SIZE
#define MEM_SIZE(postfix, size) MEM_SZ##postfix = ((1ULL << size) - 1),
enum MEM_SIZES_SZ {
MEM_SIZES_xxx
NR_MEM_SZ
};
#undef MEM_SIZE

#undef MEM_SIZES_xxx

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;
typedef int64_t		s64;

typedef uint8_t		page_t;
typedef uint8_t		databus_t;
typedef uint16_t	addr_t;

typedef union addr_range {
	struct {
		addr_t hi;
		addr_t lo;
	};
	u32 mask;
} addr_range_t;

static inline u32
get_mask_addr_range(const addr_t& from_hi, const addr_t& to_lo)
{
	return ((from_hi << 8) | to_lo);
}

static inline void
fill_addr_range(addr_range_t& tgt, const addr_t& from_hi, const addr_t& to_lo)
{
	tgt.hi = from_hi;
	tgt.lo = to_lo;
}

static inline void fill_addr_range(addr_range_t& tgt, const u32& mask)
{
	tgt.mask = mask;
}

static inline bool
isin_addr_range(const addr_range_t& tgt, const addr_t& addr)
{
	if (tgt.hi <= tgt.lo)
		return (addr >= tgt.hi && addr <= tgt.lo);

	return (addr >= tgt.lo && addr <= tgt.hi);
}

static inline bool
addr_range_intersects(const addr_range_t& tgt, const addr_range_t& src)
{
	if ((tgt.hi > src.hi && tgt.hi > src.lo) &&
			(tgt.lo < src.lo && tgt.lo < src.hi))
		return false;

	return true;
}

static inline void
addr_range_squeeze(const addr_range_t& big, addr_range_t& small)
{
	if (!addr_range_intersects(big, small))
		return;

	small.hi = small.hi < big.hi ? big.hi : small.hi;
	small.lo = small.lo > big.lo ? big.lo : small.lo;
}

#define __SIX502_RET_SECTION_INFO		__RET_SEC_INFO,
#define __SIX502_RET_SECTION_WARNINGS	__RET_SEC_WARN,
#define __SIX502_RET_SECTION_ERRORS		__RET_SEC_ERR,
#define __SIX502_RET_SECTION_END		SIX502_NR_RET

#define RESULT_OK(x)	(x < __RET_SEC_INFO)
#define RESULT_INFO(x)	(x < __RET_SEC_WARN && x > __RES_SEC_INFO)
#define RESULT_WARN(x)	(x > __RET_SEC_WARN && x < __RET_SEC_ERR)
#define RESULT_ERR(x)	(x > __RET_SEC_ERR)

enum __SIX502_RET {
	SIX502_RET_SUCCESS,
	SIX502_RET_QUIT,

	SIX502_RET_DEVICE_BREAK,
	SIX502_RET_DEVICE_NOBREAK,

__SIX502_RET_SECTION_INFO
	SIX502_RET_NO_RW,

__SIX502_RET_SECTION_WARNINGS
	SIX502_RET_IRQ_DISABLED,
	SIX502_RET_STRAY_MEM,

__SIX502_RET_SECTION_ERRORS
	SIX502_RET_NO_DEV,
	SIX502_RET_BAD_INPUT,
	SIX502_RET_NO_BUS,
	SIX502_RET_HALT,

__SIX502_RET_SECTION_END
};
typedef enum __SIX502_RET result_t;

} /* namespace SIX502 */

#define HI8(x)			(((x) >> 8) & 0x00FF)
#define LO8(x)			((x) & 0x00FF)
#define HI16(x)			(((x) >> 16) & 0x0000FFFF)
#define LO16(x)			((x) & 0x0000FFFF)

#define MERGE16(hi, lo) \
	((((u16)(hi) << 8) & 0xFF00) | ((u16)(lo) & 0x00FF))

#define MERGE32(hi, lo) \
	((((u32)(hi) << 16) & 0xFFFF0000) | ((u32)(lo) & 0x0000FFFF))

#endif	/* _SIX502_CMN_H_ */
