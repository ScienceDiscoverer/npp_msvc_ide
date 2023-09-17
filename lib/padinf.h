#pragma once

#define PADL				0x0u
#define PADR				0x1u
#define PADC				0x2u

#define PAD_INFO_LENGTH		0x0
#define PAD_INFO_CHAR		0x4000000000000000
#define PAD_INFO_DIR		0x8000000000000000

#define PAD_INFO_PAD_NEXT	0x2000000000000000

struct PadInfo
{
	ui64 val;
};

struct SpaceNum
{
	ui64 n;
};

struct ConCurPosSet
{
	ui32 x;
	ui32 y;
};

struct ConCurPosGet
{
	ui64 *x;
	ui64 *y;
};

inline PadInfo SP(ui64 len)
{
	PadInfo pi; // 2 MS bits siginfy type of pad setting
	pi.val = len & 0x3FFFFFFFFFFFFFFF;
	return pi;
}

inline PadInfo SPC(wchar_t c)
{
	PadInfo pi;
	pi.val = (ui64)c & 0x3FFFFFFFFFFFFFFF | PAD_INFO_CHAR;
	return pi;
}

inline PadInfo SPD(ui64 dir)
{
	PadInfo pi;
	pi.val = dir & 0x3FFFFFFFFFFFFFFF | PAD_INFO_DIR;
	return pi;
}

inline PadInfo SPN(ui64 len)
{
	PadInfo pi;
	pi = SP(len); // 3rd MS bit signals single print pad mode
	pi.val = pi.val & 0xDFFFFFFFFFFFFFFF | PAD_INFO_PAD_NEXT;
	return pi;
}
	
inline PadInfo SPCN(wchar_t c)
{
	PadInfo pi;
	pi = SPC(c);
	pi.val = pi.val & 0xDFFFFFFFFFFFFFFF | PAD_INFO_PAD_NEXT;
	return pi;
}

inline PadInfo SPDN(ui64 dir)
{
	PadInfo pi;
	pi = SPD(dir);
	pi.val = pi.val & 0xDFFFFFFFFFFFFFFF | PAD_INFO_PAD_NEXT;
	return pi;
}

inline SpaceNum S(ui64 n)
{
	SpaceNum sn;
	sn.n = n;
	return sn;
}

inline ConCurPosSet SCP(ui64 x, ui64 y)
{
	ConCurPosSet ccp;
	ccp.x = (ui32)x;
	ccp.y = (ui32)y;
	return ccp;
}

inline ConCurPosGet GCP(ui64 &x, ui64 &y)
{
	ConCurPosGet ccp;
	ccp.x = &x;
	ccp.y = &y;
	return ccp;
}