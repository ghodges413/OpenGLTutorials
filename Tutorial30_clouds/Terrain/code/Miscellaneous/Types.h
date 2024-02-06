//
//  Types.h
//
#pragma once

typedef char				int8;
typedef unsigned char		uint8;
typedef short				int16;
typedef unsigned short		uint16;
typedef int					int32;
typedef unsigned int		uint32;
#if 1	// TDOO: the long/longint is compiler setting dependent 32vs64 and possibly other compilers
typedef long int			int64;
typedef unsigned long int	uint64;
#else
typedef long long int			int64;
typedef unsigned long long int	uint64;
#endif
typedef	float				float32;
typedef double				float64;