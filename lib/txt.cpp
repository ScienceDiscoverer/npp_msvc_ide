// SLIB
#include "txt"

HANDLE txt::heap;

txt & txt::operator+=(const txt &o)
{
	ui64 ns = s + o.s;
	if(ts <= ns)
	{
		char *tmp = Alloc(ns * TXT_CONCAT_MULT, ts);
		mcpy(tmp, t, s);
		TXT_FREE(t);
		t = tmp;
	}
	
	mcpy(t+s, o.t, o.s);
	s = ns;
	t[s] = 0;
	
	return *this;
}

txt & txt::operator+=(cstr cs)
{
	ui64 ns = s + cs.s;
	if(ts <= ns)
	{
		char *tmp = Alloc(ns * TXT_CONCAT_MULT, ts);
		mcpy(tmp, t, s);
		TXT_FREE(t);
		t = tmp;
	}
	
	mcpy(t+s, cs.t, cs.s);
	s = ns;
	t[s] = 0;
	
	return *this;
}

txt & txt::operator+=(char c) // Very effective for rapid chars appending
{
	ui64 ns = s + 1;
	if(ts <= ns)
	{
		char *tmp = Alloc(ns * TXT_CONCAT_MULT, ts);
		mcpy(tmp, t, s);
		TXT_FREE(t);
		t = tmp;
	}

	t[s] = c;
	s = ns;
	t[s] = 0;
	
	return *this;
}

void txt::ExceptBuf(DWORD c, ui64 pos) const
{
	char *tmp = (char *)HeapAlloc(heap, 0, ts);
	mcpy(tmp, t, s);	// Leaking memory here for debug purposes
	tmp[s] = 0;			// This will allow for temp texts to be passed
	
	static ULONG_PTR params[4];
	params[0] = (ULONG_PTR)tmp;
	params[1] = EXCEPT_TXT_UTF8;
	params[2] = s;
	params[3] = pos;
	RaiseException(c, 0, 4, params);
}

void txt::ExceptNum(DWORD c) const
{
	char *tmp = (char *)HeapAlloc(heap, 0, ts);
	mcpy(tmp, t, s);
	tmp[s] = 0;
	
	static ULONG_PTR params[3];
	params[0] = (ULONG_PTR)tmp;
	params[1] = EXCEPT_TXT_UTF8;
	params[2] = s;
	RaiseException(c, 0, 3, params);
}

void txt::ExceptPos(DWORD c, ui64 p0, ui64 p1) const
{
	static ULONG_PTR params[2];
	params[0] = p0;
	params[1] = p1;
	RaiseException(c, 0, 2, params);
}

ui64 txtf(const txt &t, ui64 pos, const txt &fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const char *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		if(memeq(tb, fnd.t, fnd.s))
		{
			return (ui64)(tb - t.t);
		}
		
		++tb;
	}
	
	return NFND;
}

ui64 txtf(const txt &t, ui64 pos, cstr fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const char *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		if(memeq(tb, fnd.t, fnd.s))
		{
			return (ui64)(tb - t.t);
		}
		
		++tb;
	}
	
	return NFND;
}

ui64 txtf(const txt &t, ui64 pos, char fnd)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const char *tb = t.t + pos - 1, *te = t.t + t.s;
	
	while(++tb != te)
	{	
		if(*tb == fnd)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtf(const txt &t, ui64 pos, char cb, char ce)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const char *tb = t.t + pos - 1, *te = t.t + t.s;
	
	while(++tb != te)
	{	
		if(*tb >= cb && *tb <= ce)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfe(const txt &t, ui64 pos, const txt &fnd)
{
	if(fnd.s > t.s)
	{
		return NFND;
	}
	
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const char *tb = t.t-1;
	const char *te = pos + fnd.s > t.s ? t.t + t.s - fnd.s : t.t + pos;
	
	while(te != tb)
	{
		if(memeq(te, fnd.t, fnd.s))
		{
			return (ui64)(te - t.t);
		}
		
		--te;
	}
	
	return NFND;
}

ui64 txtfe(const txt &t, ui64 pos, cstr fnd)
{
	if(fnd.s > t.s)
	{
		return NFND;
	}
	
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const char *tb = t.t-1;
	const char *te = pos + fnd.s > t.s ? t.t + t.s - fnd.s : t.t + pos;
	
	while(te != tb)
	{
		if(memeq(te, fnd.t, fnd.s))
		{
			return (ui64)(te - t.t);
		}
		
		--te;
	}
	
	return NFND;
}

ui64 txtfe(const txt &t, ui64 pos, char fnd)
{
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const char *tb = t.t-1, *te = t.t + pos + 1;
	
	while(--te != tb)
	{	
		if(*te == fnd)
		{
			return (ui64)(te - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfe(const txt &t, ui64 pos, char cb, char ce)
{
	if(pos >= t.s)
	{
		pos = t.s - 1;
	}
	
	const char *tb = t.t-1, *te = t.t + pos + 1;
	
	while(--te != tb)
	{	
		if(*te >= cb && *te <= ce)
		{
			return (ui64)(te - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfr(const txt &t, ui64 p0, ui64 p1, const txt &fnd)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(fnd.s > p1 - p0 + 1 || p0 > p1 - fnd.s + 1)
	{
		return NFND;
	}
	
	const char *tb = t.t + p0, *te = t.t + p1 - fnd.s + 2;
	
	while(tb != te)
	{
		if(memeq(tb, fnd.t, fnd.s))
		{
			return (ui64)(tb - t.t);
		}
		
		++tb;
	}
	
	return NFND;
}

ui64 txtfr(const txt &t, ui64 p0, ui64 p1, cstr fnd)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(fnd.s > p1 - p0 + 1 || p0 > p1 - fnd.s + 1)
	{
		return NFND;
	}
	
	const char *tb = t.t + p0, *te = t.t + p1 - fnd.s + 2;
	
	while(tb != te)
	{
		if(memeq(tb, fnd.t, fnd.s))
		{
			return (ui64)(tb - t.t);
		}
		
		++tb;
	}
	
	return NFND;
}

ui64 txtfr(const txt &t, ui64 p0, ui64 p1, char fnd)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(p0 > p1)
	{
		return NFND;
	}
	
	const char *tb = t.t + p0 - 1, *te = t.t + p1 + 1;
	
	while(++tb != te)
	{	
		if(*tb == fnd)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfr(const txt &t, ui64 p0, ui64 p1, char cb, char ce)
{
	if(p1 >= t.s)
	{
		p1 = t.s - 1;
	}
	
	if(p0 > p1)
	{
		return NFND;
	}
	
	const char *tb = t.t + p0 - 1, *te = t.t + p1 + 1;
	
	while(++tb != te)
	{	
		if(*tb >= cb && *tb <= ce)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

ui64 txtfa(const txt &t, ui64 pos, const txt &chars)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const char *fb = chars.t, *fe = chars.t + chars.s;
	const char *tb = t.t + pos, *te = t.t + t.s;
	
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

ui64 txtfa(const txt &t, ui64 pos, cstr chars)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const char *fb = chars.t, *fe = chars.t + chars.s;
	const char *tb = t.t + pos, *te = t.t + t.s;
	
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

ui64 txtfci(const txt &t, ui64 pos, const txt &fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const char *fb = fnd.t, *fl = fnd.t + fnd.s - 1; // Address of last character in pattern
	const char *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		const char *tc = tb;
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

ui64 txtfci(const txt &t, ui64 pos, cstr fnd)
{
	if(fnd.s > t.s || pos > t.s - fnd.s)
	{
		return NFND;
	}
	
	const char *fb = fnd.t, *fl = fnd.t + fnd.s - 1; // Address of last character in pattern
	const char *tb = t.t + pos, *te = t.t + t.s - fnd.s + 1;
	
	while(tb != te)
	{
		const char *tc = tb;
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

ui64 txtfci(const txt &t, ui64 pos, char fnd)
{
	if(pos >= t.s)
	{
		return NFND;
	}
	
	const char *tb = t.t + pos - 1, *te = t.t + t.s;
	
	while(++tb != te)
	{	
		if(c2low(*tb) == fnd)
		{
			return (ui64)(tb - t.t);
		}
	}
	
	return NFND;
}

txt & txti(txt &t, ui64 pos, const txt &ins)
{
	if(pos > t.s)
	{
		pos = t.s;
	}
	
	ui64 ns = t.s + ins.s;
	if(t.ts <= ns)
	{
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		mcpy(ipos, t.t, pos);
		mcpy(ipos += pos, ins.t, ins.s);
		mcpy(ipos + ins.s, t.t + pos, t.s - pos);
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	char *ipos = t.t + pos;
	memmove(ipos + ins.s, t.t + pos, t.s - pos);
	mcpy(ipos, ins.t, ins.s);
	t.s = ns;
	t[t.s] = 0;
	
	return t;
}

txt & txti(txt &t, ui64 pos, cstr ins)
{
	if(pos > t.s)
	{
		pos = t.s;
	}
	
	ui64 ns = t.s + ins.s;
	if(t.ts <= ns)
	{
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		mcpy(ipos, t.t, pos);
		mcpy(ipos += pos, ins.t, ins.s);
		mcpy(ipos + ins.s, t.t + pos, t.s - pos);
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	char *ipos = t.t + pos;
	memmove(ipos + ins.s, t.t + pos, t.s - pos);
	mcpy(ipos, ins.t, ins.s);
	t.s = ns;
	t[t.s] = 0;
	
	return t;
}

txt & txti(txt &t, ui64 pos, char ins)
{
	if(pos > t.s)
	{
		pos = t.s;
	}
	
	ui64 ns = t.s + 1;
	if(t.ts <= ns)
	{
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		mcpy(ipos, t.t, pos);
		*(ipos += pos) = ins;
		mcpy(ipos + 1, t.t + pos, t.s - pos);
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	char *ipos = t.t + pos;
	memmove(ipos + 1, t.t + pos, t.s - pos);
	*ipos = ins;
	t.s = ns;
	t[t.s] = 0;
	
	return t;
}

txt & txto(txt &t, ui64 pos, const txt &ovr)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	ui64 ns = pos + ovr.s;
	if(t.ts <= ns)
	{
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		mcpy(tmp, t.t, t.s);
		mcpy(tmp + pos, ovr.t, ovr.s);
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	mcpy(t.t + pos, ovr.t, ovr.s);
	
	if(ns > t.s)
	{
		t.s = ns;
		t.t[t.s] = 0;
	}
	
	return t;
}

txt & txto(txt &t, ui64 pos, cstr ovr)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return t;
	}
	
	ui64 ns = pos + ovr.s;
	if(t.ts <= ns)
	{
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		mcpy(tmp, t.t, t.s);
		mcpy(tmp + pos, ovr.t, ovr.s);
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}

	mcpy(t.t + pos, ovr.t, ovr.s);
	
	if(ns > t.s)
	{
		t.s = ns;
		t.t[t.s] = 0;
	}
	
	return t;
}

txt & txtr(txt &t, ui64 pos, ui64 n, const txt &rep)
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
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		
		mcpy(ipos, t.t, pos);
		mcpy(ipos += pos, rep.t, rep.s);
		mcpy(ipos + rep.s, t.t + pos + n, t.s - pos - n);
		
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	char *ipos = t.t + pos;
	memmove(ipos + rep.s, t.t + pos + n, t.s - pos - n);
	mcpy(ipos, rep.t, rep.s);
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtr(txt &t, ui64 pos, ui64 n, cstr rep)
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
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		
		mcpy(ipos, t.t, pos);
		mcpy(ipos += pos, rep.t, rep.s);
		mcpy(ipos + rep.s, t.t + pos + n, t.s - pos - n);
		
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	char *ipos = t.t + pos;
	memmove(ipos + rep.s, t.t + pos + n, t.s - pos - n);
	mcpy(ipos, rep.t, rep.s);
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtr(txt &t, ui64 pos, ui64 n, char rep)
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
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		mcpy(ipos, t.t, pos);
		*(ipos += pos) = rep;
		mcpy(ipos + 1, t.t + pos + n, t.s - pos - n);
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	char *ipos = t.t + pos;
	memmove(ipos + 1, t.t + pos + n, t.s - pos - n);
	*ipos = rep;
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtrp(txt &t, ui64 p0, ui64 p1, const txt &rep)
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
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		
		mcpy(ipos, t.t, p0);
		mcpy(ipos += p0, rep.t, rep.s);
		mcpy(ipos + rep.s, t.t + p0 + n, t.s - p0 - n);
		
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	char *ipos = t.t + p0;
	memmove(ipos + rep.s, t.t + p0 + n, t.s - p0 - n);
	mcpy(ipos, rep.t, rep.s);
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtrp(txt &t, ui64 p0, ui64 p1, cstr rep)
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
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		
		mcpy(ipos, t.t, p0);
		mcpy(ipos += p0, rep.t, rep.s);
		mcpy(ipos + rep.s, t.t + p0 + n, t.s - p0 - n);
		
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	char *ipos = t.t + p0;
	memmove(ipos + rep.s, t.t + p0 + n, t.s - p0 - n);
	mcpy(ipos, rep.t, rep.s);
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtrp(txt &t, ui64 p0, ui64 p1, char rep)
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
		char *tmp = txt::Alloc(ns * TXT_CONCAT_MULT, t.ts);
		char *ipos = tmp;
		mcpy(ipos, t.t, p0);
		*(ipos += p0) = rep;
		mcpy(ipos + 1, t.t + p0 + n, t.s - p0 - n);
		TXT_FREE(t.t);
		t.t = tmp;
		t.s = ns;
		t.t[t.s] = 0;
		
		return t;
	}
	
	char *ipos = t.t + p0;
	memmove(ipos + 1, t.t + p0 + n, t.s - p0 - n);
	*ipos = rep;
	
	t.s = ns;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtd(txt &t, ui64 pos, ui64 n)
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
	
	memmove(t.t + pos, t.t + pos + n, t.s - pos - n);
	t.s -= n;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtdp(txt &t, ui64 p0, ui64 p1)
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
	
	memmove(t.t + p0, t.t + p1 + 1, t.s - p1 - 1);
	t.s -= p1 - p0 + 1;
	t.t[t.s] = 0;
	
	return t;
}

txt txts(const txt &t, ui64 pos, ui64 n)
{
	if(pos >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, pos);
		return txt();
	}
	
	if(n > t.s || pos + n > t.s)
	{
		n = t.s - pos;
	}
	
	txt res = n;
	mcpy(res.t, t.t + pos, n);
	res.s = n;
	res.t[res.s] = 0;
	
	return res;
}

txt txtsp(const txt &t, ui64 p0, ui64 p1)
{
	if(p0 >= t.s)
	{
		t.ExceptBuf(EXCEPTION_TXT_BUFF_OVERRUN, p0);
		return txt();
	}
	
	if(p1 < p0)
	{
		t.ExceptPos(EXCEPTION_TXTSP_P1_LESS_P0, p0, p1);
		return txt();
	}
	
	if(p1 > t.s)
	{
		p1 = t.s-1;
	}
	
	ui64 n = p1 - p0 + 1;
	txt res = n;
	mcpy(res.t, t.t + p0, n);
	res.s = n;
	res.t[res.s] = 0;
	
	return res;
}

txt & txts(txt &t, const txt &src, ui64 pos, ui64 n)
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
	mcpy(t.t, src.t + pos, n);
	t.s = n;
	t.t[t.s] = 0;
	
	return t;
}

txt & txtsp(txt &t, const txt &src, ui64 p0, ui64 p1)
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
	mcpy(t.t, src.t + p0, n);
	t.s = n;
	t.t[t.s] = 0;
	
	return t;
}

txt i2t(ui64 i)
{
	char b[21]; // Max digits 18446744073709551615
	b[20] = 0;
	ui64 c = 20;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	return cstr({ b + c, 20 - c });
}

txt i2t(ui32 i)
{
	char b[11]; // Max digits 4294967295
	b[10] = 0;
	ui64 c = 10;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	return cstr({ b + c, 10 - c });
}

txt i2t(i64 i)
{
	char b[21]; // Max digits -9223372036854775808
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
	
	return cstr({ b + c, 20 - c });
}

txt i2t(i32 i)
{
	char b[12]; // Max digits -2147483648
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
	
	return cstr({ b + c, 11 - c });
}

txt b2t(ui8 b)
{
	char buf[4]; // Max digits 255
	buf[3] = 0;
	ui64 c = 3;
	
	do
	{
		buf[--c] = (char)(b % 10 | 0x30);
		b /= 10;
	}
	while(b > 0);
	
	return cstr({ buf + c, 3 - c });
}

txt b2t(i8 b)
{
	char buf[5]; // Max digits -128
	buf[4] = 0;
	ui64 c = 4;
	
	ui8 ia = b & 0x80 ? ui8(~b + 1) : (ui8)b;
	
	do
	{
		buf[--c] = (char)(ia % 10 | 0x30);
		ia /= 10;
	}
	while(ia > 0);
	
	if(b & 0x80)
	{
		buf[--c] = 0x2D;
	}
	
	return cstr({ buf + c, 4 - c });
}

txt i2h(ui64 i)
{
	char b[17]; // Max digits FFFFFFFFFFFFFFFF
	b[16] = 0; 
	ui64 c = 16;

	char *a = ((char *)&i) - 1;
	char *ae = a + 9;
	while(++a != ae)
	{
		b[--c] = char((*a & 0xF) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[--c] = char((*a >> 4) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	return cstr({ b, 16 });
}

txt i2h(ui32 i)
{
	char b[9]; // Max digits FFFFFFFF
	b[8] = 0;
	ui64 c = 8;

	char *a = ((char *)&i) - 1;
	char *ae = a + 5;
	while(++a != ae)
	{
		b[--c] = char((*a & 0xF) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[--c] = char((*a >> 4) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	return cstr({ b, 8 });
}

txt i2h(ui16 i)
{
	char b[5]; // Max digits FFFF
	b[4] = 0;
	
	char *a = ((char *)&i);
	
	b[3] = char((a[0] & 0xF) | 0x30);
	if(b[3] > 0x39)
	{
		b[3] += 0x7;
	}
	
	b[2] = char((a[0] >> 4) | 0x30);
	if(b[2] > 0x39)
	{
		b[2] += 0x7;
	}
	
	b[1] = char((a[1] & 0xF) | 0x30);
	if(b[1] > 0x39)
	{
		b[1] += 0x7;
	}
	
	b[0] = char((a[1] >> 4) | 0x30);
	if(b[0] > 0x39)
	{
		b[0] += 0x7;
	}
	
	return cstr({ b, 4 });
}

txt b2h(ui8 b)
{
	char buf[3]; // Max digits FF
	buf[2] = 0;
	
	buf[1] = char((b & 0xF) | 0x30);
	if(buf[1] > 0x39)
	{
		buf[1] += 0x7;
	}
	
	buf[0] = char((b >> 4) | 0x30);
	if(buf[0] > 0x39)
	{
		buf[0] += 0x7;
	}
	
	return cstr({ buf, 2 });
}

txt i2b(ui64 i)
{
	txt out = 64;
	ui64 mask = 0x8000000000000000;
	for(; mask != 1; mask /= 2)
	{
		out += i & mask ? '1' : '0';
	}
	
	return out += i & mask ? '1' : '0';
}

txt i2b(ui32 i)
{
	txt out = 64;
	ui64 mask = 0x80000000;
	for(; mask != 1; mask /= 2)
	{
		out += i & mask ? '1' : '0';
	}
	
	return out += i & mask ? '1' : '0';
}

txt i2b(ui16 i)
{
	txt out = 64;
	ui64 mask = 0x8000;
	for(; mask != 1; mask /= 2)
	{
		out += i & mask ? '1' : '0';
	}
	
	return out += i & mask ? '1' : '0';
}

txt b2b(ui8 b)
{
	txt out = 64;
	ui64 mask = 0x80;
	for(; mask != 1; mask /= 2)
	{
		out += b & mask ? '1' : '0';
	}
	
	return out += b & mask ? '1' : '0';
}

txt & i2t(ui64 i, txt &res)
{
	char b[21]; // Max digits 18446744073709551615
	b[20] = 0;
	ui64 c = 20;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	res = cstr({ b + c, 20 - c });
	return res;
}

txt & i2t(ui32 i, txt &res)
{
	char b[11]; // Max digits 4294967295
	b[10] = 0;
	ui64 c = 10;
	
	do
	{
		b[--c] = i % 10 | 0x30;
		i /= 10;
	}
	while(i > 0);
	
	res = cstr({ b + c, 10 - c });
	return res;
}

txt & i2t(i64 i, txt &res)
{
	char b[21]; // Max digits -9223372036854775808
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
	
	res = cstr({ b + c, 20 - c });
	return res;
}

txt & i2t(i32 i, txt &res)
{
	char b[12]; // Max digits -2147483648
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
	
	res = cstr({ b + c, 11 - c });
	return res;
}

txt & b2t(ui8 b, txt &res)
{
	char buf[4]; // Max digits 255
	buf[3] = 0;
	ui64 c = 3;
	
	do
	{
		buf[--c] = (char)(b % 10 | 0x30);
		b /= 10;
	}
	while(b > 0);
	
	res = cstr({ buf + c, 3 - c });
	return res;
}

txt & b2t(i8 b, txt &res)
{
	char buf[5]; // Max digits -128
	buf[4] = 0;
	ui64 c = 4;
	
	ui8 ia = b & 0x80 ? ui8(~b + 1) : (ui8)b;
	
	do
	{
		buf[--c] = (char)(ia % 10 | 0x30);
		ia /= 10;
	}
	while(ia > 0);
	
	if(b & 0x80)
	{
		buf[--c] = 0x2D;
	}
	
	res = cstr({ buf + c, 4 - c });
	return res;
}

txt & i2h(ui64 i, txt &res)
{
	char b[17]; // Max digits FFFFFFFFFFFFFFFF
	b[16] = 0; 
	ui64 c = 16;

	char *a = ((char *)&i) - 1;
	char *ae = a + 9;
	while(++a != ae)
	{
		b[--c] = char((*a & 0xF) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[--c] = char((*a >> 4) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	res = cstr({ b, 16 });
	return res;
}

txt & i2h(ui32 i, txt &res)
{
	char b[9]; // Max digits FFFFFFFF
	b[8] = 0;
	ui64 c = 8;

	char *a = ((char *)&i) - 1;
	char *ae = a + 5;
	while(++a != ae)
	{
		b[--c] = char((*a & 0xF) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
		
		b[--c] = char((*a >> 4) | 0x30);
		if(b[c] > 0x39)
		{
			b[c] += 0x7;
		}
	}
	
	res = cstr({ b, 8 });
	return res;
}

txt & i2h(ui16 i, txt &res)
{
	char b[5]; // Max digits FFFF
	b[4] = 0;
	
	char *a = ((char *)&i);
	
	b[3] = char((a[0] & 0xF) | 0x30);
	if(b[3] > 0x39)
	{
		b[3] += 0x7;
	}
	
	b[2] = char((a[0] >> 4) | 0x30);
	if(b[2] > 0x39)
	{
		b[2] += 0x7;
	}
	
	b[1] = char((a[1] & 0xF) | 0x30);
	if(b[1] > 0x39)
	{
		b[1] += 0x7;
	}
	
	b[0] = char((a[1] >> 4) | 0x30);
	if(b[0] > 0x39)
	{
		b[0] += 0x7;
	}
	
	res = cstr({ b, 4 });
	return res;
}

txt & b2h(ui8 b, txt &res)
{
	char buf[3]; // Max digits FF
	buf[2] = 0;
	
	buf[1] = char((b & 0xF) | 0x30);
	if(buf[1] > 0x39)
	{
		buf[1] += 0x7;
	}
	
	buf[0] = char((b >> 4) | 0x30);
	if(buf[0] > 0x39)
	{
		buf[0] += 0x7;
	}
	
	res = cstr({ buf, 2 });
	return res;
}

txt & i2b(ui64 i, txt &res)
{
	res = 64;
	ui64 mask = 0x8000000000000000;
	for(; mask != 1; mask /= 2)
	{
		res += i & mask ? '1' : '0';
	}
	
	return res += i & mask ? '1' : '0';
}

txt & i2b(ui32 i, txt &res)
{
	res = 64;
	ui64 mask = 0x80000000;
	for(; mask != 1; mask /= 2)
	{
		res += i & mask ? '1' : '0';
	}
	
	return res += i & mask ? '1' : '0';
}

txt & i2b(ui16 i, txt &res)
{
	res = 64;
	ui64 mask = 0x8000;
	for(; mask != 1; mask /= 2)
	{
		res += i & mask ? '1' : '0';
	}
	
	return res += i & mask ? '1' : '0';
}

txt & b2b(ui8 b, txt &res)
{
	res = 64;
	ui64 mask = 0x80;
	for(; mask != 1; mask /= 2)
	{
		res += b & mask ? '1' : '0';
	}
	
	return res += b & mask ? '1' : '0';
}

ui64 t2i(const txt &t) // Check out ql.cpp for most minimal and fastest implementation of this
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

ui64 h2i(const txt &t)
{
	if(t.s > 16)
	{
		t.ExceptNum(EXCEPTION_H2I_OVERFLOW);
		return 0x0;
	}
	
	ui64 n = 0;
	
	const char *tb = t.t-1;
	const char *te = t.t + t.s;
	ui64 i = 0;
	while((te -= 2) > tb)
	{
		ui8 ms = *te, ls = *(te+1);
		
		if(ms > 0x39)
		{
			ms = ui8((ms & 0xDF) - 0x7);
		}
		
		if(ls > 0x39)
		{
			ls = ui8((ls & 0xDF) - 0x7);
		}
		
		if((ms < 0x30 || ms > 0x3F) || (ls < 0x30 || ls > 0x3F))
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
		ui8 ls = *(te+1);
		
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