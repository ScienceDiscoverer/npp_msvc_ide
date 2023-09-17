// SLIB
#include "txtp"

wchar_t TxtPrinter::defpad = ' ';

txt TxtPrinter::PadDec(const txt &dn)
{
	char num[27]; // 18'446'744'073'709'551'616 MAX
	num[26] = 0;
	char *nmax = num + 26;
	char *n = nmax;
	
	ui32 d = 0;
	for(i64 i = (i64)~dn-1; i >= 0; --i)
	{
		*(--n) = *((const char *)dn + i);
		if(++d % 3 == 0)
		{
			*(--n) = '\'';
		}
	}
	
	if(*n == '\'')
	{
		++n;
	}
	
	npad = false;
	return cstr({ n, (ui64)(nmax - n) });
}

txt TxtPrinter::PadHex(const txt &hn)
{
	char num[25]; // 'FF'FF'FF'FF'FF'FF'FF'FF MAX
	num[24] = 0;
	char *nmax = num + 24;
	char *n = nmax;
	
	ui32 d = 0;
	for(i64 i = (i64)~hn-1; i >= 0; --i)
	{
		*(--n) = *((const char *)hn + i);
		if(++d % 2 == 0)
		{
			*(--n) = ' ';
		}
	}
	
	if(*n == ' ')
	{
		++n;
	}
	
	npad = false;
	return cstr({ n, (ui64)(nmax - n) });
}