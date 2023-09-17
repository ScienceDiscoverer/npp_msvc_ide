// CONSOLE
#include <stdp>
#include <txta>
#include <ui64a>

#include <shellapi.h>

#pragma comment( lib, "user32")
#pragma comment( lib, "shell32")

// 1 param -> only WPARAM, 2 params WPARAM and LPARAM → -> return value meaning
#define SCI_GETCURRENTPOS			2008 // → position
#define SCI_GOTOLINE				2024 // (line line)
#define SCI_SETSELECTIONSTART		2142 // (position anchor)
#define SCI_GETSELECTIONSTART		2143 // → position
#define SCI_SETSELECTIONEND			2144 // (position caret)
#define SCI_GETSELECTIONEND			2145 // → position
#define SCI_GETFIRSTVISIBLELINE		2152 // → line
#define SCI_SETSEL					2160 // (position anchor, position caret)
#define SCI_POSITIONFROMLINE		2167 // (line line) → position
#define SCI_GETTEXTLENGTH			2183 // → position
#define SCI_VISIBLEFROMDOCLINE		2220 // (line docLine) → line
#define SCI_DOCLINEFROMVISIBLE		2221 // (line displayLine) → line
#define SCI_GETLINEVISIBLE			2228 // (line line) → bool
#define SCI_ENSUREVISIBLE			2232 // (line line)
#define SCI_LINELENGTH				2350 // (line line) → position
#define SCI_LINESONSCREEN			2370 // → line
#define SCI_SETEMPTYSELECTION		2556 // (position caret)
#define SCI_SETFIRSTVISIBLELINE		2613 // (line displayLine)

#define NPPMSG				(WM_USER + 1000)
#define NPPM_MENUCOMMAND	(NPPMSG + 48)		// void NPPM_MENUCOMMAND(0, int cmdID)

#define    IDM    							40000
#define    IDM_SEARCH    					(IDM + 3000)
#define    IDM_SEARCH_MARKALLEXT1			(IDM_SEARCH + 22)
#define    IDM_SEARCH_UNMARKALLEXT1			(IDM_SEARCH + 23)
#define    IDM_SEARCH_MARKALLEXT2			(IDM_SEARCH + 24)
#define    IDM_SEARCH_UNMARKALLEXT2			(IDM_SEARCH + 25)
#define    IDM_SEARCH_MARKALLEXT3			(IDM_SEARCH + 26)
#define    IDM_SEARCH_UNMARKALLEXT3			(IDM_SEARCH + 27)
#define    IDM_SEARCH_MARKALLEXT4			(IDM_SEARCH + 28)
#define    IDM_SEARCH_UNMARKALLEXT4			(IDM_SEARCH + 29)
#define    IDM_SEARCH_MARKALLEXT5			(IDM_SEARCH + 30)
#define    IDM_SEARCH_UNMARKALLEXT5			(IDM_SEARCH + 31)
#define    IDM_SEARCH_CLEARALLMARKS			(IDM_SEARCH + 32)
#define    IDM_SEARCH_MARKONEEXT1			(IDM_SEARCH + 62)
#define    IDM_SEARCH_MARKONEEXT2			(IDM_SEARCH + 63)
#define    IDM_SEARCH_MARKONEEXT3			(IDM_SEARCH + 64)
#define    IDM_SEARCH_MARKONEEXT4			(IDM_SEARCH + 65)
#define    IDM_SEARCH_MARKONEEXT5			(IDM_SEARCH + 66)

#define SCROLL_LINES 4

#define T_ERROR		0
#define T_WARNING	1

#define CODE_SIZE	5 // Size of error/warning code string

#ifdef UNICODE
#define TOWSTR(x) L ## x
#else
#define TOWSTR(x) x
#endif

#define APP_GUID "-{AAA6D28F-DF14-4570-A273-21DD2708F5AD}"
#define REGISTER_MESSAGE(n) \
	static const UINT n = RegisterWindowMessage(TOWSTR(#n APP_GUID))
	
//#define NPP_TOGGLE_TOPMOST

struct Error
{
	txt file;
	txt code;
	ui32 line;
	ui32 pos_il; // Position in line
	ui32 type;
	ui32 x; // Coord of first digit in line number
	ui32 y;
	ui32 s; // Size of number
	ui32 dist_to_code; // From first digit of line number
	ui32 reserved;
};

#ifndef ErrorDarr // Dynamic/|\Array ////////////////////////////////////////
class ErrorDarr
{
public:
	ErrorDarr() : ts(0), s(0), d(nullptr) {}
	ErrorDarr(const ErrorDarr &o) = delete;
	ErrorDarr(ErrorDarr &&o) = delete;
	
	~ErrorDarr()	{ if(d != nullptr) VirtualFree(d, 0, MEM_RELEASE); } // CALL DESTRUCTOS OF COMPLEX OBJECTS HERE!
	
	ErrorDarr & operator=(const ErrorDarr &o) = delete;
	ErrorDarr & operator=(ErrorDarr &&o) = delete;
	
	Error & operator[](ui64 i)
	{
#ifdef DEBUG
		if(i >= s) RaiseException(0xE0000011, 0, 0, 0);
#endif
		return d[i];
	}
	const Error & operator[](ui64 i) const
	{
#ifdef DEBUG
		if(i >= s) RaiseException(0xE0000011, 0, 0, 0);
#endif
		return d[i];
	}
	
	ui64 operator~() const { return s; }  // Size operator
	ui64 operator!() const { return ts; } // True Size Operator
	
	ErrorDarr & operator<<(const Error &data)
	{
		ui64 ns = s + 1;
		if(ts < ns)
		{
			ts = ns < 8 ? 8 : ns * 2; // REALLOC_MULT
			Error *tmp = (Error *)VirtualAlloc(NULL, ts * sizeof(Error),
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if(d != nullptr)
			{
				mcpy(tmp, d, s * sizeof(Error));
				VirtualFree(d, 0, MEM_RELEASE);
			}
			d = tmp;
		}

		d[s] = data;
		s = ns;
		
		return *this;
	}
	
private:
	ui64 ts;
	ui64 s;
	Error *d;
};
#endif // Dynamic/|\Array ////////////////////////////////////////

REGISTER_MESSAGE(NPP_PLUGIN_GET_SCINTILLA);
REGISTER_MESSAGE(NPP_PLUGIN_GET_CUR_FILE_NAME);
const wchar_t *msg_wnd_class = L"ErrLogsMsgWindow";
const wchar_t *plugin_msg_wnd_class = L"SdppNppPluginMsgWindow";
HWND msg_wnd;
HWND plugin_wnd;

CHAR_INFO *output;
SHORT ox;
SHORT oy;
ui64 out_size;

HANDLE pause_event;
HANDLE ih, oh;

HWND nppw, scintilla;
HWND conw;

ErrorDarr errs;

txt cur_file;
txta uniq_files(8);
txta uniq_files_no_ext(8);
ui64a scintillas;

bool npp_was_topmost;

i32 return_value = 1; // For error_level 1 (error)

ui64 con_cell_w, con_cell_h;
bool text_selected;

void scroll(WPARAM dir);
void nppMarkLine(ui64 line, ui64 type);
void nppGoToLine(ui64 line, ui64 pos_il);
void nppOpenFile(txt file);
void nppUpdateCurrFile();
void searchErrorWeb(txt error);
WORD invert(WORD ch_style);
DWORD NOTHROW deselectText(LPVOID lp);
void blinkText(SHORT x, SHORT y, ui32 s);
void lmbProc(const MOUSE_EVENT_RECORD *m);
DWORD NOTHROW inputThread(LPVOID lp);
BOOL NOTHROW nppCleanup(DWORD ctrl_sig);
LRESULT CALLBACK msgWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void spawnMsgWnd();
DWORD NOTHROW msgProcThread(LPVOID lp);
bool isWndTopmost(HWND wnd);
bool isLinNum(const txt &t, ui64 pos);
void setFocusToNpp();
void setFocusToMyself();
DWORD NOTHROW mousePollThread(LPVOID lp);

i32 wmain()
{
	p|TB|DC;
	
	ih = GetStdHandle(STD_INPUT_HANDLE);
	oh = GetStdHandle(STD_OUTPUT_HANDLE);
	bool has_errors = false;
	
	plugin_wnd = FindWindowEx(HWND_MESSAGE, NULL, plugin_msg_wnd_class, NULL);
	
	pause_event = CreateEventA(NULL, TRUE, FALSE, NULL);
	CreateThread(NULL, 0, inputThread, NULL, 0, NULL);
	CreateThread(NULL, 0, msgProcThread, NULL, 0, NULL);
	CreateThread(NULL, 0, mousePollThread, NULL, 0, NULL);
	
	SetConsoleMode(ih, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);
	SetConsoleCtrlHandler(nppCleanup, TRUE);
	conw = GetConsoleWindow();
	
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(oh, &csbi);
	ox = csbi.dwSize.X;
	oy = csbi.dwSize.Y - 1; // Maybe to skip caret?
	out_size = (ui64)(ox * oy);
	output = (CHAR_INFO *)VirtualAlloc(NULL, out_size * sizeof(CHAR_INFO), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	COORD buff_sz = { ox , oy };
	COORD buff_co = { 0 , 0 };
	SMALL_RECT to_read;
	to_read.Left = 0;
	to_read.Top = 0;
	to_read.Right = ox;
	to_read.Bottom = oy;
	
	RECT rec;
	GetClientRect(conw, &rec);
	con_cell_w = (ui64)((rec.right - rec.left)/csbi.srWindow.Right);
	con_cell_h = (ui64)((rec.bottom - rec.top)/csbi.srWindow.Bottom);

	ReadConsoleOutput(oh, output, buff_sz, buff_co, &to_read);
	
	txt tbuf = out_size;
	for(ui64 i = 0; i < out_size; ++i)
	{
		tbuf += output[i].Char.AsciiChar;
	}
	
	for(ui64 i = 0; i < ~tbuf; ++i)
	{
		if(i % ox == 0 && tbuf[i] >= 'A' && tbuf[i] <= 'Z' && txtfr(tbuf, i, i+2, L(":\\")) != NFND)
		{
			Error err;
			
			ui64 op_brak = txtf(tbuf, i, '(');
			while(!isLinNum(tbuf, op_brak))
			{
				op_brak = txtf(tbuf, op_brak+1, '(');
			}
			
			err.y = ui32(i/(ui64)ox);
			err.x = ui32((op_brak + 1) % (ui64)ox);
			err.file = txtsp(tbuf, i, op_brak-1);
			
			ui64 cl_brak = txtf(tbuf, op_brak, ')');
			ui64 comma = txtfr(tbuf, op_brak+1, cl_brak-1, ',');
			txt line;
			if(comma != NFND)
			{
				line = txtsp(tbuf, op_brak+1, comma-1);
				err.pos_il = (ui32)t2i(txtsp(tbuf, comma+1, cl_brak-1));
			}
			else
			{
				line = txtsp(tbuf, op_brak+1, cl_brak-1);
				err.pos_il = 0;
			}
			err.line = (ui32)t2i(line);
			err.s = (ui32)~line;
			
			ui64 type_s = txtfr(tbuf, cl_brak+1, cl_brak+8, 'a', 'z'); // Start of type
			if(tbuf[type_s] == 'e' || tbuf[type_s] == 'f') // [f] for "fatal error"
			{
				type_s = txtfr(tbuf, cl_brak+1, cl_brak+8, 'e');
				err.type = T_ERROR;
				err.code = txtsp(tbuf, type_s+6, txtf(tbuf, type_s+6, ':')-1);
				err.dist_to_code = ui32(type_s - op_brak + 5);
				uniq_files.AddUnique(err.file);
			}
			else if(tbuf[type_s] == 'w')
			{
				err.type = T_WARNING;
				err.code = txtsp(tbuf, type_s+8, txtf(tbuf, type_s+8, ':')-1);
				
				err.dist_to_code = ui32(type_s - op_brak + 7);
				// D:\P\MT\s.cpp(555) : warning C4702: unreachable code
				// 13-----------^       ^
				// 21-------------------|
				uniq_files.AddUnique(err.file);
			}
			else // note
			{
				goto skip_array_append;
			}
			
			errs << err;
		skip_array_append:
			i += ~err.file + err.s + 6;
		}
	}
	
	for(ui64 i = 0; i < ~uniq_files; ++i)
	{
		uniq_files_no_ext.AddUnique(uniq_files[i]);
		ui64 lst_slash = txtfe(uniq_files[i], TEND, '\\');
		ui64 dot = txtfr(uniq_files[i], lst_slash+1, TEND, '.');
		if(dot != NFND)
		{
			txtd(uniq_files_no_ext[i], dot, TEND);
		}
	}
	
	scintilla = (HWND)SendMessage(plugin_wnd, NPP_PLUGIN_GET_SCINTILLA, 0, 0);
	
	if(scintilla == NULL)
	{
		p|R|"[      SCINTILLA IS 0!      ]"|D|N;
		WaitForSingleObject(pause_event, INFINITE);
		return 1;
	}
	
	// Find Notepad++ window from it's kid
	nppw = GetParent(scintilla);
	if(nppw == NULL)
	{
		return 1;
	}
	
	// Allign console to the mid of Notepad++ and make NPP topmost, it it wasn't
	RECT npp_rect, con_rect, work_rect;
	GetWindowRect(nppw, &npp_rect);
	GetWindowRect(conw, &con_rect);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_rect, 0);
	
	i64 npp_h = npp_rect.bottom - npp_rect.top;
	i64 con_h = con_rect.bottom - con_rect.top;
	i64 con_w = con_rect.right - con_rect.left;
	i64 vshift = (npp_h - con_h)/2;
	i64 hshift = con_w - 1;
	i32 conx = i32(npp_rect.left - hshift);
	i32 cony = i32(npp_rect.top + vshift);
	
	i32 nppx = npp_rect.left, nppy = npp_rect.top, delta = 0, con_bott = cony + (i32)con_h; 
	if(conx < work_rect.left)
	{
		delta = work_rect.left - conx;
		conx += delta;
		nppx += delta;
	}
	if(con_bott > work_rect.bottom)
	{
		delta = con_bott - work_rect.bottom;
		cony -= delta;
		nppy -= delta;
	}

	SetWindowPos(conw, HWND_NOTOPMOST, conx, cony, 0, 0, SWP_NOSIZE);
	if(nppx != npp_rect.left || nppy != npp_rect.top)
	{
		SetWindowPos(nppw, HWND_NOTOPMOST, nppx, nppy, 0, 0, SWP_NOSIZE);
	}
	
#ifdef NPP_TOGGLE_TOPMOST
	if(!isWndTopmost(nppw))
	{
		SetWindowPos(nppw, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	else
	{
		npp_was_topmost = true;
	}
#endif
	
	if(~errs == 0)
	{
		goto error_marking_skip;
	}
	
	nppUpdateCurrFile();
	
	for(ui64 i = ~errs-1; i != UI64_MAX; --i)
	{
		if(errs[i].file != cur_file)
		{
			nppOpenFile(errs[i].file);
			cur_file = errs[i].file;
			nppMarkLine(errs[i].line, errs[i].type);
			continue;
		}
		
		if(errs[i].type == T_ERROR)
		{
			has_errors = true;
		}
		
		nppMarkLine(errs[i].line, errs[i].type);
	}
	
	nppGoToLine(errs[0].line, errs[0].pos_il);
	
	if(has_errors)
	{
		setFocusToNpp();
	}
	else
	{
		setFocusToMyself();
	}

error_marking_skip:
	WaitForSingleObject(pause_event, INFINITE);
	nppCleanup(CTRL_CLOSE_EVENT);
	VirtualFree(output, 0, MEM_RELEASE);
	
	p|RC|RM;
	return return_value;
}

void scroll(WPARAM dir)
{
	for(ui64 i = 0; i < SCROLL_LINES; ++i)
	{
		PostMessage(conw, WM_VSCROLL, dir, NULL);
	}
}

void nppMarkLine(ui64 line, ui64 type)
{
	LPARAM line_style = IDM_SEARCH_MARKONEEXT5;
	if(type == T_WARNING)
	{
		line_style = IDM_SEARCH_MARKONEEXT4;
	}
	
	// Find exact coordinates/dimentions of the line
	LRESULT line_pos = SendMessage(scintilla, SCI_POSITIONFROMLINE, (WPARAM)line-1, 0);
	LRESULT line_size = SendMessage(scintilla, SCI_LINELENGTH, (WPARAM)line-1, 0);
	
	// Select the line
	SendMessage(scintilla, SCI_SETSELECTIONSTART, (WPARAM)line_pos, 0);
	SendMessage(scintilla, SCI_SETSELECTIONEND, WPARAM(line_pos + line_size), 0);
	
	// Style the line with Notepad++'s 5th OR 4th Style
	SendMessage(nppw, NPPM_MENUCOMMAND, 0, line_style);
}

void nppGoToLine(ui64 line, ui64 pos_il)
{
	SendMessage(scintilla, SCI_ENSUREVISIBLE, (WPARAM)line-1, 0);
	SendMessage(scintilla, SCI_GOTOLINE, (WPARAM)line-1, 0);
	
	// Scroll window so that the line is in the middle of view
	LRESULT sl = SendMessage(scintilla, SCI_LINESONSCREEN, 0, 0);
	
	i64 scr_l = (i64)line - 1 - sl/2;
	i64 doc_vl = scr_l < 0 ? 0 : scr_l;
	
	i64 hidden_ls = 0;
	for(i64 i = (i64)line-1; i >= doc_vl; --i)
	{
		if(SendMessage(scintilla, SCI_GETLINEVISIBLE, (WPARAM)i, 0) == 0)
		{
			++hidden_ls;
		}
	}
	
	doc_vl -= hidden_ls;
	WPARAM first_doc_vl = doc_vl < 0 ? 0 : (WPARAM)doc_vl;
	WPARAM first_vis_vl = (WPARAM)SendMessage(scintilla, SCI_VISIBLEFROMDOCLINE, first_doc_vl, 0);
	
	SendMessage(scintilla, SCI_SETFIRSTVISIBLELINE, first_vis_vl, 0);
	LRESULT abs_pos = SendMessage(scintilla, SCI_POSITIONFROMLINE, (WPARAM)line-1, 0) + (LRESULT)pos_il-1;
	SendMessage(scintilla, SCI_SETSEL, (WPARAM)abs_pos, abs_pos);
}

void nppOpenFile(txt file)
{
	wtxt fname = t2u16(file);
	static COPYDATASTRUCT data;
	data.dwData = 0;
	data.cbData = ((DWORD)~fname + 1) * sizeof(wchar_t);
	data.lpData = PVOID((const wchar_t *)fname);
	
	SendMessage(plugin_wnd, WM_COPYDATA, 0, (LPARAM)&data);
}

void nppUpdateCurrFile()
{
	SendMessage(plugin_wnd, NPP_PLUGIN_GET_CUR_FILE_NAME, (WPARAM)msg_wnd, 0);
}

void searchErrorWeb(txt error)
{
	txt link = L("https://learn.microsoft.com/en-us/search/?terms=") + error;
	
	ShellExecuteA( // =============================================================================
	NULL,				//  [I|O]  Handle to parent window that will show error or UI messages
	"open",				//  [I|O]  Verb string -> open|edit|explore|find|open|print|runas|NULL(def)
	link,				//    [I]  File or object that the verb is beeing executed on
	NULL,				//  [I|O]  Cmd arguments to pass to the file, if it is exe file
	NULL,				//  [I|O]  Default working directory of the action NULL -> current dir
	SW_SHOWNORMAL);		//    [I]  Parameter that sets default nCmdShow status of the window
	// ============================================================================================
}

WORD invert(WORD ch_style)
{
	ui8 tmp = ui8(ch_style << 4);
	tmp |= (ch_style >> 4) & 0xF;
	return (ch_style & 0xFF00) | (DWORD)tmp;
}

struct TextParams
{
	COORD xy;
	ui32 s;
};

DWORD NOTHROW deselectText(LPVOID lp)
{
	text_selected = true;
	Sleep(150);
	TextParams tp = *((TextParams *)&lp); // A clever hack to decode struct sent by value
	static DWORD ret_val;
	WORD attr = 0;
	ReadConsoleOutputAttribute(oh, &attr, 1, tp.xy, &ret_val);
	FillConsoleOutputAttribute(oh, invert(attr), tp.s, tp.xy, &ret_val);
	text_selected = false;
	
	return 0;
}

void blinkText(SHORT x, SHORT y, ui32 s)
{
	if(text_selected)
	{
		return;
	}
	
	static DWORD ret_val;
	COORD xy;
	xy.X = x;
	xy.Y = y;
	
	WORD attr = 0;
	ReadConsoleOutputAttribute(oh, &attr, 1, xy, &ret_val);
	FillConsoleOutputAttribute(oh, invert(attr), s, xy, &ret_val);
	
	TextParams tp;
	tp.xy = xy;
	tp.s = s; // Size of text to blink
	LPVOID tp_void = *((LPVOID *)&tp); // A clever hack to send struct by value
	
	CreateThread(NULL, 0, deselectText, tp_void, 0, NULL);
}

void lmbProc(const MOUSE_EVENT_RECORD *m)
{
	COORD pos = m->dwMousePosition;
	nppUpdateCurrFile();
	
	for(ui64 i = 0; i < ~errs; ++i)
	{
		SHORT x0 = (SHORT)errs[i].x;
		SHORT x1 = (SHORT)errs[i].s + x0 - 1;
		SHORT y = (SHORT)errs[i].y;
		
		SHORT cx0 = SHORT(errs[i].x + errs[i].dist_to_code);
		SHORT cx1 = cx0 + CODE_SIZE - 1;
		
		if(pos.Y == y)
		{
			if(pos.X >= x0 && pos.X <= x1) // Line number clicked
			{
				blinkText(x0, y, errs[i].s);
				if(errs[i].file != cur_file)
				{
					nppOpenFile(errs[i].file);
				}
				nppGoToLine(errs[i].line, errs[i].pos_il);
				setFocusToNpp();
				break;
			}
			else if(pos.X >= cx0 && pos.X <= cx1) // Error/warning code clicked
			{
				blinkText(cx0, y, CODE_SIZE);
				searchErrorWeb(errs[i].code);
			}
		}
	}
}

#pragma warning( suppress : 4100 )
DWORD NOTHROW inputThread(LPVOID lp)
{
	INPUT_RECORD ir[128];
	DWORD nread;
	while(1)
	{
		ReadConsoleInput(ih, ir, 128, &nread);
		for(DWORD i = 0; i < nread; ++i)
		{
			if(ir[i].EventType == MOUSE_EVENT)
			{
				MOUSE_EVENT_RECORD *m = &ir[i].Event.MouseEvent;
				
				switch(m->dwEventFlags)
				{
				case 0:
					if(m->dwButtonState == 0x1)
					{
						//lmbProc(m);
					}
					//else if(m->dwButtonState == 0x2)
					//{
					//	
					//}
					break;
				case MOUSE_WHEELED:
					if((i32)m->dwButtonState > 0)
					{
						scroll(SB_LINEUP);
					}
					else
					{
						scroll(SB_LINEDOWN);
					}
					break;
				default:
					break;
				}
			}
			else if(ir[i].EventType == KEY_EVENT)
			{
				KEY_EVENT_RECORD *k = &ir[i].Event.KeyEvent;
				
				switch(k->wVirtualKeyCode)
				{
				case 0x20: // VK_SPACE
					return_value = 2; // For error_level 2 (warning)
					SetEvent(pause_event);
					break;
				default:
					SetEvent(pause_event);
					break;
				}
			}
		}
	}
	
	return 0;
}

BOOL NOTHROW nppCleanup(DWORD ctrl_sig)
{
	if(ctrl_sig != CTRL_CLOSE_EVENT)
	{
		return FALSE;
	}
	
	nppUpdateCurrFile();
	txt last_file = cur_file;
	
	for(ui64 i = 0; i < ~uniq_files; ++i)
	{
		if(uniq_files[i] != cur_file)
		{
			nppOpenFile(uniq_files[i]);
			cur_file = uniq_files[i];
			SendMessage(scintilla, SCI_SETEMPTYSELECTION, 0, 0);
		}
		
		SendMessage(nppw, NPPM_MENUCOMMAND, 0, IDM_SEARCH_UNMARKALLEXT5);
		SendMessage(nppw, NPPM_MENUCOMMAND, 0, IDM_SEARCH_UNMARKALLEXT4);
	}
	
	if(cur_file != last_file)
	{
		nppOpenFile(last_file);
	}
	
#ifdef NPP_TOGGLE_TOPMOST
	if(!npp_was_topmost)
	{
		AllowSetForegroundWindow(ASFW_ANY);
		
		DWORD cur_thread = GetCurrentThreadId();
		DWORD fg_pid = 0;
		DWORD fg_thread = GetWindowThreadProcessId(nppw, &fg_pid);
		
		AttachThreadInput(cur_thread, fg_thread, TRUE);
		SetFocus(nppw);
		SetForegroundWindow(nppw);
		AttachThreadInput(cur_thread, fg_thread, FALSE);
		
		SetWindowPos(nppw, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
#else
	setFocusToNpp();
#endif
	
	return FALSE; // Handlers are added in a list FILO. Def. first. Returning FALSE calls next in a list
}

#pragma warning( suppress : 4100 )
LRESULT CALLBACK msgWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	COPYDATASTRUCT *data = NULL;
	
	if(msg == WM_COPYDATA)
	{
		data = (COPYDATASTRUCT *)lp;
		
		if(data->dwData == 1)
		{
			wtxt cur_n = (ui64)data->cbData/2 + 1;
			cur_n = {(const wchar_t *)data->lpData, strl((const wchar_t *)data->lpData)};
			cur_file = wt2u8(cur_n);
			
			return TRUE;
		}
		
		txt dt = (ui64)data->cbData;
		dt = {(const char *)data->lpData, strl((const char *)data->lpData)};
		
		if(~uniq_files_no_ext == 0)
		{
			nppUpdateCurrFile();
			if(txtf(cur_file, 0, dt) != NFND)
			{
				SetEvent(pause_event);
			}
			
			return TRUE;
		}
		
		for(ui64 i = 0; i < ~uniq_files_no_ext; ++i)
		{
			if(uniq_files_no_ext[i] == dt)
			{
				SetEvent(pause_event);
				break;
			}
		}
		
		return TRUE;
	}
	
	return DefWindowProc(hwnd, msg, wp, lp);
}

void spawnMsgWnd()
{
	static WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = msgWndProc;
	wc.lpszClassName = msg_wnd_class;
	RegisterClassEx(&wc);
	msg_wnd = CreateWindowEx(0, msg_wnd_class, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
}

#pragma warning( suppress : 4100 )
DWORD NOTHROW msgProcThread(LPVOID lp)
{
	spawnMsgWnd();
	
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}
	
	return 0;
}

bool isWndTopmost(HWND wnd)
{
	DWORD exs = (DWORD)GetWindowLong(wnd, GWL_EXSTYLE);
	return exs & WS_EX_TOPMOST;
}

bool isLinNum(const txt &t, ui64 pos)
{
	for(ui64 i = pos+1; i < ~t; ++i)
	{
		if(t[i] == ')')
		{
			if(i+2 < ~t && t[i+1] == ':' || (t[i+1] == ' ' && t[i+2] == ':'))
			{
				return true;
			}
		}
		
		if(t[i] != ',' && (t[i] < '0' || t[i] > '9'))
		{
			return false;
		}
	}
	
	return false;
}

void setFocusToNpp()
{
	AllowSetForegroundWindow(ASFW_ANY);
	SetFocus(nppw);
	SetForegroundWindow(nppw);
}

void setFocusToMyself()
{
	//DWORD cur_thread = GetCurrentThreadId();
	//static DWORD fg_pid = 0;
	//DWORD fg_thread = GetWindowThreadProcessId(nppw, &fg_pid);
	
	//AttachThreadInput(cur_thread, fg_thread, TRUE);
	//AllowSetForegroundWindow(ASFW_ANY);
	SetFocus(conw);
	SetForegroundWindow(conw);
	//AttachThreadInput(cur_thread, fg_thread, FALSE);
}

SCROLLINFO si = { sizeof(SCROLLINFO), SIF_POS };

#pragma warning( suppress : 4100 )
DWORD NOTHROW mousePollThread(LPVOID lp)
{
	POINT mp, cr_lt, cr_rb; // Client rect left-top/right-bottom
	RECT cr;
	SHORT lmb;
	
	static bool lmb_pressed;
	static MOUSE_EVENT_RECORD mer;
	
	while(1)
	{
		Sleep(20);
		GetCursorPos(&mp);
		GetClientRect(conw, &cr);
		cr_lt.x = cr.left, cr_lt.y = cr.top;
		cr_rb.x = cr.right, cr_rb.y = cr.bottom;
		ClientToScreen(conw, &cr_lt);
		ClientToScreen(conw, &cr_rb);
		
		if(mp.x > cr_rb.x || mp.x < cr_lt.x || mp.y < cr_lt.y || mp.y > cr_rb.y)
		{
			continue;
		}
		
		lmb = GetAsyncKeyState(VK_LBUTTON);
		if(lmb != 0 && lmb & 0x8000) // Key down
		{
			if(!lmb_pressed)
			{
				GetScrollInfo(conw, SB_VERT, &si);
				ScreenToClient(conw, &mp);
				mer.dwMousePosition.X = (SHORT)(mp.x/(i64)con_cell_w);
				mer.dwMousePosition.Y = (SHORT)(mp.y/(i64)con_cell_h + si.nPos);
				lmbProc(&mer);
				lmb_pressed = true;
			}
		}
		else
		{
			lmb_pressed = false;
		}
	}
	
	return 0;
}