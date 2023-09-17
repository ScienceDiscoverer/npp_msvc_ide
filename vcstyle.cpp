// CONSOLE
#pragma warning( push, 0 )
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#pragma warning( pop )

#define MAX_SCR_W 70

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

typedef unsigned int ui32;

int error_level;

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

void styleDiagnostics(LPCSTR text, SIZE_T l)
{
	static HANDLE ih = GetStdHandle(STD_INPUT_HANDLE);
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	static CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(oh, &csbi);
	static WORD d = csbi.wAttributes;
	static WORD c = FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN;
	static WORD r = FOREGROUND_INTENSITY | FOREGROUND_RED;
	static WORD y = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
	static WORD g = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
	
	csbi.dwSize.X = MAX_SCR_W + 1;
	csbi.srWindow.Left = 0;
	csbi.srWindow.Top = 0;
	csbi.srWindow.Right = MAX_SCR_W;
	csbi.srWindow.Bottom = 45;
	SetConsoleScreenBufferSize(oh, csbi.dwSize);
	SetConsoleWindowInfo(oh, TRUE, &csbi.srWindow);
	
	bool line_col = false, err_war = false, code = false;
	int cpars = NORM_TXT;
	
	bool entry_end = true; // This signals if end of the error/warning/note was reached
	bool code_gen_mode = *text == 'G' && *(text+14) == 'e' && *(text+16) == '\n';
	bool line_col_comma_found = false;
	
	int ew_space = 0; // Error/Warning space, used to skip space between error and it's code
	WORD ew_col = d;
	
	LPCSTR hls = NULL, hle = hls; // Highlight start/end
	LPCSTR code_end = NULL;
	
	LPCSTR te = text + l;
	--text;

	while(++text != te)
	{
		switch(cpars)
		{
		case LINE_COL:
			if(*text == ',')
			{
				line_col_comma_found = true;
				SetConsoleTextAttribute(oh, d);
				PC(text);
				SetConsoleTextAttribute(oh, c);
				goto loop_continue;
			}
			
			if(*text == ')')
			{
				SetConsoleTextAttribute(oh, d);
				PC(text);
				
				if(!line_col_comma_found)
				{
					code_gen_mode = true;
					cpars = GEN_CODE;
				}
				
				goto loop_continue;
			}
			
			if(*text == ' ')
			{
				line_col = true;
				if(*(text+1) == 'e') // error CXXXX:
				{
					ew_col = r;
					error_level |= 0x1;
				}
				else if(*(text+1) == 'f') // fatal error CXXXX:
				{
					ew_col = r;
					error_level |= 0x1;
					--ew_space;
				}
				else // warning CXXXX:
				{
					ew_col = y;
					error_level |= 0x2;
				}
				
				if(*(text+1) == 'n') // note:
				{
					ew_col = g;
					++ew_space;
				}
				SetConsoleTextAttribute(oh, ew_col);
				cpars = ERR_WAR;
			}
			
			PC(text);
			goto loop_continue;
		case ERR_WAR:
			if(*text == ' ' && ++ew_space == 2) // warning|1st|C4129:|2nd_space|
			{
				ew_space = 0;
				err_war = false;
				SetConsoleTextAttribute(oh, d);
				cpars = NORM_TXT;
			}
			PC(text);
			goto loop_continue;
		case CODE:
			if(text == code_end)
			{
				LPCSTR car = text;
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
					PC(text);
					PC("\n"); // Add new line for readability
					
					// Parsing block finished
					line_col = false;
					err_war = false;
					code = false;
					entry_end = true; // Error/Warning has ended
					cpars = NORM_TXT;
					goto loop_continue;
				}
			}
			else if(text == hls)
			{
				SetConsoleTextAttribute(oh, ew_col);
			}
			else if(text == hle)
			{
				SetConsoleTextAttribute(oh, d);
			}
			
			PC(text);
			goto loop_continue;
		case CARET:
			if(*text == '^')
			{
				SetConsoleTextAttribute(oh, ew_col);
				PC(text);
				SetConsoleTextAttribute(oh, d);
				PC("\n"); // Add new line for readability
				
				// Parsing block finished
				line_col = false;
				err_war = false;
				code = false;
				entry_end = true; // Error/Warning has ended
				cpars = NORM_TXT;
				goto loop_continue;
			}
			PC(text);
			goto loop_continue;
		case GEN_CODE:
			if(*text == ')')
			{
				SetConsoleTextAttribute(oh, d);
				PC(text);
				goto loop_continue;
			}
			
			if(*text == ':')
			{
				if(*(text+2) == 'e')
				{
					ew_col = r;
					error_level |= 0x1;
				}
				else
				{
					ew_col = y;
					error_level |= 0x2;
				}
				
				if(*(text+2) == 'n') // note:
				{
					ew_col = g;
					++ew_space;
				}
				
				PC(text);
				SetConsoleTextAttribute(oh, ew_col);
				goto loop_continue;
			}
			
			if(*(text) == ' ' && ++ew_space == 4) // |1st|:|2nd|warning|3nd|C4129:|4th_space|
			{
				ew_space = 0;
				SetConsoleTextAttribute(oh, d);
				cpars = GEN_CBODY;
			}

			PC(text);
			goto loop_continue;
		case GEN_CBODY:
			if(*text == '\n')
			{
				cpars = NORM_TXT;
				entry_end = true; // Code generation Error/Warning has ended
				if(*(text+24) != 'e' && *(text+25) != '\r') // Skip last newline 'Finished generating code'
				{
					PC("\n"); // Add new line for readability
				}
			}
			
			PC(text);
			goto loop_continue;
		case LINK_ERR:
			if(*text == ':')
			{
				error_level |= 0x1;
				PC(text);
				SetConsoleTextAttribute(oh, d);
				cpars = LINK_BODY;
				goto loop_continue;
			}
			
			PC(text);
			goto loop_continue;
		case LINK_BODY:
			if(*text == '\n')
			{
				cpars = NORM_TXT;
				entry_end = true; // Code generation Error/Warning has ended
			}
			
			PC(text);
			goto loop_continue;
		default:
			PC(text);
			break;
		}
		
		if(!line_col)
		{
			if(entry_end && lncol(text, te)) // Find line/column (xxx,xx)
			{
				SetConsoleTextAttribute(oh, c);
				cpars = code_gen_mode ? GEN_CODE : LINE_COL;
				entry_end = false;
			}
		}
		else
		{
			if(!code && *text == '\n') // Find start of code sample
			{
				cpars = CODE;
				code_end = text;
				while(++code_end != te && *code_end != '\n') // Find code snippet end
				{
					if(*code_end == '.') // Check if the code + code_end is entirely missing
					{
						LPCSTR tmp_lncol = code_end;
						while(++tmp_lncol != te && *tmp_lncol != '(' && *tmp_lncol != '\n')
						{
							;
						}
						
						if(*tmp_lncol == '(')
						{
							while(++tmp_lncol != te && *tmp_lncol != ',')
							{
								if(*tmp_lncol < '0' || *tmp_lncol > '9')
								{
									break;
								}
							}
							
							if(*tmp_lncol == ',')
							{
								while(++tmp_lncol != te && *tmp_lncol != ')')
								{
									if(*tmp_lncol < '0' || *tmp_lncol > '9')
									{
										break;
									}
								}
								
								if(*tmp_lncol == ')' && ++tmp_lncol != te && *tmp_lncol == ':')
								{
									SetConsoleTextAttribute(oh, d);
									PC("\n"); // Add new line for readability
									
									// Parsing block finished
									line_col = false;
									err_war = false;
									code = false;
									entry_end = true; // Error/Warning has ended
									cpars = NORM_TXT;
									goto loop_continue;
								}
							}
						}
					}
				}
				LPCSTR caret = code_end;
				
				if(caret == te) // End of the input was reached
				{
					goto loop_continue;
				}
				
				while(++caret != te && *caret != '^') // Find caret
				{
					;
				}
				SIZE_T caret_pos = SIZE_T(caret - code_end - 1);
				
				hls = text + caret_pos + 1, hle = hls; // Find highlight start/end
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
		if(entry_end && *text == ':' && (*(text+8) == 'L' || *(text+8) == 'e')
			&& (*(text+9) == 'N' || *(text+9) == 'r'))
		{
			SetConsoleTextAttribute(oh, r);
			cpars = LINK_ERR;
			entry_end = false;
		}
		
		// Detect '\nGenerating code\r\n'
		if(!code_gen_mode && *text == '\n' && *(text+1) == 'G' && *(text+15) == 'e' && *(text+17) == '\n')
		{
			code_gen_mode = true;
		}
		
		loop_continue:;
	}
	
	COORD buf = csbi.dwSize;
	SHORT wnd_h = csbi.srWindow.Bottom - csbi.srWindow.Top;
	
	SMALL_RECT scrsz = csbi.srWindow;
	scrsz.Left = 0;
	scrsz.Top = 0;
	scrsz.Right = csbi.srWindow.Right - csbi.srWindow.Left;
	scrsz.Bottom = wnd_h;
	SetConsoleScreenBufferSize(oh, buf);
	SetConsoleWindowInfo(oh, TRUE, &scrsz);
	
	CONSOLE_CURSOR_INFO cur;
	cur.dwSize = 25;
	cur.bVisible = FALSE;
	SetConsoleCursorInfo(oh, &cur);
	
	// MB 2 SEPARATE STRAMS ARE SENT TO PIPE AND CONSOLE POSITON RESETZ DA POS!!!
	
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
	LPSTR text = (LPSTR)HeapAlloc(heap, NULL, txts);

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
			memcpy(tmp, text, txts);
			HeapFree(heap, NULL, text);
			text = tmp;
			txts = tmps;
		}
		
		memcpy(text+txtl, buff, rd);
		txtl += rd;
	}
	
	//perr(); // GetLastError ---> [E:109] The pipe has been ended.
	// Pipe end also disables system("pause") command!
	
	styleDiagnostics(text, rd_tot);

	HeapFree(heap, NULL, text);
	
	return error_level & 0x1 ? 1 : error_level & 0x2 ? 2 : 0;
}