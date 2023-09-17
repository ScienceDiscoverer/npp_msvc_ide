// SLIB
#include "utf"


/*-----------------------------------------------------------------
	Unicode						UTF-8
	0x00000000 — 0x0000007F		0xxxxxxx
	0x00000080 — 0x000007FF		110xxxxx 10xxxxxx
	0x00000800 — 0x0000FFFF 	1110xxxx 10xxxxxx 10xxxxxx
	0x00010000 — 0x001FFFFF 	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	----------------------------------------------------------------
	
    High surrogates — U+D800 to U+DBFF (total of 1,024 code points)
    Low surrogates — U+DC00 to U+DFFF (total of 1,024 code points)
	----------------------------------------------------------------
	
	byte order mark (BOM) is a particular usage of the special Unicode character, U+FEFF BYTE ORDER MARK,
	whose appearance as a magic number at the start of a text stream can signal several things to a program reading the text:
	- The byte order, or endianness, of the text stream in the cases of 16-bit and 32-bit encodings;
	- The fact that the text stream's encoding is Unicode, to a high level of confidence;
	- Which Unicode character encoding is used.
	----------------------------------------------------------------*/


static wtxt tle = 1024; // Text Little Endian

txt wt2u8(const wtxt &t16)
{
	const wchar_t *tb;
	const wchar_t *te;
	if(t16[0] == 0xFFFE) // Check for BOM
	{
		tle = t16;
		wchar_t *tmp = (wchar_t *)tle - 1;
		wchar_t *tmpe = (wchar_t *)tle + ~tle;
		while(++tmp != tmpe) // Convert to Little Endian
		{
			wchar_t ctmp = wchar_t(*tmp << 8);
			ctmp |= *tmp >> 8;
			*tmp = ctmp;
		}
		
		tb = tle;
		te = tb + ~tle;
	}
	else if(t16[0] == 0xFEFF) // Already Little Endian, just skip
	{
		tb = t16;
		te = tb + ~t16;
	}
	else // Assume Little Endian by default
	{
		tb = (const wchar_t *)t16 - 1;
		te = (const wchar_t *)t16 + ~t16 - 1; // Exclude null char
	}
	
	txt res = ~t16;
	
	while(tb != te)
	{
		ui16 u16 = (ui16)*(++tb);

		if(u16 <= 0x7F)
		{
			res += (char)u16;
		}
		else if(u16 > 0x7FF)
		{
			if((u16 & 0xF000) == 0xD000) // Check for surrogates
			{
				ui16 hsur = u16;
				ui16 lsur = (ui16)*(++tb);
				
				ui32 u32 = ui32(hsur & 0x3FF);
				u32 <<= 10;
				u32 |= lsur & 0x3FF;
				u32 += 0x10000;
				
				char c0 = char((u32 & 0x3F) | 0x80); // Least significant             10xxxxxx 
				char c1 = char(((u32 >> 6) & 0x3F) | 0x80); // A bit more significant 10xxxxxx 
				char c2 = char(((u32 >> 12) & 0x3F) | 0x80); // Somewhat significant  10xxxxxx 
				char c3 = char(((u32 >> 18) & 0x7) | 0xF0); // Most significant       11110xxx
				
				res += c3; // Big Endian Network Byte Order
				res += c2;
				res += c1;
				res += c0;
				
				continue;
			}
			
			char c0 = char((u16 & 0x3F) | 0x80); // Least significant       10xxxxxx
			char c1 = char(((u16 >> 6) & 0x3F) | 0x80); // Middle one       10xxxxxx
			char c2 = char(((u16 >> 12) & 0xF) | 0xE0); // Most significant 1110xxxx
			
			res += c2;
			res += c1;
			res += c0;
		}
		else // u16 > 0x7F && u16 < 0x7FF
		{
			char c0 = char(((char)u16 & 0x3F) | 0x80); // Least significant 10xxxxxx
			char c1 = char(((u16 >> 6) & 0x1F) | 0xC0); // Most significant 110xxxxx
			
			res += c1;
			res += c0;
		}
	}

	return res;
}

wtxt t2u16(const txt &t8)
{
	const char *tb = (const char *)t8 - 1;
	const char *te = (const char *)t8 + ~t8 - 1; // Exclude null char
	
	if(t8[0] == 0xEF && t8[1] == 0xBB && t8[2] == 0xBF) // Check for BOM
	{
		tb += 3;
	}
	
	wtxt res = ~t8;
	
	while(tb != te)
	{
		char u8 = *(++tb);

		if(u8 <= 0x7F)
		{
			res += (wchar_t)u8;
		}
		else
		{
			char msn = char(u8 & 0xF0); // Most Significant Nibble
			
			switch(msn)
			{
			case 0xE0:
			{
				char c2 = u8; // Most significant       1110xxxx
				char c1 = *(++tb); // Middle one        10xxxxxx
				char c0 = *(++tb); // Least significant 10xxxxxx
				
				wchar_t wc = wchar_t(c2 & 0xF);
				wc <<= 6;
				wc |= c1 & 0x3F;
				wc <<= 6;
				wc |= c0 & 0x3F;
				
				res += wc;
			}	break;
			case 0xF0:
			{
				char c3 = u8; // Most significant            11110xxx
				char c2 = *(++tb); // Somewhat significant   10xxxxxx 
				char c1 = *(++tb); // A bit more significant 10xxxxxx 
				char c0 = *(++tb); // Least significant      10xxxxxx
				
				ui32 u32 = ui32(c3 & 0x7);
				u32 <<= 6;
				u32 |= c2 & 0x3F;
				u32 <<= 6;
				u32 |= c1 & 0x3F;
				u32 <<= 6;
				u32 |= c0 & 0x3F;
				
				u32 -= 0x10000;
				
				wchar_t wc1 = wchar_t(0xD800); // High Surrogate
				wchar_t wc0 = wchar_t(0xDC00); // Low Surrogate
				
				wc1 |= u32 >> 10;
				wc0 |= u32 & 0x3FF;
				
				res += wc1;
				res += wc0;
			}	break;
			default: // 110xxxxx
			{
				char c1 = u8; // Most significant       110xxxxx
				char c0 = *(++tb); // Least significant 10xxxxxx
				
				wchar_t wc = wchar_t(c1 & 0x1F);
				wc <<= 6;
				wc |= c0 & 0x3F;
				
				res += wc;
			}	break;
			}
		}
	}
	
	return res;
}

txt & wt2u8(txt &t8, const wtxt &t16)
{
	const wchar_t *tb;
	const wchar_t *te;
	if(t16[0] == 0xFFFE) // Check for BOM
	{
		tle = t16;
		wchar_t *tmp = (wchar_t *)tle - 1;
		wchar_t *tmpe = (wchar_t *)tle + ~tle;
		while(++tmp != tmpe) // Convert to Little Endian
		{
			wchar_t ctmp = wchar_t(*tmp << 8);
			ctmp |= *tmp >> 8;
			*tmp = ctmp;
		}
		
		tb = tle;
		te = tb + ~tle;
	}
	else if(t16[0] == 0xFEFF) // Already Little Endian, just skip
	{
		tb = t16;
		te = tb + ~t16;
	}
	else // Assume Little Endian by default
	{
		tb = (const wchar_t *)t16 - 1;
		te = (const wchar_t *)t16 + ~t16 - 1; // Exclude null char
	}
	
	t8 = ~t16;
	
	while(tb != te)
	{
		ui16 u16 = (ui16)*(++tb);

		if(u16 <= 0x7F)
		{
			t8 += (char)u16;
		}
		else if(u16 > 0x7FF)
		{
			if((u16 & 0xF000) == 0xD000) // Check for surrogates
			{
				ui16 hsur = u16;
				ui16 lsur = (ui16)*(++tb);
				
				ui32 u32 = ui32(hsur & 0x3FF);
				u32 <<= 10;
				u32 |= lsur & 0x3FF;
				u32 += 0x10000;
				
				char c0 = char((u32 & 0x3F) | 0x80); // Least significant             10xxxxxx 
				char c1 = char(((u32 >> 6) & 0x3F) | 0x80); // A bit more significant 10xxxxxx 
				char c2 = char(((u32 >> 12) & 0x3F) | 0x80); // Somewhat significant  10xxxxxx 
				char c3 = char(((u32 >> 18) & 0x7) | 0xF0); // Most significant       11110xxx
				
				t8 += c3; // Big Endian Network Byte Order
				t8 += c2;
				t8 += c1;
				t8 += c0;
				
				continue;
			}
			
			char c0 = char((u16 & 0x3F) | 0x80); // Least significant       10xxxxxx
			char c1 = char(((u16 >> 6) & 0x3F) | 0x80); // Middle one       10xxxxxx
			char c2 = char(((u16 >> 12) & 0xF) | 0xE0); // Most significant 1110xxxx
			
			t8 += c2;
			t8 += c1;
			t8 += c0;
		}
		else // u16 > 0x7F && u16 < 0x7FF
		{
			char c0 = char(((char)u16 & 0x3F) | 0x80); // Least significant 10xxxxxx
			char c1 = char(((u16 >> 6) & 0x1F) | 0xC0); // Most significant 110xxxxx
			
			t8 += c1;
			t8 += c0;
		}
	}

	return t8;
}

wtxt & t2u16(wtxt &t16, const txt &t8)
{
	const char *tb = (const char *)t8 - 1;
	const char *te = (const char *)t8 + ~t8 - 1; // Exclude null char
	
	if(t8[0] == 0xEF && t8[1] == 0xBB && t8[2] == 0xBF) // Check for BOM
	{
		tb += 3;
	}
	
	t16 = ~t8;
	
	while(tb != te)
	{
		char u8 = *(++tb);

		if(u8 <= 0x7F)
		{
			t16 += (wchar_t)u8;
		}
		else
		{
			char msn = char(u8 & 0xF0); // Most Significant Nibble
			
			switch(msn)
			{
			case 0xE0:
			{
				char c2 = u8; // Most significant       1110xxxx
				char c1 = *(++tb); // Middle one        10xxxxxx
				char c0 = *(++tb); // Least significant 10xxxxxx
				
				wchar_t wc = wchar_t(c2 & 0xF);
				wc <<= 6;
				wc |= c1 & 0x3F;
				wc <<= 6;
				wc |= c0 & 0x3F;
				
				t16 += wc;
			}	break;
			case 0xF0:
			{
				char c3 = u8; // Most significant            11110xxx
				char c2 = *(++tb); // Somewhat significant   10xxxxxx 
				char c1 = *(++tb); // A bit more significant 10xxxxxx 
				char c0 = *(++tb); // Least significant      10xxxxxx
				
				ui32 u32 = ui32(c3 & 0x7);
				u32 <<= 6;
				u32 |= c2 & 0x3F;
				u32 <<= 6;
				u32 |= c1 & 0x3F;
				u32 <<= 6;
				u32 |= c0 & 0x3F;
				
				u32 -= 0x10000;
				
				wchar_t wc1 = wchar_t(0xD800); // High Surrogate
				wchar_t wc0 = wchar_t(0xDC00); // Low Surrogate
				
				wc1 |= u32 >> 10;
				wc0 |= u32 & 0x3FF;
				
				t16 += wc1;
				t16 += wc0;
			}	break;
			default: // 110xxxxx
			{
				char c1 = u8; // Most significant       110xxxxx
				char c0 = *(++tb); // Least significant 10xxxxxx
				
				wchar_t wc = wchar_t(c1 & 0x1F);
				wc <<= 6;
				wc |= c0 & 0x3F;
				
				t16 += wc;
			}	break;
			}
		}
	}
	
	return t16;
}

ui64 u8ts(const txt &t8)
{
	if(~t8 == 0)
	{
		return 0;
	}
	
	const char *tb = (const char *)t8 - 1;
	const char *te = (const char *)t8 + ~t8 - 1; // Exclude null char
	
	if(~t8 > 2 && t8[0] == 0xEF && t8[1] == 0xBB && t8[2] == 0xBF) // Check for BOM
	{
		tb += 3;
	}
	
	ui64 ts = 0;
	
	while(tb != te)
	{
		char u8 = *++tb;
		++ts;

		char msn = char(u8 & 0xF0); // Most Significant Nibble
		
		if(u8 > 0x7F)
		{
			switch(msn)
			{
			case 0xE0:
				tb += 2;
				break;
			case 0xF0:
				tb += 3;
				break;
			default:
				++tb;
				break;
			}
		}
	}
	
	return ts;
}

ui64 u16ts(const wtxt &t16)
{
	if(~t16 == 0)
	{
		return 0;
	}
	
	const wchar_t *tb;
	const wchar_t *te;
	if(t16[0] == 0xFFFE) // Check for BOM
	{
		tle = t16;
		wchar_t *tmp = (wchar_t *)tle - 1;
		wchar_t *tmpe = (wchar_t *)tle + ~tle;
		while(++tmp != tmpe) // Convert to Little Endian
		{
			wchar_t ctmp = wchar_t(*tmp << 8);
			ctmp |= *tmp >> 8;
			*tmp = ctmp;
		}
		
		tb = tle;
		te = tb + ~tle;
	}
	else if(t16[0] == 0xFEFF) // Already Little Endian, just skip
	{
		tb = t16;
		te = tb + ~t16;
	}
	else // Assume Little Endian by default
	{
		tb = (const wchar_t *)t16 - 1;
		te = (const wchar_t *)t16 + ~t16 - 1; // Exclude null char
	}
	
	ui64 ts = 0;
	
	while(tb != te)
	{
		ui16 u16 = (ui16)*++tb;
		++ts;
		
		if((u16 & 0xF000) == 0xD000) // Check for surrogates
		{
			++tb;
		}
	}
	
	return ts;
}