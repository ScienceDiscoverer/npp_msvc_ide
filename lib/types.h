#pragma once

typedef unsigned long long int ui64;
typedef signed long long int i64;
typedef unsigned int ui32;
typedef signed int i32;
typedef unsigned short int ui16;
typedef signed short int i16;
typedef unsigned char ui8;
typedef signed char i8;

typedef unsigned long int uli32;
typedef signed long int li32;

typedef unsigned long long int bool64;

#define NOTHROW __declspec(nothrow)
#define THREAD __pragma(warning( suppress : 4100 ))\
DWORD NOTHROW 
#define PARAMNOUSE __pragma(warning( suppress : 4100 ))

#define NFND		0xFFFFFFFFFFFFFFFF		// Value was not found

#define UI64_MAX	0xFFFFFFFFFFFFFFFF
#define UI32_MAX	0xFFFFFFFF
#define UI16_MAX	0xFFFF
#define UI8_MAX		0xFF

#define I64_MAX		0x7FFFFFFFFFFFFFFF
#define I32_MAX		0x7FFFFFFF
#define I16_MAX		0x7FFF
#define I8_MAX		0x7F

#define I64_MIN		((i64)(ui64)0x8000000000000000)
#define I32_MIN		((i32)(ui32)0x80000000)
#define I16_MIN		((i16)(ui16)0x8000)
#define I8_MIN		((i8)(ui8)0x80)

inline void empty() {}