// SLIB
#include "stdp"

#include <darr>
#include <ui64a>

struct Cell
{
	txt t;		// Text
	WORD col;	// Color
	ui16 al;	// Alignment
	ui32 reserved_1;
};

class Row : public darr
{
public:
	Row() { d = NULL; }
	Row(const Row &o) = delete;
	
	~Row()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			txtfree(d[i].t);
		}
		DARR_FREE(d);
	}
	
	Row & operator=(const Row &o)
	{
		for(ui64 i = 0; i < s; ++i)
		{
			txtfree(d[i].t); // No need to invalidate texts. Yet
		}

		if(ts < o.s)
		{
			DARR_FREE(d);
			ts = o.s;
			d = (Cell *)Alloc(ts * sizeof(Cell));
			s = ts;
		}

		memset(d, 0, s * sizeof(Cell)); // All texts invalidated

		s = o.s;
		for(ui64 i = 0; i < s; ++i)
		{
			d[i].t = o.d[i].t;
			d[i].col = o.d[i].col;
			d[i].al = o.d[i].al;
		}

		return *this;
	}
	
	Cell & operator[](ui64 i) { return d[i]; }
	const Cell & operator[](ui64 i) const  { return d[i]; }
	
	Row & operator<<(const Cell &c)
	{
		ui64 ns = s + 1;
		if(ts < ns)
		{
			d = (Cell *)ReAlloc(d, ns, sizeof(Cell));
			memset(&d[s], 0, (ts - s) * sizeof(Cell)); // All new texts invalidated
		}

		d[s] = c;
		s = ns;
		
		return *this;
	}
	
	Row & Clear()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			txtfree(d[i].t); // Will invalidate texts later
		}
		memset(d, 0, s * sizeof(Cell)); // All texts invalidated
		s = 0;
		return *this;
	}
	
private:
	Cell *d;
};

class Table : public darr
{
public:
	Table() { d = NULL; }
	Table(const Table &o) = delete;
	
	~Table()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			d[i].~Row();
		}
		DARR_FREE(d);
	}
	
	Table & operator=(const Table &o) = delete;
	
	Row & operator[](ui64 i) { return d[i]; }
	const Row & operator[](ui64 i) const  { return d[i]; }
	
	Table & operator<<(const Row &r)
	{
		ui64 ns = s + 1;
		if(ts < ns)
		{
			d = (Row *)ReAlloc(d, ns, sizeof(Table));
			memset(&d[s], 0, (ts - s) * sizeof(Row)); // All new rows initialised
		}

		d[s] = r;
		s = ns;
		
		return *this;
	}
	
	Table & Clear()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			d[i].~Row();
		}
		
		memset(d, 0, s * sizeof(Row)); // All rows invalidated
		s = 0;
		return *this;
	}
	
private:
	Row *d;
};

DWORD StdPrinter::bw;
WORD StdPrinter::defc = StdPrinter::GetDefConParams();
CONSOLE_CURSOR_INFO StdPrinter::con;
CONSOLE_CURSOR_INFO StdPrinter::coff;
CONSOLE_CURSOR_INFO StdPrinter::cdef;
DWORD StdPrinter::defcm;
CONSOLE_SCREEN_BUFFER_INFO StdPrinter::dcsbi;
wchar_t StdPrinter::defpad = ' ';

StdPrinter p;

Table t;
Row cr;					// Current row
ui64 prev_row;
ui64a col_w = 8;		// Maximum width of each column
ui64a col_add_w = 8;	// Additional padding width for each column
ui64 col_i;

StdPrinter & TB(StdPrinter &stdp)
{
    /* typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
	  COORD      dwSize;
	  COORD      dwCursorPosition;
	  WORD       wAttributes;
	  SMALL_RECT srWindow;
	  COORD      dwMaximumWindowSize;
	} CONSOLE_SCREEN_BUFFER_INFO; */
	
	static CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(stdp.oh, &csbi);
	
	static HANDLE heap = GetProcessHeap();
	CHAR_INFO *pci = (CHAR_INFO *)HeapAlloc(heap, 0, csbi.dwSize.X * csbi.dwSize.Y * sizeof(CHAR_INFO));
	
	/* typedef struct _CHAR_INFO {
	  union {
		WCHAR UnicodeChar;
		CHAR  AsciiChar;
	  } Char;
	  WORD  Attributes;
	} CHAR_INFO, *PCHAR_INFO; */
	
	SMALL_RECT read_region;
	read_region.Left = 0;
	read_region.Top = 0;
	read_region.Right = csbi.dwSize.X;
	read_region.Bottom = csbi.dwSize.Y;
	COORD buff_sz = csbi.dwSize;
	COORD buff_coord = {0};
	ReadConsoleOutputA(stdp.oh,	pci, buff_sz, buff_coord, &read_region);
	
	ui64 last_row = 0;
	for(ui32 i = 0; i < (ui32)csbi.dwSize.Y; ++i)
	{
		for(ui32 j = 0; j < (ui32)csbi.dwSize.X; ++j)
		{
			if(pci[i * csbi.dwSize.X + j].Char.AsciiChar != ' ')
			{
				last_row = 0;
				goto continue_main_loop;
			}
		}
		
		if(last_row == 0)
		{
			last_row = i;
		}
	continue_main_loop:;
	}
	
	SHORT wnd_sz = csbi.srWindow.Bottom - csbi.srWindow.Top;
	buff_sz.Y = (SHORT)last_row + 1;
	if(buff_sz.Y < wnd_sz)
	{
		buff_sz.Y = wnd_sz + 1;
	}
	
	// Window size must be always at least 1 cell less than buffer size
	// Reason: probably because srWindow is coordinates, swSize is actual number of cells
	SetConsoleScreenBufferSize(stdp.oh, buff_sz);
	
	HeapFree(heap, 0, pci);
	
	return stdp;
}

StdPrinter & TE(StdPrinter &stdp)
{
	stdp.tmode = false;
	
	t << cr;
	
	for(ui64 i = 0; i < ~t; ++i)
	{
		for(ui64 j = 0; j < ~t[i]; ++j)
		{
			SetConsoleTextAttribute(stdp.oh, t[i][j].col);
			
			ui64 pad = col_w[j];
			if(j < ~col_add_w)
			{
				pad += col_add_w[j];
			}
			
			if(t[i][j].al == TABLE_CENTER)
			{
				ui64 pure_pad = pad - ~t[i][j].t;
				ui64 lr_pad = pure_pad/2;
				p|S(lr_pad)|t[i][j].t|S(lr_pad + pure_pad % 2)|S;
				continue;
			}
			
			ui64 pad_dir = t[i][j].al == TABLE_LEFT ? PADR : PADL;
			p|DP|SPN(pad)|SPDN(pad_dir)|t[i][j].t|S;
		}
		p|N;
	}
	
	p|DS;
	prev_row = 0;
	col_i = 0;
	t.Clear();
	col_w.Clear();
	col_add_w.Clear();
	cr.Clear();
	stdp.tcelal = TABLE_LEFT;
	return stdp;
}

StdPrinter & StdPrinter::operator|(StdTableColPad pn)
{
	col_add_w << pn.amount;
	return *this;
}

void StdPrinter::PrintToTable(const txt &cell)
{
	if(trow != prev_row)
	{
		prev_row = trow;
		t << cr;
		col_i = 0;
		cr.Clear();
	}
	
	Cell c;
	c.t = cell;
	c.col = txtattr;
	c.al = tcelal;
	
	cr << c;
	
	if(~col_w == col_i)
	{
		col_w << 0;
	}
	
	if(col_w[col_i] < ~cell)
	{
		col_w[col_i] = ~cell;
	}
	++col_i;
}




// BLACK      	0x00 --> 0
// DARKBLUE   	0x01 --> FOREGROUND_BLUE
// DARKGREEN  	0x02 --> FOREGROUND_GREEN
// DARKCYAN   	0x03 --> FOREGROUND_GREEN | FOREGROUND_BLUE
// DARKRED    	0x04 --> FOREGROUND_RED
// DARKMAGENTA	0x05 --> FOREGROUND_RED | FOREGROUND_BLUE
// DARKYELLOW 	0x06 --> FOREGROUND_RED | FOREGROUND_GREEN
// GRAY   		0x07 --> FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
// DARKGRAY     0x08 --> FOREGROUND_INTENSITY
// BLUE       	0x09 --> FOREGROUND_INTENSITY | FOREGROUND_BLUE
// GREEN      	0x0A --> FOREGROUND_INTENSITY | FOREGROUND_GREEN
// CYAN       	0x0B --> FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE
// RED        	0x0C --> FOREGROUND_INTENSITY | FOREGROUND_RED
// MAGENTA    	0x0D --> FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE
// YELLOW     	0x0E --> FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN
// WHITE      	0x0F --> FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE

// BLACK      	0x00 --> 0
// DARKBLUE   	0x10 --> BACKGROUND_BLUE
// DARKGREEN  	0x20 --> BACKGROUND_GREEN
// DARKCYAN   	0x30 --> BACKGROUND_GREEN | BACKGROUND_BLUE
// DARKRED    	0x40 --> BACKGROUND_RED
// DARKMAGENTA	0x50 --> BACKGROUND_RED | BACKGROUND_BLUE
// DARKYELLOW 	0x60 --> BACKGROUND_RED | BACKGROUND_GREEN
// LIGHTGRAY   	0x70 --> BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
// DARKGRAY     0x80 --> BACKGROUND_INTENSITY
// BLUE       	0x90 --> BACKGROUND_INTENSITY | BACKGROUND_BLUE
// GREEN      	0xA0 --> BACKGROUND_INTENSITY | BACKGROUND_GREEN
// CYAN       	0xB0 --> BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE
// RED        	0xC0 --> BACKGROUND_INTENSITY | BACKGROUND_RED
// MAGENTA    	0xD0 --> BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE
// YELLOW     	0xE0 --> BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN
// WHITE      	0xF0 --> BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE

//	COMMON_LVB_LEADING_BYTE 		// 0x0100 	Leading byte.
//	COMMON_LVB_TRAILING_BYTE 		// 0x0200 	Trailing byte.
//	COMMON_LVB_GRID_HORIZONTAL 		// 0x0400 	Top horizontal.
//	COMMON_LVB_GRID_LVERTICAL 		// 0x0800 	Left vertical.
//	COMMON_LVB_GRID_RVERTICAL 		// 0x1000 	Right vertical.
//	COMMON_LVB_REVERSE_VIDEO 		// 0x4000 	Reverse foreground and background attribute ! ONLY WORKING ONE !
//	COMMON_LVB_UNDERSCORE 			// 0x8000 	Underscore.