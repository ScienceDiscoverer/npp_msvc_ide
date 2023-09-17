// SLIB
#include "wtxtp"

wchar_t WtxtPrinter::defpad = ' ';

wtxt WtxtPrinter::PadDec(const wtxt &dn)
{
	wchar_t num[27]; // 18'446'744'073'709'551'616 MAX
	num[26] = 0;
	wchar_t *nmax = num + 26;
	wchar_t *n = nmax;
	
	ui32 d = 0;
	for(i64 i = (i64)~dn-1; i >= 0; --i)
	{
		*(--n) = *((const wchar_t *)dn + i);
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
	return cwstr({ n, (ui64)(nmax - n) });
}

wtxt WtxtPrinter::PadHex(const wtxt &hn)
{
	wchar_t num[25]; // 'FF'FF'FF'FF'FF'FF'FF'FF MAX
	num[24] = 0;
	wchar_t *nmax = num + 24;
	wchar_t *n = nmax;
	
	ui32 d = 0;
	for(i64 i = (i64)~hn-1; i >= 0; --i)
	{
		*(--n) = *((const wchar_t *)hn + i);
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
	return cwstr({ n, (ui64)(nmax - n) });
}