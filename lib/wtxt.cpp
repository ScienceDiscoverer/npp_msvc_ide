// SLIB
#include "wtxt"

HANDLE wtxt::heap;

wtxt & wtxt::operator+=(const wtxt &o)
{
	ui64 ns = s + o.s;
	if(ts <= ns)
	{
		wchar_t *tmp = Alloc(ns * TXT_CONCAT_MULT, ts);
		mcpy(tmp, t, s * sizeof(wchar_t));
		WTXT_FREE(t);
		t = tmp;
	}
	
	mcpy(t+s, o.t, o.s * sizeof(wchar_t));
	s = ns;
	t[s] = 0;
	
	return *this;
}

wtxt & wtxt::operator+=(cwstr cs)
{
	ui64 ns = s + cs.s;
	if(ts <= ns)
	{
		wchar_t *tmp = Alloc(ns * TXT_CONCAT_MULT, ts);
		mcpy(tmp, t, s * sizeof(wchar_t));
		WTXT_FREE(t);
		t = tmp;
	}
	
	mcpy(t+s, cs.t, cs.s * sizeof(wchar_t));
	s = ns;
	t[s] = 0;
	
	return *this;
}

wtxt & wtxt::operator+=(wchar_t c) // Very effective for rapid chars appending
{
	ui64 ns = s + 1;
	if(ts <= ns)
	{
		wchar_t *tmp = Alloc(ns * TXT_CONCAT_MULT, ts);
		mcpy(tmp, t, s * sizeof(wchar_t));
		WTXT_FREE(t);
		t = tmp;
	}

	t[s] = c;
	s = ns;
	t[s] = 0;
	
	return *this;
}

void wtxt::ExceptBuf(DWORD c, ui64 pos) const
{
	wchar_t *tmp = (wchar_t *)HeapAlloc(heap, 0, ts * sizeof(wchar_t));
	mcpy(tmp, t, s * sizeof(wchar_t));	// Leaking memory here for debug purposes
	tmp[s] = 0;								// This will allow for temp texts to be passed
	
	static ULONG_PTR params[4];
	params[0] = (ULONG_PTR)tmp;
	params[1] = EXCEPT_TXT_UTF16;
	params[2] = s;
	params[3] = pos;
	RaiseException(c, 0, 4, params);
}

void wtxt::ExceptNum(DWORD c) const
{
	wchar_t *tmp = (wchar_t *)HeapAlloc(heap, 0, ts * sizeof(wchar_t));
	mcpy(tmp, t, s * sizeof(wchar_t));
	tmp[s] = 0;
	
	static ULONG_PTR params[3];
	params[0] = (ULONG_PTR)tmp;
	params[1] = EXCEPT_TXT_UTF16;
	params[2] = s;
	RaiseException(c, 0, 3, params);
}

void wtxt::ExceptPos(DWORD c, ui64 p0, ui64 p1) const
{
	static ULONG_PTR params[2];
	params[0] = p0;
	params[1] = p1;
	RaiseException(c, 0, 2, params);
}

ui64 txtf(const wtxt &t, ui64 pos, const wtxt &fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1; // Address of last character in pattern f
	const wchar_t *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		const wchar_t *tc = tb;
		while(*fb == *tc)
		{
			if(fb == fl)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, ++tb;
	}
	
	return NFND;
}

ui64 txtf(const wtxt &t, ui64 pos, cwstr fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1;
	const wchar_t *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		const wchar_t *tc = tb;
		while(*fb == *tc)
		{
			if(fb == fl)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, ++tb;
	}
	
	return NFND;
}

ui64 txtf(const wtxt &t, ui64 pos, wchar_t fnd)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const wchar_t *tb = t.t + pos - 1, *te = t.t + t.s;
	
	while(++tb != te)
	{	
		if(*tb == fnd)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtf(const wtxt &t, ui64 pos, wchar_t cb, wchar_t ce)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const wchar_t *tb = t.t + pos - 1, *te = t.t + t.s;
	
	while(++tb != te)
	{	
		if(*tb >= cb && *tb <= ce)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfe(const wtxt &t, ui64 pos, const wtxt &fnd)
{
	if(fnd.s > t.s)
	{
		return NFND;
	}
	
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1;
	const wchar_t *tb = t.t-1;
	const wchar_t *te = pos + fnd.s > t.s ? t.t + t.s - fnd.s : t.t + pos;
	
	while(te != tb)
	{
		const wchar_t *tc = te;
		while(*fb == *tc)
		{
			if(fb == fl)
			{
				return (ui64)(te - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, --te;
	}
	
	return NFND;
}

ui64 txtfe(const wtxt &t, ui64 pos, cwstr fnd)
{
	if(fnd.s > t.s)
	{
		return NFND;
	}
	
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1;
	const wchar_t *tb = t.t-1;
	const wchar_t *te = pos + fnd.s > t.s ? t.t + t.s - fnd.s : t.t + pos;
	
	while(te != tb)
	{
		const wchar_t *tc = te;
		while(*fb == *tc)
		{
			if(fb == fl)
			{
				return (ui64)(te - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, --te;
	}
	
	return NFND;
}

ui64 txtfe(const wtxt &t, ui64 pos, wchar_t fnd)
{
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const wchar_t *tb = t.t-1, *te = t.t + pos + 1;
	
	while(--te != tb)
	{	
		if(*te == fnd)
		{
			return (ui64)(te - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfe(const wtxt &t, ui64 pos, wchar_t cb, wchar_t ce)
{
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const wchar_t *tb = t.t-1, *te = t.t + pos + 1;
	
	while(--te != tb)
	{	
		if(*te >= cb && *te <= ce)
		{
			return (ui64)(te - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfr(const wtxt &t, ui64 p0, ui64 p1, const wtxt &fnd)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(fnd.s > p1 - p0 + 1 || p0 > p1 - fnd.s + 1)
	{
		return NFND;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1;
	const wchar_t *tb = t.t + p0, *te = t.t + p1 - fnd.s + 2;
	
	while(tb != te)
	{
		const wchar_t *tc = tb;
		while(*fb == *tc)
		{
			if(fb == fl)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, ++tb;
	}
	
	return NFND;
}

ui64 txtfr(const wtxt &t, ui64 p0, ui64 p1, cwstr fnd)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(fnd.s > p1 - p0 + 1 || p0 > p1 - fnd.s + 1)
	{
		return NFND;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1;
	const wchar_t *tb = t.t + p0, *te = t.t + p1 - fnd.s + 2;
	
	while(tb != te)
	{
		const wchar_t *tc = tb;
		while(*fb == *tc)
		{
			if(fb == fl)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, ++tb;
	}
	
	return NFND;
}

ui64 txtfr(const wtxt &t, ui64 p0, ui64 p1, wchar_t fnd)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(p0 > p1)
	{
		return NFND;
	}
	
	const wchar_t *tb = t.t + p0 - 1, *te = t.t + p1 + 1;
	
	while(++tb != te)
	{	
		if(*tb == fnd)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfr(const wtxt &t, ui64 p0, ui64 p1, wchar_t cb, wchar_t ce)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(p0 > p1)
	{
		return NFND;
	}
	
	const wchar_t *tb = t.t + p0 - 1, *te = t.t + p1 + 1;
	
	while(++tb != te)
	{	
		if(*tb >= cb && *tb <= ce)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfa(const wtxt &t, ui64 pos, const wtxt &chars)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const wchar_t *fb = chars.t, *fe = chars.t + chars.s;
	const wchar_t *tb = t.t + pos, *te = t.t + t.s;
	
	while(tb != te)
	{
		while(fb != fe)
		{
			if(*fb == *tb)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb;
		}
		
		fb = chars.t, ++tb;
	}
	
	return NFND;
}

ui64 txtfa(const wtxt &t, ui64 pos, cwstr chars)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const wchar_t *fb = chars.t, *fe = chars.t + chars.s;
	const wchar_t *tb = t.t + pos, *te = t.t + t.s;
	
	while(tb != te)
	{
		while(fb != fe)
		{
			if(*fb == *tb)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb;
		}
		
		fb = chars.t, ++tb;
	}
	
	return NFND;
}

ui64 txtfci(const wtxt &t, ui64 pos, const wtxt &fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1; // Address of last character in pattern
	const wchar_t *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		const wchar_t *tc = tb;
		while(*fb == c2low(*tc))
		{
			if(fb == fl)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, ++tb;
	}
	
	return NFND;
}

ui64 txtfci(const wtxt &t, ui64 pos, cwstr fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const wchar_t *fb = fnd.t, *fl = fnd.t + fnd.s - 1;
	const wchar_t *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		const wchar_t *tc = tb;
		while(*fb == c2low(*tc))
		{
			if(fb == fl)
			{
				return (ui64)(tb - t.t);
			}
			
			++fb, ++tc;
		}
		
		fb = fnd.t, ++tb;
	}
	
	return NFND;
}

ui64 txtfci(const wtxt &t, ui64 pos, wchar_t fnd)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const wchar_t *tb = t.t + pos - 1, *te = t.t + t.s;
	
	while(++tb != te)
	{	
		if(c2low(*tb) == fnd)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

wtxt & txti(wtxt &t, ui64 pos, const wtxt &ins)
{
	if(pos > t.s)
	{
		pos = t.s;
	}
	
	ui64 ns = t.s + ins.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		mcpy(ipos, t.t, pos * sizeof(wchar_t));
		mcpy(ipos += pos, ins.t, ins.s * sizeof(wchar_t));
		mcpy(ipos + ins.s, t.t + pos, (t.s - pos) * sizeof(wchar_t));
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	wchar_t *ipos = t.t + pos;
	memmove(ipos + ins.s, t.t + pos, (t.s - pos) * sizeof(wchar_t));
	mcpy(ipos, ins.t, ins.s * sizeof(wchar_t));
	t.s = ns;
	t[t.s] = 0;
	
	return t;
}

wtxt & txti(wtxt &t, ui64 pos, cwstr ins)
{
	if(pos > t.s)
	{
		pos = t.s;
	}
	
	ui64 ns = t.s + ins.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		mcpy(ipos, t.t, pos * sizeof(wchar_t));
		mcpy(ipos += pos, ins.t, ins.s * sizeof(wchar_t));
		mcpy(ipos + ins.s, t.t + pos, (t.s - pos) * sizeof(wchar_t));
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	wchar_t *ipos = t.t + pos;
	memmove(ipos + ins.s, t.t + pos, (t.s - pos) * sizeof(wchar_t));
	mcpy(ipos, ins.t, ins.s * sizeof(wchar_t));
	t.s = ns;
	t[t.s] = 0;
	
	return t;
}

wtxt & txti(wtxt &t, ui64 pos, wchar_t ins)
{
	if(pos > t.s)
	{
		pos = t.s;
	}
	
	ui64 ns = t.s + 1;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		mcpy(ipos, t.t, pos * sizeof(wchar_t));
		*(ipos += pos) = ins;
		mcpy(ipos + 1, t.t + pos, (t.s - pos) * sizeof(wchar_t));
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	wchar_t *ipos = t.t + pos;
	memmove(ipos + 1, t.t + pos, (t.s - pos) * sizeof(wchar_t));
	*ipos = ins;
	t.s = ns;
	t[t.s] = 0;
	
	return t;
}

wtxt & txto(wtxt &t, ui64 pos, const wtxt &ovr)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	ui64 ns = pos + ovr.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		mcpy(tmp, t.t, t.s * sizeof(wchar_t));
		mcpy(tmp + pos, ovr.t, ovr.s * sizeof(wchar_t));
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	mcpy(t.t + pos, ovr.t, ovr.s * sizeof(wchar_t));
	
	if(ns > t.s)
	{
		t.s = ns;
		t.t[t.s] = 0;
	}
	
	return t;
}

wtxt & txto(wtxt &t, ui64 pos, cwstr ovr)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	ui64 ns = pos + ovr.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		mcpy(tmp, t.t, t.s * sizeof(wchar_t));
		mcpy(tmp + pos, ovr.t, ovr.s * sizeof(wchar_t));
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	mcpy(t.t + pos, ovr.t, ovr.s * sizeof(wchar_t));
	
	if(ns > t.s)
	{
		t.s = ns;
		t.t[t.s] = 0;
	}
	
	return t;
}

wtxt & txtr(wtxt &t, ui64 pos, ui64 n, const wtxt &rep)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	if(n >= t.s || pos + n >= t.s)
	{
		n = t.s - pos;
	}
	
	ui64 ns = t.s - n + rep.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		
		mcpy(ipos, t.t, pos * sizeof(wchar_t));
		mcpy(ipos += pos, rep.t, rep.s * sizeof(wchar_t));
		mcpy(ipos + rep.s, t.t + pos + n, (t.s - pos - n) * sizeof(wchar_t));
		
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	wchar_t *ipos = t.t + pos;
	memmove(ipos + rep.s, t.t + pos + n, (t.s - pos - n) * sizeof(wchar_t));
	mcpy(ipos, rep.t, rep.s * sizeof(wchar_t));
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtr(wtxt &t, ui64 pos, ui64 n, cwstr rep)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	if(n >= t.s || pos + n >= t.s)
	{
		n = t.s - pos;
	}
	
	ui64 ns = t.s - n + rep.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		
		mcpy(ipos, t.t, pos * sizeof(wchar_t));
		mcpy(ipos += pos, rep.t, rep.s * sizeof(wchar_t));
		mcpy(ipos + rep.s, t.t + pos + n, (t.s - pos - n) * sizeof(wchar_t));
		
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	wchar_t *ipos = t.t + pos;
	memmove(ipos + rep.s, t.t + pos + n, (t.s - pos - n) * sizeof(wchar_t));
	mcpy(ipos, rep.t, rep.s * sizeof(wchar_t));
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtr(wtxt &t, ui64 pos, ui64 n, wchar_t rep)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	if(n >= t.s || pos + n >= t.s)
	{
		n = t.s - pos;
	}
	
	ui64 ns = t.s - n + 1; // n = 0 means that [rep] will be INSERTED at pos
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		mcpy(ipos, t.t, pos * sizeof(wchar_t));
		*(ipos += pos) = rep;
		mcpy(ipos + 1, t.t + pos + n, (t.s - pos - n) * sizeof(wchar_t));
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	wchar_t *ipos = t.t + pos;
	memmove(ipos + 1, t.t + pos + n, (t.s - pos - n) * sizeof(wchar_t));
	*ipos = rep;
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtrp(wtxt &t, ui64 p0, ui64 p1, const wtxt &rep)
{
	if(p0 >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, p0);
		return t;
	}
	
	if(p1 < p0)
	{
		t.ExceptPos(EXCEPTION_TXTRP_P1_LESS_P0, p0, p1);
		return t;
	}
	
	if(p1 >= t.s)
	{
		p1 = t.s-1;
	}
	
	ui64 n = p1 - p0 + 1;
	ui64 ns = t.s - n + rep.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		
		mcpy(ipos, t.t, p0 * sizeof(wchar_t));
		mcpy(ipos += p0, rep.t, rep.s * sizeof(wchar_t));
		mcpy(ipos + rep.s, t.t + p0 + n, (t.s - p0 - n) * sizeof(wchar_t));
		
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	wchar_t *ipos = t.t + p0;
	memmove(ipos + rep.s, t.t + p0 + n, (t.s - p0 - n) * sizeof(wchar_t));
	mcpy(ipos, rep.t, rep.s * sizeof(wchar_t));
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtrp(wtxt &t, ui64 p0, ui64 p1, cwstr rep)
{
	if(p0 >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, p0);
		return t;
	}
	
	if(p1 < p0)
	{
		t.ExceptPos(EXCEPTION_TXTRP_P1_LESS_P0, p0, p1);
		return t;
	}
	
	if(p1 >= t.s)
	{
		p1 = t.s-1;
	}
	
	ui64 n = p1 - p0 + 1;
	ui64 ns = t.s - n + rep.s;
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		
		mcpy(ipos, t.t, p0 * sizeof(wchar_t));
		mcpy(ipos += p0, rep.t, rep.s * sizeof(wchar_t));
		mcpy(ipos + rep.s, t.t + p0 + n, (t.s - p0 - n) * sizeof(wchar_t));
		
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	wchar_t *ipos = t.t + p0;
	memmove(ipos + rep.s, t.t + p0 + n, (t.s - p0 - n) * sizeof(wchar_t));
	mcpy(ipos, rep.t, rep.s * sizeof(wchar_t));
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtrp(wtxt &t, ui64 p0, ui64 p1, char rep)
{
	if(p0 >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, p0);
		return t;
	}
	
	if(p1 < p0)
	{
		t.ExceptPos(EXCEPTION_TXTRP_P1_LESS_P0, p0, p1);
		return t;
	}
	
	if(p1 >= t.s)
	{
		p1 = t.s-1;
	}
	
	ui64 n = p1 - p0 + 1;
	ui64 ns = t.s - n + 1; // n = 0 means that [rep] will be INSERTED at pos
	if(t.ts <= ns)
	{
		wchar_t *tmp = wtxt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		wchar_t *ipos = tmp;
		mcpy(ipos, t.t, p0 * sizeof(wchar_t));
		*(ipos += p0) = rep;
		mcpy(ipos + 1, t.t + p0 + n, (t.s - p0 - n) * sizeof(wchar_t));
		WTXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	wchar_t *ipos = t.t + p0;
	memmove(ipos + 1, t.t + p0 + n, (t.s - p0 - n) * sizeof(wchar_t));
	*ipos = rep;
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtd(wtxt &t, ui64 pos, ui64 n)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	if(n >= t.s || pos + n >= t.s)
	{
		t.s = pos;
		t.t[pos] = 0;
		return t;
	}
	
	memmove(t.t + pos, t.t + pos + n, (t.s - pos - n) * sizeof(wchar_t));
	t.s -= n;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtdp(wtxt &t, ui64 p0, ui64 p1)
{
	if(p0 >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, p0);
		return t;
	}
	
	if(p1 < p0)
	{
		t.ExceptPos(EXCEPTION_TXTRP_P1_LESS_P0, p0, p1);
		return t;
	}
	
	if(p1 >= t.s)
	{
		t.s = p0;
		t.t[p0] = 0;
		return t;
	}
	
	memmove(t.t + p0, t.t + p1 + 1, (t.s - p1 - 1) * sizeof(wchar_t));
	t.s -= p1 - p0 + 1;
	t.t[t.s] = 0;
	
	return t;
}

wtxt txts(const wtxt &t, ui64 pos, ui64 n)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return wtxt();
	}
	
	if(n > t.s || pos + n > t.s)
	{
		n = t.s - pos;
	}
	
	wtxt res = n;
	mcpy(res.t, t.t + pos, n * sizeof(wchar_t));
	res.s = n;
	res.t[res.s] = 0;
	
	return res;
}

wtxt txtsp(const wtxt &t, ui64 p0, ui64 p1)
{
	if(p0 >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, p0);
		return wtxt();
	}
	
	if(p1 < p0)
	{
		t.ExceptPos(EXCEPTION_TXTSP_P1_LESS_P0, p0, p1);
		return wtxt();
	}
	
	if(p1 > t.s)
	{
		p1 = t.s-1;
	}
	
	ui64 n = p1 - p0 + 1;
	wtxt res = n;
	mcpy(res.t, t.t + p0, n * sizeof(wchar_t));
	res.s = n;
	res.t[res.s] = 0;
	
	return res;
}

wtxt & txts(wtxt &t, const wtxt &src, ui64 pos, ui64 n)
{
	if(pos >= src.s)
	{
		src.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	if(n > src.s || pos + n > src.s)
	{
		n = src.s - pos;
	}
	
	t = n;
	mcpy(t.t, src.t + pos, n * sizeof(wchar_t));
	t.s = n;
	t.t[t.s] = 0;
	
	return t;
}

wtxt & txtsp(wtxt &t, const wtxt &src, ui64 p0, ui64 p1)
{
	if(p0 >= src.s)
	{
		src.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, p0);
		return t;
	}
	
	if(p1 < p0)
	{
		src.ExceptPos(EXCEPTION_TXTSP_P1_LESS_P0, p0, p1);
		return t;
	}
	
	if(p1 > src.s)
	{
		p1 = src.s-1;
	}
	
	ui64 n = p1 - p0 + 1;
	t = n;
	mcpy(t.t, src.t + p0, n * sizeof(wchar_t));
	t.s = n;
	t.t[t.s] = 0;
	
	return t;
}

wtxt i2wt(ui64 i)
{
	wchar_t b[21]; // Max digits 18446744073709551615
	b[20] = 0;
	ui64 c = 20;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	return cwstr({ b + c, 20 - c });
}

wtxt i2wt(ui32 i)
{
	wchar_t b[11]; // Max digits 4294967295
	b[10] = 0;
	ui64 c = 10;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	return cwstr({ b + c, 10 - c });
}

wtxt i2wt(i64 i)
{
	wchar_t b[21]; // Max digits -9223372036854775808
	b[20] = 0;
	ui64 c = 20;
	
	ui64 ia = i & 0x8000000000000000 ? ui64(~i + 1) : (ui64)i;
	
	do
	{
		b[--c] = ia % 10 | 0x30;
		ia /= 10;
	}
	while(ia > 0);
	
	if(i & 0x8000000000000000)
	{
		b[--c] = 0x2D;
	}
	
	return cwstr({ b + c, 20 - c });
}

wtxt i2wt(i32 i)
{
	wchar_t b[12]; // Max digits -2147483648
	b[11] = 0;
	ui64 c = 11;
	
	ui32 ia = i & 0x80000000 ? ui32(~i + 1) : (ui32)i;
	
	do
	{
		b[--c] = ia % 10 | 0x30;
		ia /= 10;
	}
	while(ia > 0);
	
	if(i & 0x80000000)
	{
		b[--c] = 0x2D;
	}
	
	return cwstr({ b + c, 11 - c });
}

wtxt b2wt(ui8 b)
{
	wchar_t buf[4]; // Max digits 255
	buf[3] = 0;
	ui64 c = 3;
	
	do
	{
		buf[--c] = (wchar_t)(b % 10 | 0x30);
		b /= 10;
	}
	while(b > 0);
	
	return cwstr({ buf + c, 3 - c });
}

wtxt b2wt(i8 b)
{
	wchar_t buf[5]; // Max digits -128
	buf[4] = 0;
	ui64 c = 4;
	
	ui8 ia = b & 0x80 ? ui8(~b + 1) : (ui8)b;
	
	do
	{
		buf[--c] = (wchar_t)(ia % 10 | 0x30);
		ia /= 10;
	}
	while(ia > 0);
	
	if(b & 0x80)
	{
		buf[--c] = 0x2D;
	}
	
	return cwstr({ buf + c, 4 - c });
}


wtxt i2wh(ui64 i)
{
	char b[34]; // Max digits FFFFFFFFFFFFFFFF
	b[32] = 0;
	b[33] = 0;
	ui64 c = 32;

	char *a = ((char *)&i) - 1;
	char *ae = a + 9;
	while(++a != ae)
	{
		b[c-=2] = char((*a & 0xF) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[c-=2] = char((*a >> 4) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	return cwstr({ (wchar_t *)b, 16 });
}

wtxt i2wh(ui32 i)
{
	char b[18]; // Max digits FFFFFFFF
	b[16] = 0;
	b[17] = 0;
	ui64 c = 16;

	char *a = ((char *)&i) - 1;
	char *ae = a + 5;
	while(++a != ae)
	{
		b[c-=2] = char((*a & 0xF) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[c-=2] = char((*a >> 4) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	return cwstr({ (wchar_t *)b, 8 });
}

wtxt i2wh(ui16 i)
{
	char b[10]; // Max digits FFFF
	b[8] = 0;
	b[9] = 0;
	
	char *a = ((char *)&i);
	
	b[6] = char((a[0] & 0xF) | 0x30);
	b[7] = 0;
	if(b[6] > 0x39)
	{
		b[6] += 0x7;
	}
	
	b[4] = char((a[0] >> 4) | 0x30);
	b[5] = 0;
	if(b[4] > 0x39)
	{
		b[4] += 0x7;
	}
	
	b[2] = char((a[1] & 0xF) | 0x30);
	b[3] = 0;
	if(b[2] > 0x39)
	{
		b[2] += 0x7;
	}
	
	b[0] = char((a[1] >> 4) | 0x30);
	b[1] = 0;
	if(b[0] > 0x39)
	{
		b[0] += 0x7;
	}
	
	return cwstr({ (wchar_t *)b, 4 });
}

wtxt b2wh(ui8 b)
{
	char buf[6]; // Max digits FF
	buf[4] = 0;
	buf[5] = 0;
	
	buf[2] = char((b & 0xF) | 0x30);
	buf[3] = 0;
	if(buf[2] > 0x39)
	{
		buf[2] += 0x7;
	}
	
	buf[0] = char((b >> 4) | 0x30);
	buf[1] = 0;
	if(buf[0] > 0x39)
	{
		buf[0] += 0x7;
	}
	
	return cwstr({ (wchar_t *)buf, 2 });
}

wtxt i2wb(ui64 i)
{
	wtxt out = 64;
	ui64 mask = 0x8000000000000000;
	for(; mask != 1; mask /= 2)
	{
		out += i & mask ? '1' : '0';
	}
	
	return out += i & mask ? '1' : '0';
}

wtxt i2wb(ui32 i)
{
	wtxt out = 64;
	ui64 mask = 0x80000000;
	for(; mask != 1; mask /= 2)
	{
		out += i & mask ? '1' : '0';
	}
	
	return out += i & mask ? '1' : '0';
}

wtxt i2wb(ui16 i)
{
	wtxt out = 64;
	ui64 mask = 0x8000;
	for(; mask != 1; mask /= 2)
	{
		out += i & mask ? '1' : '0';
	}
	
	return out += i & mask ? '1' : '0';
}

wtxt b2wb(ui8 b)
{
	wtxt out = 64;
	ui64 mask = 0x80;
	for(; mask != 1; mask /= 2)
	{
		out += b & mask ? '1' : '0';
	}
	
	return out += b & mask ? '1' : '0';
}

wtxt & i2t(ui64 i, wtxt &res)
{
	wchar_t b[21]; // Max digits 18446744073709551615
	b[20] = 0;
	ui64 c = 20;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	res = cwstr({ b + c, 20 - c });
	return res;
}

wtxt & i2t(ui32 i, wtxt &res)
{
	wchar_t b[11]; // Max digits 4294967295
	b[10] = 0;
	ui64 c = 10;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	res = cwstr({ b + c, 10 - c });
	return res;
}

wtxt & i2t(i64 i, wtxt &res)
{
	wchar_t b[21]; // Max digits -9223372036854775808
	b[20] = 0;
	ui64 c = 20;
	
	ui64 ia = i & 0x8000000000000000 ? ui64(~i + 1) : (ui64)i;
	
	do
	{
		b[--c] = ia % 10 | 0x30;
		ia /= 10;
	}
	while(ia > 0);
	
	if(i & 0x8000000000000000)
	{
		b[--c] = 0x2D;
	}
	
	res = cwstr({ b + c, 20 - c });
	return res;
}

wtxt & i2t(i32 i, wtxt &res)
{
	wchar_t b[12]; // Max digits -2147483648
	b[11] = 0;
	ui64 c = 11;
	
	ui32 ia = i & 0x80000000 ? ui32(~i + 1) : (ui32)i;
	
	do
	{
		b[--c] = ia % 10 | 0x30;
		ia /= 10;
	}
	while(ia > 0);
	
	if(i & 0x80000000)
	{
		b[--c] = 0x2D;
	}
	
	res = cwstr({ b + c, 11 - c });
	return res;
}

wtxt & b2t(ui8 b, wtxt &res)
{
	wchar_t buf[4]; // Max digits 255
	buf[3] = 0;
	ui64 c = 3;
	
	do
	{
		buf[--c] = (wchar_t)(b % 10 | 0x30);
		b /= 10;
	}
	while(b > 0);
	
	res = cwstr({ buf + c, 3 - c });
	return res;
}

wtxt & b2t(i8 b, wtxt &res)
{
	wchar_t buf[5]; // Max digits -128
	buf[4] = 0;
	ui64 c = 4;
	
	ui8 ia = b & 0x80 ? ui8(~b + 1) : (ui8)b;
	
	do
	{
		buf[--c] = (wchar_t)(ia % 10 | 0x30);
		ia /= 10;
	}
	while(ia > 0);
	
	if(b & 0x80)
	{
		buf[--c] = 0x2D;
	}
	
	res = cwstr({ buf + c, 4 - c });
	return res;
}


wtxt & i2h(ui64 i, wtxt &res)
{
	char b[34]; // Max digits FFFFFFFFFFFFFFFF
	b[32] = 0;
	b[33] = 0;
	ui64 c = 32;

	char *a = ((char *)&i) - 1;
	char *ae = a + 9;
	while(++a != ae)
	{
		b[c-=2] = char((*a & 0xF) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[c-=2] = char((*a >> 4) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	res = cwstr({ (wchar_t *)b, 16 });
	return res;
}

wtxt & i2h(ui32 i, wtxt &res)
{
	char b[18]; // Max digits FFFFFFFF
	b[16] = 0;
	b[17] = 0;
	ui64 c = 16;

	char *a = ((char *)&i) - 1;
	char *ae = a + 5;
	while(++a != ae)
	{
		b[c-=2] = char((*a & 0xF) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[c-=2] = char((*a >> 4) | 0x30);
		b[c+1] = 0;
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	res = cwstr({ (wchar_t *)b, 8 });
	return res;
}

wtxt & i2h(ui16 i, wtxt &res)
{
	char b[10]; // Max digits FFFF
	b[8] = 0;
	b[9] = 0;
	
	char *a = ((char *)&i);
	
	b[6] = char((a[0] & 0xF) | 0x30);
	b[7] = 0;
	if(b[6] > 0x39)
	{
		b[6] += 0x7;
	}
	
	b[4] = char((a[0] >> 4) | 0x30);
	b[5] = 0;
	if(b[4] > 0x39)
	{
		b[4] += 0x7;
	}
	
	b[2] = char((a[1] & 0xF) | 0x30);
	b[3] = 0;
	if(b[2] > 0x39)
	{
		b[2] += 0x7;
	}
	
	b[0] = char((a[1] >> 4) | 0x30);
	b[1] = 0;
	if(b[0] > 0x39)
	{
		b[0] += 0x7;
	}
	
	res = cwstr({ (wchar_t *)b, 4 });
	return res;
}

wtxt & b2h(ui8 b, wtxt &res)
{
	char buf[6]; // Max digits FF
	buf[4] = 0;
	buf[5] = 0;
	
	buf[2] = char((b & 0xF) | 0x30);
	buf[3] = 0;
	if(buf[2] > 0x39)
	{
		buf[2] += 0x7;
	}
	
	buf[0] = char((b >> 4) | 0x30);
	buf[1] = 0;
	if(buf[0] > 0x39)
	{
		buf[0] += 0x7;
	}
	
	res = cwstr({ (wchar_t *)buf, 2 });
	return res;
}

wtxt & i2b(ui64 i, wtxt &res)
{
	res = 64;
	ui64 mask = 0x8000000000000000;
	for(; mask != 1; mask /= 2)
	{
		res += i & mask ? '1' : '0';
	}
	
	return res += i & mask ? '1' : '0';
}

wtxt & i2b(ui32 i, wtxt &res)
{
	res = 64;
	ui64 mask = 0x80000000;
	for(; mask != 1; mask /= 2)
	{
		res += i & mask ? '1' : '0';
	}
	
	return res += i & mask ? '1' : '0';
}

wtxt & i2b(ui16 i, wtxt &res)
{
	res = 64;
	ui64 mask = 0x8000;
	for(; mask != 1; mask /= 2)
	{
		res += i & mask ? '1' : '0';
	}
	
	return res += i & mask ? '1' : '0';
}

wtxt & b2b(ui8 b, wtxt &res)
{
	res = 64;
	ui64 mask = 0x80;
	for(; mask != 1; mask /= 2)
	{
		res += b & mask ? '1' : '0';
	}
	
	return res += b & mask ? '1' : '0';
}

ui64 t2i(const wtxt &t) // Check out ql.cpp for most minimal and fastest implementation of this
{
	ui64 negative = 0;
	
	if(t.t[0] == 0x2D)
	{
		negative = 1;
	}
	else if(t.t[0] < 0x30 || t.t[0] > 0x39)
	{
		t.ExceptNum(EXCEPTION_T2I_NON_NUMBER);
		return 0;
	}
	
	ui64 n = 0;
	ui64 pren = 0;
	for(ui64 i = negative; i < t.s; ++i)
	{
		if(t.t[i] < 0x30 || t.t[i] > 0x39)
		{
			t.ExceptNum(EXCEPTION_T2I_NON_NUMBER);
			return 0;
		}
		
		n = n * 10 + (t.t[i] & 0xF);
		
		if(n < pren)
		{
			t.ExceptNum(EXCEPTION_T2I_OVERFLOW);
			return 0;
		}
		
		pren = n;
	}
	
	if(negative)
	{
		n = ~n + 1; // 2-s Complement
	}
	
	if(n < pren)
	{
		t.ExceptNum(EXCEPTION_T2I_OVERFLOW);
		return 0;
	}
	
	return n;
}

ui64 h2i(const wtxt &t)
{
	if(t.s > 16)
	{
		t.ExceptNum(EXCEPTION_H2I_OVERFLOW);
		return 0x0;
	}
	
	ui64 n = 0;
	
	const wchar_t *tb = t.t-1;
	const wchar_t *te = t.t + t.s;
	ui64 i = 0;
	while((te -= 2) > tb)
	{
		ui8 ms = (ui8)(*te), ls = (ui8)(*(te+1));
		
		if(ms > 0x39)
		{
			ms = ui8((ms & 0xDF) - 0x7);
		}
		
		if(ls > 0x39)
		{
			ls = ui8((ls & 0xDF) - 0x7);
		}
		
		if((ms < 0x30 || ms > 0x3F) || (ls < 0x30 || ls > 0x3F) || (*te & 0xFF00) || (*te+1 & 0xFF00))
		{
			t.ExceptNum(EXCEPTION_H2I_NON_NUMBER);
			return 0x0;
		}
		
		ui8 b = ui8((ms << 4) | (ls & 0xF));
		
		n |= (ui64)b << i * 8;
		++i;
	}
	
	if(te == tb) // Uneven number of nibbles
	{
		ui8 ls = (ui8)(*(te+1));
		
		if(ls > 0x39)
		{
			ls = ui8((ls & 0xDF) - 0x7);
		}
		
		if(ls < 0x30 || ls > 0x3F)
		{
			t.ExceptNum(EXCEPTION_H2I_NON_NUMBER);
			return 0x0;
		}
		
		n |= (ui64)(ls & 0xF) << i * 8;
	}
	
	return n;
}