// CONSOLE
#pragma warning( push, 0 )
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#pragma warning( pop )

// #include "sddb.h"

bool lncol(LPCSTR t, LPCSTR te) // ....([0-9].....
{
	LPCSTR la = t + 1; // Look Ahead
	if(la > te)
	{
		return false;
	}
	
	if(*t != '(')
	{
		return false;
	}
	
	if(*(la) >= '0' && *(la) <= '9')
	{
		return true;
	}
	
	return false;
}

#define MAX_SCR_W 119

void calcLines(SIZE_T *line, SIZE_T *lines)
{
	if(*line > MAX_SCR_W)
	{
		*lines += *line/MAX_SCR_W;
		*lines += *line % MAX_SCR_W > 0 ? 1 : 0;
	}
	else
	{
		++(*lines);
	}
}

#define PC(x) WriteFile(oh, (x), 1, NULL, NULL)

#define NORM_TXT  0x0
#define LINE_COL  0x1
#define ERR_WAR   0x2
#define CODE      0x3
#define CARET     0x4
#define GEN_CODE  0x5
#define GEN_CBODY 0x6
#define LINK_ERR  0X7
#define LINK_BODY 0X8

int error_level;

void styleDiagnostics(LPCSTR txt, SIZE_T l)
{
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	static CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(oh, &csbi);
	static WORD d = csbi.wAttributes;
	static WORD c = FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN;
	static WORD r = FOREGROUND_INTENSITY | FOREGROUND_RED;
	static WORD y = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
	static WORD g = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
	
	bool line_col = false, err_war = false, code = false;
	int cpars = NORM_TXT;
	
	bool entry_end = true; // This signals if end of the error/warning/note was reached
	bool code_gen_mode = *txt == 'G' && *(txt+14) == 'e' && *(txt+16) == '\n';
	bool line_col_comma_found = false;
	
	int ew_space = 0; // Error/Warning space, used to skip space between error and it's code
	WORD ew_col = d;
	
	LPCSTR hls = NULL, hle = hls; // Highlight start/end
	LPCSTR code_end = NULL;
	
	LPCSTR te = txt + l;
	SIZE_T lines = 0;
	SIZE_T line = 0;
	--txt;
	while(++txt != te)
	{
		++line;
		if(*txt == '\n')
		{
			calcLines(&line, &lines);
			line = 0;
		}
		
		switch(cpars)
		{
		case LINE_COL:
			if(*txt == ',')
			{
				line_col_comma_found = true;
				SetConsoleTextAttribute(oh, d);
				PC(txt);
				SetConsoleTextAttribute(oh, c);
				goto loop_continue;
			}
			
			if(*txt == ')')
			{
				SetConsoleTextAttribute(oh, d);
				PC(txt);
				
				if(!line_col_comma_found)
				{
					code_gen_mode = true;
					cpars = GEN_CODE;
				}
				
				goto loop_continue;
			}
			
			if(*txt == ' ')
			{
				line_col = true;
				if(*(txt+1) == 'e')
				{
					ew_col = r;
					error_level |= 0x1;
				}
				else if(*(txt+1) == 'f')
				{
					ew_col = r;
					error_level |= 0x1;
					--ew_space;
				}
				else
				{
					ew_col = y;
					error_level |= 0x2;
				}
				
				if(*(txt+1) == 'n') // note:
				{
					ew_col = g;
					++ew_space;
				}
				SetConsoleTextAttribute(oh, ew_col);
				cpars = ERR_WAR;
			}
			
			PC(txt);
			goto loop_continue;
		case ERR_WAR:
			if(*txt == ' ' && ++ew_space == 2) // warning|1st|C4129:|2nd_space|
			{
				ew_space = 0;
				err_war = false;
				SetConsoleTextAttribute(oh, d);
				cpars = NORM_TXT;
			}
			PC(txt);
			goto loop_continue;
		case CODE:
			if(txt == code_end)
			{
				LPCSTR car = txt;
				while(*(++car) == ' ' || *(car) == '\t') // Confirm presence of carete
				{
					;
				}

				code = true;
				
				if(*car == '^')
				{
					cpars = CARET;
				}
				else
				{
					PC(txt);
					PC("\n"); // Add new line for readability
					++lines;
					
					// Parsing block finished
					line_col = false;
					err_war = false;
					code = false;
					entry_end = true; // Error/Warning has ended
					cpars = NORM_TXT;
					goto loop_continue;
				}
			}
			else if(txt == hls)
			{
				SetConsoleTextAttribute(oh, ew_col);
			}
			else if(txt == hle)
			{
				SetConsoleTextAttribute(oh, d);
			}
			
			PC(txt);
			goto loop_continue;
		case CARET:
			if(*txt == '^')
			{
				SetConsoleTextAttribute(oh, ew_col);
				PC(txt);
				SetConsoleTextAttribute(oh, d);
				PC("\n"); // Add new line for readability
				++lines; // I forgot to add this. Was debugging for good hour...
				
				// Parsing block finished
				line_col = false;
				err_war = false;
				code = false;
				entry_end = true; // Error/Warning has ended
				cpars = NORM_TXT;
				goto loop_continue;
			}
			PC(txt);
			goto loop_continue;
		case GEN_CODE:
			if(*txt == ')')
			{
				SetConsoleTextAttribute(oh, d);
				PC(txt);
				goto loop_continue;
			}
			
			if(*txt == ':')
			{
				if(*(txt+2) == 'e')
				{
					ew_col = r;
					error_level |= 0x1;
				}
				else
				{
					ew_col = y;
					error_level |= 0x2;
				}
				
				if(*(txt+2) == 'n') // note:
				{
					ew_col = g;
					++ew_space;
				}
				
				PC(txt);
				SetConsoleTextAttribute(oh, ew_col);
				goto loop_continue;
			}
			
			if(*(txt) == ' ' && ++ew_space == 4) // |1st|:|2nd|warning|3nd|C4129:|4th_space|
			{
				ew_space = 0;
				SetConsoleTextAttribute(oh, d);
				cpars = GEN_CBODY;
			}

			PC(txt);
			goto loop_continue;
		case GEN_CBODY:
			if(*txt == '\n')
			{
				cpars = NORM_TXT;
				entry_end = true; // Code generation Error/Warning has ended
				if(*(txt+24) != 'e' && *(txt+25) != '\r') // Skip last newline 'Finished generating code'
				{
					PC("\n"); // Add new line for readability
					++lines;
				}
			}
			
			PC(txt);
			goto loop_continue;
		case LINK_ERR:
			if(*txt == ':')
			{
				error_level |= 0x1;
				PC(txt);
				SetConsoleTextAttribute(oh, d);
				cpars = LINK_BODY;
				goto loop_continue;
			}
			
			PC(txt);
			goto loop_continue;
		case LINK_BODY:
			if(*txt == '\n')
			{
				cpars = NORM_TXT;
				entry_end = true; // Code generation Error/Warning has ended
			}
			
			PC(txt);
			goto loop_continue;
		default:
			PC(txt);
			break;
		}
		
		if(!line_col)
		{
			if(entry_end && lncol(txt, te)) // Find line/column (xxx,xx)
			{
				SetConsoleTextAttribute(oh, c);
				cpars = code_gen_mode ? GEN_CODE : LINE_COL;
				entry_end = false;
			}
		}
		else
		{
			if(!code && *txt == '\n') // Find start of code sample
			{
				cpars = CODE;
				
				code_end = txt;
				while(++code_end != te && *code_end != '\n') // Find code snippet end
				{
					;
				}
				LPCSTR caret = code_end;
				while(++caret != te && *caret != '^') // Find caret
				{
					;
				}
				SIZE_T caret_pos = SIZE_T(caret - code_end - 1);
				
				hls = txt + caret_pos + 1, hle = hls; // Find highlight start/end
				while(*(--hls) != ' ' && *(hls) != '\n')
				{
					;
				}
				while(*(++hle) != ' ' && *(hle) != '\n')
				{
					;
				}
				++hls;
			}
		}
		
		// Detect Linker Errors i.e. 'LNK2001'
		if(entry_end && *txt == ':' && (*(txt+8) == 'L' || *(txt+8) == 'e')
			&& (*(txt+9) == 'N' || *(txt+9) == 'r'))
		{
			SetConsoleTextAttribute(oh, r);
			cpars = LINK_ERR;
			entry_end = false;
		}
		
		// Detect '\nGenerating code\r\n'
		if(!code_gen_mode && *txt == '\n' && *(txt+1) == 'G' && *(txt+15) == 'e' && *(txt+17) == '\n')
		{
			code_gen_mode = true;
		}
		
		loop_continue:;
	}

	if(*(--txt) != '\n') // No terminating newline found
	{
		calcLines(&line, &lines);
	}
	
	lines += 2; // For blank 2 lines in beginning
	
	COORD buf = csbi.dwSize;
	SHORT wnd_h = csbi.srWindow.Bottom - csbi.srWindow.Top;
	
	buf.Y = lines < (SIZE_T)wnd_h ? wnd_h + 1 : lines > 0x7FFF ? 0x7FFF : (SHORT)lines + 1;
	
	SMALL_RECT scrsz = csbi.srWindow;
	scrsz.Left = 0;
	scrsz.Top = 0;
	scrsz.Right = buf.X-1;
	scrsz.Bottom = wnd_h;
	SetConsoleScreenBufferSize(oh, buf);
	SetConsoleWindowInfo(oh, TRUE, &scrsz);
	
	CONSOLE_CURSOR_INFO cur;
	cur.dwSize = 25;
	cur.bVisible = FALSE;
	SetConsoleCursorInfo(oh, &cur);
	
	COORD p;
	p.X = 0;
	p.Y = 0;
	SetConsoleCursorPosition(oh, p);
}

#define BUFF_SZ 4176 // CMD's |(pipe) max buffer
char buff[BUFF_SZ];

int wmain(/* int argc, wchar_t**argv */)
{
	HANDLE ih = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	
	SIZE_T txts = 0x8000; // 32768 bytes
	SIZE_T txtl = 0; // Current length of text array
	HANDLE heap = GetProcessHeap();
	LPSTR txt = (LPSTR)HeapAlloc(heap, NULL, txts);
	
	PC("\n");
	PC("\n");
	
	DWORD rd = 0;
	SIZE_T rd_tot = 0;
	while(ReadFile(ih, buff, BUFF_SZ, &rd, NULL))
	{
		rd_tot += rd;
		
		if(rd_tot > txts)
		{
			SIZE_T tmps = txts * 2;
			LPSTR tmp = (LPSTR)HeapAlloc(heap, NULL, tmps);
			memcpy(tmp, txt, txts);
			HeapFree(heap, NULL, txt);
			txt = tmp;
			txts = tmps;
		}
		
		memcpy(txt+txtl, buff, rd);
		txtl += rd;
	}
	
	//perr(); // GetLastError ---> [E:109] The pipe has been ended.
	// Pipe end also disables system("pause") command!
	
	styleDiagnostics(txt, rd_tot);
	
	HeapFree(heap, NULL, txt);
	
	return error_level & 0x1 ? 1 : error_level & 0x2 ? 2 : 0;
}