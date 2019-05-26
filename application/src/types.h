#ifndef __TYPES_H__
#define __TYPES_H__

struct message_t
{
	u32_t RESERVED;
	u32_t length;
	u8_t data[32];
};

#endif // __TYPES_H__
