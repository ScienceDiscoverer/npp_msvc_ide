// SLIB
#include <txtp>
#include <wtxtp>

#include "time"

static ui64 sw_freq;

inline ui64 swInit()
{
	// Freqency may be in 100/10/1 ns, depending on device
	// Get exect ns resolution by dividing 1 000 000 000 / freq
	QueryPerformanceFrequency((LARGE_INTEGER *)&sw_freq);
	return 1'000'000'000/(ui64)sw_freq;

	// Note: int64 can store up to 58 494 years, 88 days, 5 h, 36 m, 10 s, 955 ms, 161 us 600 ns
	// at 10 000 000 ticks per second freqency (100 ns)
}

ui64 sw_ns_per_tick = swInit();

txt ticks2txt(ui64 timing)
{
	ui64 s = timing/sw_freq;
	ui64 ms = timing/(1000000/sw_ns_per_tick) - s * 1000;
	ui64 us = timing/(1000/sw_ns_per_tick) - s * 1000000 - ms * 1000;
	ui64 ns = timing * sw_ns_per_tick - s * 1000000000 - ms * 1000000 - us * 1000;
	// This math will overflow when timing reach 584.9424 years
	// Should be totaly enough for profiling!
	
	txtp text;
	text|SPC('0');
	
	if(s == 0)
	{
		if(ms == 0)
		{
			if(us == 0)
			{
				text|ns|L(" ns");
			}
			else
			{
				text|us|'.'|SPN(3)|ns|L(" us");
			}
		}
		else
		{
			text|ms|'.'|us|'\''|SPN(3)|ns|L(" ms");
		}
	}
	else
	{
		if(s < 60)
		{
			text|s|'.'|SPN(3)|ms|L(" s");
		}
		else if(s < 3600)
		{
			text|SPN(2)|s/60|':'|SPN(2)|s%60|L(" m");
		}
		else if(s < 86400)
		{
			text|SPN(2)|s/3600|':'|SPN(2)|(s%3600)/60|':'|SPN(2)|s%3600%60|L(" h");
		}
		else
		{
			text|s/86400|L(" d ")|SPN(2)|(s%86400)/3600|L(" h");
		}
	}

	return *text;
}

wtxt ticks2wtxt(ui64 timing)
{
	ui64 s = timing/sw_freq;
	ui64 ms = timing/(1000000/sw_ns_per_tick) - s * 1000;
	ui64 us = timing/(1000/sw_ns_per_tick) - s * 1000000 - ms * 1000;
	ui64 ns = timing * sw_ns_per_tick - s * 1000000000 - ms * 1000000 - us * 1000;
	// This math will overflow when timing reach 584.9424 years
	// Should be totaly enough for profiling!
	
	wtxtp text;
	text|SPC('0');
	
	if(s == 0)
	{
		if(ms == 0)
		{
			if(us == 0)
			{
				text|ns|WL(" ns");
			}
			else
			{
				text|us|'.'|SPN(3)|ns|WL(" us");
			}
		}
		else
		{
			text|ms|'.'|us|'\''|SPN(3)|ns|WL(" ms");
		}
	}
	else
	{
		if(s < 60)
		{
			text|s|'.'|SPN(3)|ms|WL(" s");
		}
		else if(s < 3600)
		{
			text|SPN(2)|s/60|':'|SPN(2)|s%60|WL(" m");
		}
		else if(s < 86400)
		{
			text|SPN(2)|s/3600|':'|SPN(2)|(s%3600)/60|':'|SPN(2)|s%3600%60|WL(" h");
		}
		else
		{
			text|s/86400|WL(" d ")|SPN(2)|(s%86400)/3600|WL(" h");
		}
	}

	return *text;
}

ui64a swt_st = 8;
ui64a swt_ed = 8;