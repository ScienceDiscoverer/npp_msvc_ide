// DLIB
#include <utf>
#include <wtxta>
#include <ui64a>
#include <clip>

#include <npp.h>

#include <shlobj.h>
#include <shlwapi.h>

#define RESOURCE_FILE_INCLUDED
#include "sdpp.rc"

#pragma comment( lib, "user32" )
#pragma comment( lib, "gdi32" )
#pragma comment( lib, "shell32" )
#pragma comment( lib, "ole32" )

// TODO AppendMenuA function (winuser.h) https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-appendmenua THIS CAN INJECT CONTEXT MENU INTO NPP CONTEXT MENU! OR SPAWN A NEW MENU AND ADD ITEMS TO IT, ETC

// TODO REMOVE/DISABLE DEBUG PRINTS --> CLEAR STYLE 5 HIGHLIGHTS!

// TODO FINFILES BUGG: NOT REDRAWN WHEN MOVED OUT OF THE SCREEN
// TODO redrawMouseHov BUG!!! IF WINDOW IS SCROLLED DOWN, HIGHLIGHT IS BROKEN ::: FIXED ::: BUILD ::: TEST

// TODO Alternatively, call tips can be displayed when you leave the mouse pointer for a while over a word in response to the SCN_DWELLSTART notification andcancelled in response to SCN_DWELLEND.This method could be used in a debugger to give the value of a variable, or during editing to give information about the word under the pointer.
//SCN_DWELLSTART
//SCN_DWELLEND
//SCN_DWELLSTART is generated when the user keeps the mouse in one position for the dwell period (see SCI_SETMOUSEDWELLTIME).SCN_DWELLEND is generated after a SCN_DWELLSTART andthe mouse is moved or other activity such as key press indicates the dwell is over.Both notifications set the same fields in SCNotification :
//Field 	Usage
//position 	This is the nearest position in the document to the position where the mouse pointer was lingering.
//x, y 	Where the pointer lingered.The position field is set to SCI_POSITIONFROMPOINTCLOSE(x, y).

//SCI_CALLTIPSHOW(position pos, const char *definition)
//This message starts the process by displaying the call tip window.If a call tip is already active, this has no effect.
//pos is the position in the document at which to align the call tip.The call tip text is aligned to start 1 line below this character unless you have included up and/or down arrows in the call tip text in which case the tip is aligned to the right-hand edge of the rightmost arrow.The assumption is that you will start the text with something like "\001 1 of 3 \002".
//definition is the call tip text.This can contain multiple lines separated by '\n' (Line Feed, ASCII code 10) characters.Do not include '\r' (Carriage Return, ASCII code 13), as this will most likely print as an empty box. '\t' (Tab, ASCII code 9) is supported if you set a tabsize with SCI_CALLTIPUSESTYLE.
//The position of the caret is remembered here so that the call tip can be cancelled automatically if subsequent deletion moves the caret before this position.
//
//SCI_CALLTIPCANCEL
//This message cancels any displayed call tip.Scintilla will also cancel call tips for you if you use any keyboard commands that are not compatible with editing the argument list of a function.Call tips are cancelled if you delete back past the position where the caret was when the tip was triggered.
//
//SCI_CALLTIPACTIVE  bool64
//This returns 1 if a call tip is active and 0 if it is not active.
//
//SCI_CALLTIPPOSSTART  position
//SCI_CALLTIPSETPOSSTART(position posStart)
//This message returns or sets the value of the current position when SCI_CALLTIPSHOW started to display the tip.
//
//SCI_CALLTIPSETHLT(position highlightStart, position highlightEnd)
//This sets the region of the call tips text to display in a highlighted style.highlightStart is the zero-based index into the string of the first character to highlight and highlightEnd is the index of the first character after the highlight.highlightEnd must be greater than highlightStart; highlightEnd-highlightStart is the number of characters to highlight.Highlights can extend over line ends if this is required.
//
//Unhighlighted text is drawn in a mid grey.Selected text is drawn in a dark blue.The background is white.These can be changed with SCI_CALLTIPSETBACK, SCI_CALLTIPSETFORE, andSCI_CALLTIPSETFOREHLT.
//
//SCI_CALLTIPSETBACK(colour back)
//The background colour of call tips can be set with this message; the default colour is white.It is not a good idea to set a dark colour as the background as the default colour for normal calltip text is mid grey andthe default colour for highlighted text is dark blue.This also sets the background colour of STYLE_CALLTIP.
//
//SCI_CALLTIPSETFORE(colour fore)
//The colour of call tip text can be set with this message; the default colour is mid grey.This also sets the foreground colour of STYLE_CALLTIP.
//
//SCI_CALLTIPSETFOREHLT(colour fore)
//The colour of highlighted call tip text can be set with this message; the default colour is dark blue.
//
//SCI_CALLTIPUSESTYLE(int tabSize)
//This message changes the style used for call tips from STYLE_DEFAULT to STYLE_CALLTIP andsets a tab size in screen pixels.If tabsize is less than 1, Tab characters are not treated specially.Once this call has been used, the call tip foreground andbackground colours are also taken from the style.
//
//SCI_CALLTIPSETPOSITION(bool64 above)
//By default the calltip is displayed below the text, setting above to true (1) will display it above the text.

#define CMD_FUNCS 16

// Important commands' IDs
#define CMD_FIND_IN_FILES	6
//		SEPARATOR ---------	(CMD_CONSOLE + 7) -------------------------------
#define CMD_CONSOLE			(CMD_FIND_IN_FILES + 2)	// Cpp file parameters togglers
#define CMD_NOPT			(CMD_CONSOLE + 1)
#define CMD_DBG				(CMD_CONSOLE + 2)
#define CMD_NLAUNCH			(CMD_CONSOLE + 3)
#define CMD_ASM				(CMD_CONSOLE + 4)
//		SEPARATOR ---------	(CMD_CONSOLE + 5) -------------------------------
#define CMD_DBGP_DISABLED	(CMD_ASM + 2)			// Cpp file debug toggler

#define FIND_BEFORE	0x0
#define FIND_AFTER	0x1
#define CLOSING		0x0
#define OPENING		0x2

#define MAX_LANG_LINE_SIZE		512
#define MAX_COMMENT_TAG_SIZE	32

#define FILE_READ_BUFF_SZ		4096

#ifdef UNICODE
#define TOWSTR(x) L ## x
#else
#define TOWSTR(x) x
#endif

#define APP_GUID "-{AAA6D28F-DF14-4570-A273-21DD2708F5AD}"
#define REGISTER_MESSAGE(n) \
	static const UINT n = RegisterWindowMessage(TOWSTR(#n APP_GUID))

#undef MEM_FREE
#define MEM_ALLOC(x) HeapAlloc(heap, 0, (x))
#define MEM_ZALLOC(x) HeapAlloc(heap, HEAP_ZERO_MEMORY, (x))
#define MEM_FREE(x) HeapFree(heap, 0, (x))

HANDLE heap = HeapCreate(0, 0x10000, 0); // 65 536 B, 64k granularity




struct Selection
{
	ui64 beg;
	ui64 end;
	ui64 idx;
};

struct Lang
{
	const char *n;
	char c[MAX_COMMENT_TAG_SIZE];
	char bcb[MAX_COMMENT_TAG_SIZE];
	char bce[MAX_COMMENT_TAG_SIZE];
};

struct UserLang
{
	char *n;
	char c[MAX_COMMENT_TAG_SIZE];
	char bcb[MAX_COMMENT_TAG_SIZE];
	char bce[MAX_COMMENT_TAG_SIZE];
};

struct DebugPrintLine
{
	ui64 line;
	ui32 symb_n;
	ui32 pos;
	char **symbs;
	ui64 smb_ts;
};

ui64 ulangs_s;
UserLang *user_langs;

ui64 langs_s = 87;
Lang langs[] = {
	{"normal", ">>> ", "[", "]"} ,	// Coz, why not?
	{"php", "", "", ""} ,
	{"c", "", "", ""} ,
	{"cpp", "", "", ""} ,
	{"cs", "", "", ""} ,
	{"objc", "", "", ""} ,
	{"java", "", "", ""} ,
	{"rc", "", "", ""} ,
	{"html", "", "", ""} ,
	{"xml", "", "", ""} ,
	{"makefile", "", "", ""} ,
	{"pascal", "", "", ""} ,
	{"batch", "", "", ""} ,
	{"ini", "", "", ""} ,
	{"nfo", "", "", ""} ,			// MSDOS Style/ASCII Art
	{"L_USER", "", "", ""} ,		// User Defined language file
	{"asp", "", "", ""} ,
	{"sql", "", "", ""} ,
	{"vb", "", "", ""} ,
	{"L_JS", "", "", ""} ,			// Deprecated
	{"css", "", "", ""} ,
	{"perl", "", "", ""} ,
	{"python", "", "", ""} ,
	{"lua", "", "", ""} ,
	{"tex", "", "", ""} ,
	{"fortran", "", "", ""} ,
	{"bash", "", "", ""} ,
	{"actionscript", "", "", ""} ,	// Flash ActionScript file
	{"nsis", "", "", ""} ,
	{"tcl", "", "", ""} ,
	{"lisp", "", "", ""} ,
	{"scheme", "", "", ""} ,
	{"asm", "", "", ""} ,
	{"diff", "", "", ""} ,
	{"props", "", "", ""} ,
	{"postscript", "", "", ""} ,
	{"ruby", "", "", ""} ,
	{"smalltalk", "", "", ""} ,
	{"vhdl", "", "", ""} ,
	{"kix", "", "", ""} ,
	{"autoit", "", "", ""} ,
	{"caml", "", "", ""} ,
	{"ada", "", "", ""} ,
	{"verilog", "", "", ""} ,
	{"matlab", "", "", ""} ,
	{"haskell", "", "", ""} ,
	{"inno", "", "", ""} ,
	{"searchResult", "", "", ""} ,
	{"cmake", "", "", ""} ,
	{"yaml", "", "", ""} ,
	{"cobol", "", "", ""} ,
	{"gui4cli", "", "", ""} ,
	{"d", "", "", ""} ,
	{"powershell", "", "", ""} ,
	{"r", "", "", ""} ,
	{"jsp", "", "", ""} ,
	{"coffeescript", "", "", ""} ,
	{"json", "", "", ""} ,
	{"javascript", "", "", ""} ,
	{"fortran77", "", "", ""} ,
	{"baanc", "", "", ""} ,
	{"srec", "", "", ""} ,
	{"ihex", "", "", ""} ,
	{"tehex", "", "", ""} ,
	{"swift", "", "", ""} ,
	{"asn1", "", "", ""} ,
	{"avs", "", "", ""} ,
	{"blitzbasic", "", "", ""} ,
	{"purebasic", "", "", ""} ,
	{"freebasic", "", "", ""} ,
	{"csound", "", "", ""} ,
	{"erlang", "", "", ""} ,
	{"escript", "", "", ""} ,
	{"forth", "", "", ""} ,
	{"latex", "", "", ""} ,
	{"mmixal", "", "", ""} ,
	{"nim", "", "", ""} ,
	{"nncrontab", "", "", ""} ,
	{"oscript", "", "", ""} ,
	{"rebol", "", "", ""} ,
	{"registry", "", "", ""} ,
	{"rust", "", "", ""} ,
	{"spice", "", "", ""} ,
	{"txt2tags", "", "", ""} ,
	{"visualprolog", "", "", ""} ,
	{"typescript", "", "", ""} ,
	{"L_EXTERNAL", "", "", ""} ,	// End of language enum
};


const TCHAR *const npp_plugin_name = L"C++ Tools";

FuncItem funcs[CMD_FUNCS];
NppData npp_data;
HWND scintilla;

HINSTANCE md_inst;

bool64 dotless_thread_created;
unsigned long long draw_events;
bool64 dotless_found;
wchar_t nbuf[MAX_PATH];
unsigned long long nbuf_s;
SCNotification double_click_ntf;


//const TCHAR sectionName[] = TEXT("Insert Extension");
//const TCHAR keyName[] = TEXT("doCloseTag");
//const TCHAR configFileName[] = TEXT("pluginDemo.ini");

//TCHAR iniFilePath[MAX_PATH];
//bool64 doCloseTag = false;


const char *line_comm;
const char *block_beg;
const char *block_end;
ui64 line_comm_s;
ui64 block_beg_s;
ui64 block_end_s;

Selection *selects;

ShortcutKey toggle_comm_sh_key;

REGISTER_MESSAGE(NPP_PLUGIN_GET_SCINTILLA);
REGISTER_MESSAGE(NPP_PLUGIN_GET_CUR_FILE_NAME);
const wchar_t *plugin_msg_wnd_class = L"SdppNppPluginMsgWindow";
HWND msg_wnd;

const char *dbg_bpoint = "_DBG_SBP;";
const char *dbg_print = "_DBG_P(";
const char *dbg_prefix = "_DBG_";

bool64 cpp_console_set;
bool64 cpp_nopt_set;
bool64 cpp_dbg_set;
bool64 cpp_nlaunch_set;
bool64 cpp_asm_set;
bool64 cpp_dbg_bp_off_set;

const txt cpp_console = L(" CONSOLE");
const txt cpp_nopt = L(" NOPT");
const txt cpp_dbg = L(" DBG");
const txt cpp_nlaunch = L(" NLAUNCH");
const txt cpp_asm = L(" ASM");
const txt cpp_dbg_bp_off = L("#define _DBG_BP_OFF\r\n");
const txt cpp_sddb = L("#include <sddb>\r\n");
const txt cpp_define = L("#define");




void pluginInit(HANDLE md_inst);
void pluginCleanUp();
void commandMenuInit();
void commandMenuCleanUp();

bool64 setCommand(ui64 index, const wchar_t *cmd_name, PFUNCPLUGINCMD func_p, ShortcutKey *sk, bool64 check_on_init);

void openSmallCpp();
void toggleComment();
void toggleTabs();
void breakpoint();
void debugPrint();
void findInFiles();
void toggleCppConsole();
void toggleCppNopt();
void toggleCppDbg();
void toggleCppNlaunch();
void toggleCppAsm();
void toggleCppDisableDbgp();
void cppRemoveDbgp();
void dupeFile();

void updateCppParams();

void addToolbarButton(ui64 comm_id, ui64 ico_id);
DWORD NOTHROW dotlessStyleThread(LPVOID lp);

char *strrep(char *s, ui64 pos, ui64 n, char rep);
bool64 strCmpRng(const char *s0, const char *s1_beg, const char *s1_end);
bool64 fndValRng(const char *fnd, const char *start_range, const char **beg, const char **end);
bool64 fndValRngUser(const char *fnd, const char *start_range, const char **beg, const char **end);
void strCpyRng(char *dest, const char *s_beg, const char *s_end);
void htmlEntiti2chars(char *ent_s);
void extractUserLangInfo(const wchar_t *fn);
void extractULIdir(wchar_t *dir);

DWORD NOTHROW commDbLoadingThread(LPVOID lp);

ui64 fnd1stNonSp(ui64 line);
ui64 fnd1stNonSp(ui64 line, ui64 *h_sp);
bool64 isComment(ui64 pos);
bool64 isLineBeg(ui64 pos);
bool64 isLineEnd(ui64 pos);
bool64 isPosBegLine(ui64 pos, ui64 line);
bool64 isBlockComm(ui64 pos, ui64 mode);
void insertTabs(ui64 amount, ui64 pos);
bool64 fndInSpaceSepArr(const char *fnd, const char *arr);
void wcs2str(char *dest, const wchar_t *src);

i64 doToggleComment(ui64 sel_s, ui64 sel_e, ui64 sel_i);
i64 togSingSelLineComm(ui64 sel_s, ui64 sel_e, ui64 sel_i);
i64 togMultiSelLineComm(ui64 sel_s, ui64 sel_e, ui64 sel_i);
i64 togBlockComm(ui64 sel_s, ui64 sel_e, ui64 sel_i);

void spawnMsgWnd();
LRESULT CALLBACK msgWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
DWORD NOTHROW msgProcThread(LPVOID lp);

bool64 isSymbolChar(char c);
char *getSymbol(ui64 pos);
char *getSelection(ui64 beg, ui64 end);

ui64 countNextSymbs(ui64 *pos, const txt &t);
ui64 getSymbBeg(ui64 pos, const txt &t);
ui64 getSymbEnd(ui64 pos, const txt &t);
bool64 isFuncDecl(const txt &t, ui64 *symb_pos);
void nppGoToLine(ui64 line);
void nppGoToPos(ui64 pos);
bool64 checkTxt(char *src, const txt &t);
bool64 checkTxtHasType(char *sbeg, bool64 in_class);
bool64 semicolInLine(const char *ln);
ui64 findFuncDefin(char *doc, ui64 bpos, const txt &symb, const txt &args);
ui64 findFuncDecl(char *doc, ui64 epos, const txt &symb, const txt &args);
bool64 fileExists(const wtxt &fn);
void findFuncInOtherFile(const txt &symb, const txt &args);
bool64 hasType(const txt &line, ui64 sbeg, ui64 lpos_abs);
txt findClassName(char *doc, ui64 sbeg);
DWORD __declspec(nothrow) functionJumper(void *scn);

void toggleCppParam(bool64 &cpp_param_set, ui64 cmd_func, const txt &cmd_param_txt);

txt getLine(ui64 line_i);
txt getTextRange(ui64 b, ui64 e);
void includeSddb();
void removeDbgPrints(ui64 s, ui64 e);

void doFindInFiles(const txt &pat);




inline ui64 m2scintilla(UINT id, ui64 wp, ui64 lp)
{
	return (ui64)SendMessage(scintilla, id, (WPARAM)wp, (LPARAM)lp);
}

inline ui64 m2scintilla(UINT id, ui64 wp, txt &lp)
{
	return (ui64)SendMessage(scintilla, id, (WPARAM)wp, (LPARAM)(char *)lp);;
}

inline ui64 m2scintilla(UINT id, ui64 wp, const txt &lp)
{
	return (ui64)SendMessage(scintilla, id, (WPARAM)wp, (LPARAM)(const char *)lp);
}

inline ui64 m2scintilla(UINT id, ui64 wp)
{
	return (ui64)SendMessage(scintilla, id, (WPARAM)wp, 0);
}

inline ui64 m2scintilla(UINT id)
{
	return (ui64)SendMessage(scintilla, id, 0, 0);
}

inline ui64 m2npp(UINT id, ui64 wp, ui64 lp)
{
	return (ui64)SendMessage(npp_data._nppHandle, id, (WPARAM)wp, (LPARAM)lp);
}

inline ui64 m2npp(UINT id, ui64 wp, void *lp)
{
	return (ui64)SendMessage(npp_data._nppHandle, id, (WPARAM)wp, (LPARAM)lp);
}

inline ui64 m2npp(UINT id, ui64 wp, const void *lp)
{
	return (ui64)SendMessage(npp_data._nppHandle, id, (WPARAM)wp, (LPARAM)lp);
}

inline ui64 m2npp(UINT id, ui64 wp)
{
	return (ui64)SendMessage(npp_data._nppHandle, id, (WPARAM)wp, 0);
}

inline ui64 m2npp(UINT id)
{
	return (ui64)SendMessage(npp_data._nppHandle, id, 0, 0);
}

inline void updateScintillaHwnd() // Update the current scintilla
{
	int scintilla_used = -1;
	SendMessage(npp_data._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&scintilla_used);
	if(scintilla_used == -1) // Scintilla 0 -> Main View in multivew (Left) | 1 -> Second view (Right)
	{
		return;
	}
	scintilla = (scintilla_used == 0) ? npp_data._scintillaMainHandle : npp_data._scintillaSecondHandle;
}

BOOL APIENTRY DllMain(HANDLE hm, DWORD reason, LPVOID /*lpReserved*/)
{
	switch(reason)
	{
	case DLL_PROCESS_ATTACH:
		pluginInit(hm);
		md_inst = (HINSTANCE)hm;
		break;
	case DLL_PROCESS_DETACH:
		pluginCleanUp();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
	}

    return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notepad_plus_data)
{
	npp_data = notepad_plus_data;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return npp_plugin_name;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *fn)
{
	*fn = CMD_FUNCS;
	return funcs;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *scn)
{
	switch (scn->nmhdr.code)
	{
	// NPP NOTIFICATIONS ==========================================
	case NPPN_READY:
	case NPPN_WORDSTYLESUPDATED: // User changed current theme's style
		SendMessage(npp_data._scintillaMainHandle, SCI_CALLTIPSETBACK, 0xFF1E1E1E, 0);
		SendMessage(npp_data._scintillaMainHandle, SCI_CALLTIPSETFORE, 0xFFF3F2F1, 0);
		SendMessage(npp_data._scintillaMainHandle, SCI_CALLTIPSETFOREHLT, 0xFFD69C55, 0);
		SendMessage(npp_data._scintillaSecondHandle, SCI_CALLTIPSETBACK, 0xFF1E1E1E, 0);
		SendMessage(npp_data._scintillaSecondHandle, SCI_CALLTIPSETFORE, 0xFFF3F2F1, 0);
		SendMessage(npp_data._scintillaSecondHandle, SCI_CALLTIPSETFOREHLT, 0xFFD69C55, 0);
		break;
	case NPPN_BUFFERACTIVATED:
		updateCppParams();
		break;
	case NPPN_FILESAVED:
	case NPPN_FILEBEFOREOPEN:
	{
		ui64 res = m2npp(NPPM_GETFULLPATHFROMBUFFERID, scn->nmhdr.idFrom);

		wchar_t *full_path = (wchar_t *)MEM_ALLOC((res+1) * sizeof(wchar_t));
		
		res = m2npp(NPPM_GETFULLPATHFROMBUFFERID, scn->nmhdr.idFrom, full_path);

		bool64 dot_found = false;
		unsigned long long slash = 0;
		for(ui64 i = res-1; i != UI64_MAX; --i)
		{
			if(full_path[i] == '\\')
			{
				slash = i;
				break;
			}
			else if(full_path[i] == '.')
			{
				dot_found = true;
				break;
			}
		}

		if(!dot_found)
		{
			draw_events = 0;
			if(!dotless_thread_created)
			{
				CreateThread(NULL, 0, dotlessStyleThread, NULL, NULL, NULL);
				dotless_thread_created = true;
			}
			
			dotless_found = true;

			ui64 i = slash+1, j = 0;
			for(; i < res; ++i, ++j)
			{
				nbuf[j] = full_path[i];
			}
			nbuf[j] = 0;
			nbuf_s = j;
		}

		MEM_FREE(full_path);
	}	break;
	case NPPN_TBMODIFICATION:
	{
		addToolbarButton(0, IDI_SMALLDOT_ICON);
		addToolbarButton(1, IDI_COMMENT_ICON);
		addToolbarButton(2, IDI_TABS_ICON);
		addToolbarButton(3, IDI_DEBUG_BP_ICON);
		addToolbarButton(4, IDI_DEBUG_DP_ICON);
		addToolbarButton(5, IDI_DUPE_FILE_ICON);
		// ID[6+] no buttons, context menu only or separators
	}	break;
	case NPPN_SHUTDOWN:
		commandMenuCleanUp();
		break;
	// SCINTILLA NOTIFICATIONS ====================================
	case SCN_ZOOM: // TODO ADD TOGGLE MENU COMMAND FOR THIS ~~~~~~~~~~~~~~~~~~~~
	{
		updateScintillaHwnd();
		//i32 zoom = (i32)m2scintilla(SCI_GETZOOM);
		m2scintilla(SCI_SETZOOM, 0);
	}	return;
	case SCN_PAINTED:
		if(dotless_found)
		{
			++draw_events;
		}
		break;
	case SCN_DOUBLECLICK:
		double_click_ntf.position = scn->position;
		double_click_ntf.line = scn->line;
		CreateThread(NULL, 0, functionJumper, (void *)&double_click_ntf, 0, NULL);
		break;
	default:
		return;
	}
}

#pragma warning( suppress : 4100 )
extern "C" __declspec(dllexport) LRESULT messageProc(UINT msg, WPARAM wp, LPARAM lp)
{
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif // UNICODE




#pragma warning( suppress : 4100 )
void pluginInit(HANDLE module_inst)
{
	CreateThread(NULL, 0, commDbLoadingThread, NULL, 0, NULL);
	CreateThread(NULL, 0, msgProcThread, NULL, 0, NULL);
}

void pluginCleanUp()
{
	for(ui64 i = 0; i < ulangs_s; ++i)
	{
		MEM_FREE(user_langs[i].n);
	}

	if(user_langs != NULL)
	{
		MEM_FREE(user_langs);
	}

	// Settings saving example below...
	//::WritePrivateProfileString(sectionName, keyName, doCloseTag?TEXT("1"):TEXT("0"), iniFilePath);
}

void commandMenuInit()
{
	toggle_comm_sh_key._isAlt = false;
	toggle_comm_sh_key._isCtrl = true;
	toggle_comm_sh_key._isShift = false;
	toggle_comm_sh_key._key = 0x51; // VK_Q

	ui64 fn = 0;
	setCommand(fn, L"Open s.cpp", openSmallCpp, NULL, false);
	setCommand(++fn, L"Toggle comment", toggleComment, &toggle_comm_sh_key, false);
	setCommand(++fn, L"Hide/Show Tabs", toggleTabs, NULL, false);
	setCommand(++fn, L"Put Breakpoint", breakpoint, NULL, false);
	setCommand(++fn, L"Make Debug Print", debugPrint, NULL, false);
	setCommand(++fn, L"Duplicate File", dupeFile, NULL, false);
	setCommand(++fn, L"Find in Files", findInFiles, NULL, false);
	setCommand(++fn, L"---", NULL, NULL, false); //-----------------------------------
	setCommand(++fn, L"Console", toggleCppConsole, NULL, false); // 8
	setCommand(++fn, L"No optim.", toggleCppNopt, NULL, false);
	setCommand(++fn, L"Debug", toggleCppDbg, NULL, false);
	setCommand(++fn, L"No launch", toggleCppNlaunch, NULL, false);
	setCommand(++fn, L"ASM", toggleCppAsm, NULL, true);
	setCommand(++fn, L"---", NULL, NULL, false); //-----------------------------------
	setCommand(++fn, L"Disable DBGP", toggleCppDisableDbgp, NULL, false);
	setCommand(++fn, L"Remove DBGP", cppRemoveDbgp, NULL, false);
}

bool64 setCommand(ui64 index, const wchar_t *cmd_name, PFUNCPLUGINCMD func_p, ShortcutKey *sk, bool64 check_on_init)
{
	wcscpy(funcs[index]._itemName, cmd_name);
	funcs[index]._pFunc = func_p;
	funcs[index]._init2Check = (bool)check_on_init;
	funcs[index]._pShKey = sk;
	return true;
}

void commandMenuCleanUp()
{
	// Do nothing...
}

//----------------------------------------------//
//		       FUNCTION DEFINITIONS				//
//----------------------------------------------//

void openSmallCpp()
{
	m2npp(NPPM_DOOPEN, 0, L"D:\\P\\MT\\s.cpp");
}

void toggleTabs()
{
	bool64 tabs_hidden = (bool64)m2npp(NPPM_ISTABBARHIDDEN);
	m2npp(NPPM_HIDETABBAR, 0, !tabs_hidden);
}

void toggleComment()
{
	updateScintillaHwnd();

	ui32 lang_t = L_TEXT;
	m2npp(NPPM_GETCURRENTLANGTYPE, 0, &lang_t);

	if(lang_t == L_USER)
	{
		static wchar_t fname[MAX_PATH];
		m2npp(NPPM_GETFILENAME, MAX_PATH, fname);

		wchar_t *ext = wcsrchr(fname, '.') + 1;
		if(ext == nullptr)
		{
			return;
		}

		static char ext_str[MAX_PATH];
		wcs2str(ext_str, ext);

		bool64 ulang_not_found = true;
		for(ui64 i = 0; i < ulangs_s; ++i)
		{
			if(fndInSpaceSepArr(ext_str, user_langs[i].n))
			{
				line_comm = user_langs[i].c;
				block_beg = user_langs[i].bcb;
				block_end = user_langs[i].bce;
				ulang_not_found = false;
				break;
			}
		}

		if(ulang_not_found)
		{
			return;
		}
	}
	else
	{
		line_comm = langs[lang_t].c;
		block_beg = langs[lang_t].bcb;
		block_end = langs[lang_t].bce;
	}

	line_comm_s = strlen(line_comm);
	block_beg_s = strlen(block_beg);
	block_end_s = strlen(block_end);

	ui64 sel_n = m2scintilla(SCI_GETSELECTIONS); // There is always at least one selection

	selects = (Selection *)MEM_ALLOC(sel_n * sizeof(Selection));

	for(ui64 i = 0; i < sel_n; ++i)
	{
		selects[i].beg = m2scintilla(SCI_GETSELECTIONNSTART, i);
		selects[i].end = m2scintilla(SCI_GETSELECTIONNEND, i);
		selects[i].idx = i;
	}

	for(ui64 i = 0; i < sel_n; ++i) // Insertion sort
	{
		for(ui64 j = i+1; j < sel_n; ++j)
		{
			if(selects[j].beg < selects[i].beg)
			{
				Selection tmp = selects[i];
				selects[i] = selects[j];
				selects[j] = tmp;
			}
		}
	}

	m2scintilla(SCI_BEGINUNDOACTION); // UNDO =========================

	i64 ch_ins = 0;
	for(ui64 i = 0; i < sel_n; ++i)
	{
		ui64 ss = selects[i].beg + ch_ins;
		ui64 se = selects[i].end + ch_ins;

		ch_ins += doToggleComment(ss, se, i);
	}

	if(selects[0].end < selects[0].beg)
	{
		ui64 tmp = selects[0].end;
		selects[0].end = selects[0].beg;
		selects[0].beg = tmp;
	}

	m2scintilla(SCI_SETSELECTION, selects[0].end, selects[0].beg); // Good for 1st selection

	for(ui64 i = 1; i < sel_n; ++i)
	{
		if(selects[i].end < selects[i].beg)
		{
			ui64 tmp = selects[i].end;
			selects[i].end = selects[i].beg;
			selects[i].beg = tmp;
		}

		m2scintilla(SCI_ADDSELECTION, selects[i].end, selects[i].beg);
	}

	m2scintilla(SCI_ENDUNDOACTION);   // ==============================

	MEM_FREE(selects);
}

void breakpoint()
{
	updateScintillaHwnd();

	ui64 sel_n = m2scintilla(SCI_GETSELECTIONS); // There is always at least one selection
	static ui64 bp_txt_size = strlen(dbg_bpoint);

	ui64 *lines = (ui64 *)MEM_ALLOC(sel_n * sizeof(ui64));
	for(ui64 i = 0; i < sel_n; ++i)
	{
		ui64 pos = m2scintilla(SCI_GETSELECTIONNEND, i);
		lines[i] = m2scintilla(SCI_LINEFROMPOSITION, pos);
	}

	m2scintilla(SCI_BEGINUNDOACTION); // UNDO =========================

	for(ui64 i = 0; i < sel_n; ++i)
	{
		ui64 ins_pos = m2scintilla(SCI_GETLINEENDPOSITION, lines[i]);
		m2scintilla(SCI_INSERTTEXT, ins_pos, (ui64)dbg_bpoint);
	}

	includeSddb();

	m2scintilla(SCI_ENDUNDOACTION);   // ==============================
}

void debugPrint()
{
	updateScintillaHwnd();

	ui64 sel_n = m2scintilla(SCI_GETSELECTIONS); // There is always at least one selection
	static char to_ins[MAX_PATH];
	static ui64 dp_txt_size = strlen(dbg_print);

	if(to_ins[0] == 0)
	{
		memcpy(to_ins, dbg_print, dp_txt_size+1);
	}

	Selection *sels = (Selection *)MEM_ALLOC(sel_n * sizeof(Selection));
	for(ui64 i = 0; i < sel_n; ++i)
	{
		sels[i].beg = m2scintilla(SCI_GETSELECTIONNSTART, i);
		sels[i].end = m2scintilla(SCI_GETSELECTIONNEND, i);
		//sels[i].idx = i;
	}

	for(ui64 i = 0; i < sel_n; ++i) // Insertion sort
	{
		for(ui64 j = i+1; j < sel_n; ++j)
		{
			if(sels[j].end < sels[i].end)
			{
				Selection tmp = sels[i];
				sels[i] = sels[j];
				sels[j] = tmp;
			}
		}
	}

	DebugPrintLine *dbg_lns = (DebugPrintLine *)MEM_ZALLOC(sel_n * sizeof(DebugPrintLine));

	ui64 dbl_s = UI64_MAX; // +1 will give 0
	ui64 prev_line = UI64_MAX;
	for(ui64 i = 0; i < sel_n; ++i)
	{
		ui64 line = m2scintilla(SCI_LINEFROMPOSITION, sels[i].end);

		if(line != prev_line)
		{
			prev_line = line;
			dbg_lns[++dbl_s].line = line;
		}

		if(dbg_lns[dbl_s].symb_n == dbg_lns[dbl_s].smb_ts)
		{
			dbg_lns[dbl_s].smb_ts += 8;
			char **tmp = (char **)MEM_ZALLOC(dbg_lns[dbl_s].smb_ts * sizeof(char **));
			memcpy(tmp, dbg_lns[dbl_s].symbs, dbg_lns[dbl_s].symb_n * sizeof(char **));
			MEM_FREE(dbg_lns[dbl_s].symbs);
			dbg_lns[dbl_s].symbs = tmp;
		}

		if(sels[i].end == sels[i].beg) // Single caret, no selection
		{
			dbg_lns[dbl_s].symbs[dbg_lns[dbl_s].symb_n] = getSymbol(sels[i].end);
		}
		else // Multiple characters selected
		{
			dbg_lns[dbl_s].symbs[dbg_lns[dbl_s].symb_n] = getSelection(sels[i].beg, sels[i].end);
		}

		dbg_lns[dbl_s].symb_n += 1;
	}

	++dbl_s; // Change dbl_s from index to actual size

	m2scintilla(SCI_BEGINUNDOACTION); // UNDO =========================

	ui64 ins_pos = m2scintilla(SCI_GETLINEENDPOSITION, dbg_lns[0].line);

	for(ui64 i = 0; i < dbl_s; ++i)
	{
		for(ui64 j = 0; j < dbg_lns[i].symb_n; ++j)
		{
			ins_pos = m2scintilla(SCI_GETLINEENDPOSITION, dbg_lns[i].line);

			ui64 ss = strlen(dbg_lns[i].symbs[j]);

			if(ss + dp_txt_size + 2 >= MAX_PATH)
			{
				m2scintilla(SCI_INSERTTEXT, ins_pos, (ui64)"[!!!SYMBOL_>_MAX_PATH!!!]");
				continue;
			}

			strcpy(to_ins + dp_txt_size, dbg_lns[i].symbs[j]);
			ui64 tot_s = dp_txt_size + ss + 2;
			to_ins[tot_s-2] = ')';
			to_ins[tot_s-1] = ';';
			to_ins[tot_s] = 0;

			m2scintilla(SCI_INSERTTEXT, ins_pos, (ui64)to_ins);
		}
	}

	ins_pos = m2scintilla(SCI_GETLINEENDPOSITION, dbg_lns[dbl_s-1].line);

	includeSddb();

	m2scintilla(SCI_ENDUNDOACTION);   // ==============================

	for(ui64 i = 0; i < dbl_s; ++i)
	{
		for(ui64 j = 0; j < dbg_lns[i].symb_n; ++j)
		{
			MEM_FREE(dbg_lns[i].symbs[j]);
		}

		MEM_FREE(dbg_lns[i].symbs);
	}

	MEM_FREE(sels);
	MEM_FREE(dbg_lns);
}

void findInFiles()
{
	updateScintillaHwnd();
	ui64 ss = m2scintilla(SCI_GETSELECTIONSTART);
	ui64 se = m2scintilla(SCI_GETSELECTIONEND);

	if(ss == se) // No selection
	{
		if(!isSymbolChar((char)m2scintilla(SCI_GETCHARAT, ss)))
		{
			return;
		}
		
		ui64 lnum = m2scintilla(SCI_LINEFROMPOSITION, ss);
		ui64 lbeg = m2scintilla(SCI_POSITIONFROMLINE, lnum);
		txt line = getLine(lnum);
		ui64 pos_il = ss - lbeg; // Posiiton in line
		txt symb = txtsp(line, getSymbBeg(pos_il, line), getSymbEnd(pos_il, line));

		doFindInFiles(symb);
		return;
	}

	txt pattern = getTextRange(ss, se);
	doFindInFiles(pattern);
}

void toggleCppConsole()
{
	toggleCppParam(cpp_console_set, CMD_CONSOLE, cpp_console);
}

void toggleCppNopt()
{
	toggleCppParam(cpp_nopt_set, CMD_NOPT, cpp_nopt);
}

void toggleCppDbg()
{
	toggleCppParam(cpp_dbg_set, CMD_DBG, cpp_dbg);
}

void toggleCppNlaunch()
{
	toggleCppParam(cpp_nlaunch_set, CMD_NLAUNCH, cpp_nlaunch);
}

void toggleCppAsm()
{
	toggleCppParam(cpp_asm_set, CMD_ASM, cpp_asm);
}

namespace toggleCppDisableDbgpFunc
{
	static txt line;
}
void toggleCppDisableDbgp()
{
	using namespace toggleCppDisableDbgpFunc;
	updateScintillaHwnd();
	if((char)m2scintilla(SCI_GETCHARAT, 0) != '/')
	{
		return;
	}

	bool64 dbg_bp_off_set = false;
	ui64 i = 1, bp_off_line = 1;
	do
	{
		line = getLine(i);
		if(line == cpp_dbg_bp_off)
		{
			dbg_bp_off_set = true;
			bp_off_line = i;
		}
		++i;
	}
	while(txtfr(line, 0, 6, cpp_define) != NFND);

	cpp_dbg_bp_off_set = !dbg_bp_off_set;
	if(cpp_dbg_bp_off_set && line != cpp_sddb)
	{
		return;
	}

	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_DBGP_DISABLED]._cmdID, (ui64)cpp_dbg_bp_off_set);

	ui64 line_beg = m2scintilla(SCI_POSITIONFROMLINE, i-1);
	if(cpp_dbg_bp_off_set)
	{
		m2scintilla(SCI_INSERTTEXT, line_beg, cpp_dbg_bp_off);
		return;
	}

	line_beg = m2scintilla(SCI_POSITIONFROMLINE, bp_off_line);
	m2scintilla(SCI_DELETERANGE, line_beg, ~cpp_dbg_bp_off);
}

namespace cppRemoveDbgFunc
{
	static txt line;
}
void cppRemoveDbgp()
{
	using namespace cppRemoveDbgFunc;
	updateScintillaHwnd();
	if((char)m2scintilla(SCI_GETCHARAT, 0) != '/')
	{
		return;
	}

	ui64 sel_s = m2scintilla(SCI_GETSELECTIONSTART);
	ui64 sel_e = m2scintilla(SCI_GETSELECTIONEND);

	if(sel_s != sel_e) // Active selection
	{
		removeDbgPrints(sel_s, sel_e);
		return;
	}

	bool64 dbg_bp_off_set = false;
	ui64 i = 1, bp_off_line = 1;
	do
	{
		line = getLine(i);
		if(line == cpp_dbg_bp_off)
		{
			dbg_bp_off_set = true;
			bp_off_line = i;
		}
		++i;
	} while(txtfr(line, 0, 6, cpp_define) != NFND);

	if(line == cpp_sddb)
	{
		ui64 del_beg = m2scintilla(SCI_POSITIONFROMLINE, i-1);
		m2scintilla(SCI_DELETERANGE, del_beg, ~cpp_sddb);
	}

	if(dbg_bp_off_set)
	{
		ui64 del_beg = m2scintilla(SCI_POSITIONFROMLINE, bp_off_line);
		m2scintilla(SCI_DELETERANGE, del_beg, ~cpp_dbg_bp_off);
	}

	cpp_dbg_bp_off_set = false;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_DBGP_DISABLED]._cmdID, cpp_dbg_bp_off_set);

	removeDbgPrints(0, m2scintilla(SCI_GETTEXTLENGTH));
}

void dupeFile()
{
	updateScintillaHwnd();
	char *doc = (char *)m2scintilla(SCI_GETCHARACTERPOINTER);
	m2npp(NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
	updateScintillaHwnd();
	m2scintilla(SCI_SETTEXT, 0, (ui64)doc);
}




void addToolbarButton(ui64 comm_id, ui64 ico_id)
{
	toolbarIconsWithDarkMode tbIcons; // Sending NPPM_SETMENUITEMCHECK for menu entry will also toggle button state
	tbIcons.hToolbarIcon = ::LoadIcon(md_inst, MAKEINTRESOURCE(ico_id));
	tbIcons.hToolbarIconDarkMode = ::LoadIcon(md_inst, MAKEINTRESOURCE(ico_id+1));
	tbIcons.hToolbarBmp = ::LoadBitmap(md_inst, MAKEINTRESOURCE(ico_id+2));
	m2npp(NPPM_ADDTOOLBARICON_FORDARKMODE, (ui64)funcs[comm_id]._cmdID, (ui64)&tbIcons);
}

#pragma warning( suppress : 4100 )
DWORD __declspec(nothrow) dotlessStyleThread(LPVOID lp)
{
	unsigned long long prev_draw_events = draw_events;
	Sleep(10);
	while(draw_events - prev_draw_events != 0)
	{
		//MessageBox(NULL, L"loope", L"dotto", MB_ABORTRETRYIGNORE);
		prev_draw_events = draw_events;
		Sleep(10);
	}

	static wchar_t curn[MAX_PATH];
	m2npp(NPPM_GETFILENAME, MAX_PATH, (ui64)curn);

	if(wcscmp(nbuf, curn) == 0)
	{
		m2npp(NPPM_MENUCOMMAND, 0, IDM_LANG_CPP);
	}

	dotless_found = false;
	dotless_thread_created = false;
	return 0;
}




char * strrep(char *s, ui64 pos, i64 n, char rep) // String Replace with Char
{
	ui64 sz = strlen(s);
	ui64 ns = sz - (ui64)n + 1;

	char *ipos = s + pos;
	memcpy(ipos + 1, s + pos + (ui64)n, sz - pos - (ui64)n);
	*ipos = rep;
	s[ns] = 0;

	return s;
}

bool64 strCmpRng(const char *s0, const char *s1_beg, const char *s1_end)
{
	++s1_end;
	while(*s0 != 0 && s1_beg != s1_end)
	{
		if(*s0 != *s1_beg)
		{
			return false;
		}
		++s0;
		++s1_beg;
	}

	return *s0 == 0 && s1_beg == s1_end;
}

bool64 fndValRng(const char *fnd, const char *start_range, const char **beg, const char **end)
{
	ui64 s = strlen(fnd);
	const char *val_tag = strstr(start_range, fnd);
	if(val_tag == nullptr)
	{
		return false;
	}

	*beg = val_tag + s;
	*end = strchr(*beg, '"') - 1;

	if(*end == val_tag + s - 1) // Tag is empty
	{
		return false;
	}

	return true;
}

bool64 fndValRngUser(const char *fnd, const char *start_range, const char **beg, const char **end)
{
	ui64 s = strlen(fnd);
	const char *val_tag = strstr(start_range, fnd);
	if(val_tag == nullptr)
	{
		return false;
	}

	*beg = val_tag + s;
	*end = strpbrk(*beg, " <") - 1;

	if(*end == val_tag + s - 1) // Tag is empty
	{
		return false;
	}

	return true;
}

void strCpyRng(char *dest, const char *s_beg, const char *s_end)
{
	ui64 s = (ui64)(s_end - s_beg + 1);
	strncpy(dest, s_beg, s);
	dest[s] = 0;
}

void htmlEntiti2chars(char *ent_s)
{
	--ent_s;
	while(*(++ent_s) != 0)
	{
		if(*ent_s == '&')
		{
			char *ent_end = strchr(ent_s, ';');

			if(strCmpRng("&amp;", ent_s, ent_end) || strCmpRng("&#38;", ent_s, ent_end))
			{
				strrep(ent_s, 0, ent_end - ent_s + 1, '&');
			}
			if(strCmpRng("&apos;", ent_s, ent_end) || strCmpRng("&#39;", ent_s, ent_end))
			{
				strrep(ent_s, 0, ent_end - ent_s + 1, '\'');
			}
			if(strCmpRng("&quot;", ent_s, ent_end) || strCmpRng("&#34;", ent_s, ent_end))
			{
				strrep(ent_s, 0, ent_end - ent_s + 1, '"');
			}
			if(strCmpRng("&lt;", ent_s, ent_end) || strCmpRng("&#60;", ent_s, ent_end))
			{
				strrep(ent_s, 0, ent_end - ent_s + 1, '<');
			}
			if(strCmpRng("&gt;", ent_s, ent_end) || strCmpRng("&#62;", ent_s, ent_end))
			{
				strrep(ent_s, 0, ent_end - ent_s + 1, '>');
			}
			if(strCmpRng("&nbsp;", ent_s, ent_end) || strCmpRng("&#160;", ent_s, ent_end))
			{
				strrep(ent_s, 0, ent_end - ent_s + 1, ' ');
			}
		}
	}
}

void extractUserLangInfo(const wchar_t *fn)
{
	// Create or open File or Device =================================================================
	HANDLE f = CreateFile(
		fn,							//   [I]  Name of the file or device to create/open
		GENERIC_READ,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_READ,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_EXISTING,				//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,	//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================

	if(f == INVALID_HANDLE_VALUE)
	{

		return;
	}

	char line[MAX_LANG_LINE_SIZE];
	line[MAX_LANG_LINE_SIZE-1] = 0;
	ui64 li = 0;
	bool64 ignore_cur_line = false;
	DWORD br;
	char ch = 0;

	while(ReadFile(f, &ch, 1, &br, NULL) && br)
	{
		if(ignore_cur_line)
		{
			if(ch != '\n')
			{
				continue;
			}
			ignore_cur_line = false;
			li = 0;
			continue;
		}

		if(ch == '\n')
		{
			line[li] = '\n';
			line[li+1] = 0;
			li = 0;

			char *lang_p = strstr(line, "<UserLang ");
			if(lang_p == nullptr)
			{
				char *comm_p = strstr(line, "name=\"Comments\"");
				if(comm_p == nullptr)
				{
					continue;
				}

				const char *beg = 0, *end = 0;
				if(fndValRngUser("00", comm_p, &beg, &end))
				{
					strCpyRng(user_langs[ulangs_s].c, beg, end);
					htmlEntiti2chars(user_langs[ulangs_s].c);
				}
				if(fndValRngUser("03", comm_p, &beg, &end))
				{
					strCpyRng(user_langs[ulangs_s].bcb, beg, end);
					htmlEntiti2chars(user_langs[ulangs_s].bcb);
				}
				if(fndValRngUser("04", comm_p, &beg, &end))
				{
					strCpyRng(user_langs[ulangs_s].bce, beg, end);
					htmlEntiti2chars(user_langs[ulangs_s].bce);
				}

				++ulangs_s;
				continue;
			}

			// Skip User Langs without extentions defined
			const char *ext_beg = 0, *ext_end = 0;
			if(!fndValRng("ext=\"", lang_p, &ext_beg, &ext_end))
			{
				CloseHandle(f);
				return;
			}

			ui64 ext_s = (ui64)(ext_end - ext_beg + 1);
			user_langs[ulangs_s].n = (char *)MEM_ALLOC(ext_s+1);
			memcpy(user_langs[ulangs_s].n, ext_beg, ext_s);
			user_langs[ulangs_s].n[ext_s] = 0;
			continue;
		}
		if(li == MAX_LANG_LINE_SIZE-1)
		{
			ignore_cur_line = true;
			continue;
		}

		line[li] = ch;
		++li;
	}

	CloseHandle(f);
}

void extractULIdir(wchar_t *dir)
{
	static wchar_t find_file_querry[MAX_PATH];
	wcscpy(find_file_querry, dir);

	wcscat(find_file_querry, L"*.xml");
	ui64 usr_dl_s = wcslen(dir);

	WIN32_FIND_DATA fd;
	HANDLE f = FindFirstFile(find_file_querry, &fd);
	if(f == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		if(wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
		{
			continue;
		}

		wcscpy(dir + usr_dl_s, fd.cFileName);
		extractUserLangInfo(dir);
	} while(FindNextFile(f, &fd));

	FindClose(f);
}




#pragma warning( suppress : 4100 )
DWORD __declspec(nothrow) commDbLoadingThread(LPVOID lp)
{
	wchar_t *app_data = NULL;

	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &app_data);

	wchar_t npp_appdata_lang[MAX_PATH];
	wcscpy(npp_appdata_lang, app_data);
	wcscat(npp_appdata_lang, L"\\Notepad++\\langs.xml");

	// Create or open File or Device =================================================================
	HANDLE f = CreateFile(
		npp_appdata_lang,			//   [I]  Name of the file or device to create/open
		GENERIC_READ,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_READ,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_EXISTING,				//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,	//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================

	if(f == INVALID_HANDLE_VALUE)
	{

		return 1;
	}

	char line[MAX_LANG_LINE_SIZE];
	line[MAX_LANG_LINE_SIZE-1] = 0;
	ui64 li = 0;
	bool64 ignore_cur_line = false;
	DWORD br;
	char ch = 0;

	while(ReadFile(f, &ch, 1, &br, NULL) && br)
	{
		if(ignore_cur_line)
		{
			if(ch != '\n')
			{
				continue;
			}
			ignore_cur_line = false;
			li = 0;
			continue;
		}

		if(ch == '\n')
		{
			line[li] = '\n';
			line[li+1] = 0;
			li = 0;

			char *lang_p = strstr(line, "<Language ");
			if(lang_p == nullptr)
			{
				continue;
			}

			const char *name_beg = 0, *name_end = 0;
			fndValRng("name=\"", lang_p, &name_beg, &name_end);

			for(ui64 i = 0; i < langs_s; ++i)
			{
				if(strCmpRng(langs[i].n, name_beg, name_end))
				{
					const char *beg = 0, *end = 0;
					if(fndValRng("commentLine=\"", lang_p, &beg, &end))
					{
						strCpyRng(langs[i].c, beg, end);
						htmlEntiti2chars(langs[i].c);
					}
					if(fndValRng("commentStart=\"", lang_p, &beg, &end))
					{
						strCpyRng(langs[i].bcb, beg, end);
						htmlEntiti2chars(langs[i].bcb);
					}
					if(fndValRng("commentEnd=\"", lang_p, &beg, &end))
					{
						strCpyRng(langs[i].bce, beg, end);
						htmlEntiti2chars(langs[i].bce);
					}

					break;
				}
			}

			continue;
		}
		if(li == MAX_LANG_LINE_SIZE-1)
		{
			ignore_cur_line = true;
			continue;
		}

		line[li] = ch;
		++li;
	}

	CloseHandle(f);

	if(strcmp(langs[L_BATCH].c, "REM") == 0) // Fix command line comment
	{
		strcat(langs[L_BATCH].c, " ");
	}

	// Load up all user defined languages available

	ui64 ul_max_s = m2npp(NPPM_GETNBUSERLANG);
	user_langs = (UserLang *)MEM_ALLOC(ul_max_s * sizeof(UserLang));

	wcscpy(npp_appdata_lang, app_data);
	wcscat(npp_appdata_lang, L"\\Notepad++\\userDefineLang.xml");

	extractUserLangInfo(npp_appdata_lang);

	if(ulangs_s == ul_max_s)
	{
		return 0;
	}

	wcscpy(npp_appdata_lang, app_data);
	wcscat(npp_appdata_lang, L"\\Notepad++\\userDefineLangs\\");

	extractULIdir(npp_appdata_lang);

	if(ulangs_s == ul_max_s)
	{
		return 0;
	}

	m2npp(NPPM_GETNPPDIRECTORY, MAX_PATH, npp_appdata_lang);
	wcscat(npp_appdata_lang, L"\\userDefineLangs\\");

	extractULIdir(npp_appdata_lang);

	CoTaskMemFree(app_data);

	return 0;
}




ui64 fnd1stNonSp(ui64 line) // Returns first non-horisontal space character in line
{
	ui64 line_l = m2scintilla(SCI_LINELENGTH, line); // Including any line end characters
	char *line_b = (char *)MEM_ALLOC(line_l);

	m2scintilla(SCI_GETLINE, line, (ui64)line_b); // The buffer is not terminated by a NUL(0) character

	for(ui64 i = 0; i < line_l; ++i)
	{
		if(line_b[i] != '\t' && line_b[i] != ' ')
		{
			if(line_b[i] == '\r' || line_b[i] == '\n')
			{
				break;
			}

			MEM_FREE(line_b);
			return i;
		}
	}

	MEM_FREE(line_b);
	return NFND;
}

ui64 fnd1stNonSp(ui64 line, ui64 *h_sp) // Also returns number of horisonal spaces found
{
	ui64 line_l = m2scintilla(SCI_LINELENGTH, line); // Including any line end characters
	char *line_b = (char *)MEM_ALLOC(line_l);

	m2scintilla(SCI_GETLINE, line, (ui64)line_b); // The buffer is not terminated by a NUL(0) character

	*h_sp = 0;
	for(ui64 i = 0; i < line_l; ++i)
	{
		if(line_b[i] != '\t' && line_b[i] != ' ')
		{
			if(line_b[i] == '\r' || line_b[i] == '\n')
			{
				break;
			}

			MEM_FREE(line_b);
			return i;
		}
		++(*h_sp);
	}

	MEM_FREE(line_b);
	return NFND;
}

bool64 isComment(ui64 pos) // Checks if text at pos is comment start
{
	Sci_CharacterRangeFull cr; // Including cpMin, excluding cpMax
	cr.cpMin = (Sci_Position)pos;
	cr.cpMax = (Sci_Position)(pos + line_comm_s);

	static char ctag_probe[MAX_COMMENT_TAG_SIZE];
	Sci_TextRangeFull tr;
	tr.chrg = cr;
	tr.lpstrText = ctag_probe;

	m2scintilla(SCI_GETTEXTRANGEFULL, 0, (ui64)&tr); // NULL terminated

	return strcmp(line_comm, ctag_probe) == 0;
}

bool64 isLineBeg(ui64 pos)
{
	ui64 line = m2scintilla(SCI_LINEFROMPOSITION, pos);
	ui64 line_beg = m2scintilla(SCI_POSITIONFROMLINE, line);

	if(pos == line_beg) // Position is in the start of the line
	{
		return true;
	}

	ui64 beg_seg_s = pos - line_beg;

	Sci_CharacterRangeFull cr; // Including cpMin, excluding cpMax
	cr.cpMin = (Sci_Position)line_beg;
	cr.cpMax = (Sci_Position)pos;

	char *seg_b = (char *)MEM_ALLOC(beg_seg_s+1);
	Sci_TextRangeFull tr;
	tr.chrg = cr;
	tr.lpstrText = seg_b;

	m2scintilla(SCI_GETTEXTRANGEFULL, 0, (ui64)&tr);

	for(ui64 i = beg_seg_s-1; i != UI64_MAX; --i)
	{
		if(seg_b[i] != '\t' && seg_b[i] != ' ')
		{
			MEM_FREE(seg_b);
			return false;
		}
	}

	MEM_FREE(seg_b);
	return true;
}

bool64 isLineEnd(ui64 pos)
{
	ui64 line = m2scintilla(SCI_LINEFROMPOSITION, pos);
	ui64 line_end = m2scintilla(SCI_GETLINEENDPOSITION, line);
	ui64 line_beg = m2scintilla(SCI_POSITIONFROMLINE, line);

	if(line_end == m2scintilla(SCI_GETTEXTLENGTH))
	{
		return true;
	}

	if(pos == line_end || pos == line_beg) // Position is in the end of the line or beg. of next line
	{
		return true;
	}

	ui64 end_seg_s = line_end - pos;

	Sci_CharacterRangeFull cr; // Including cpMin, excluding cpMax
	cr.cpMin = (Sci_Position)pos;
	cr.cpMax = (Sci_Position)line_end;

	char *seg_e = (char *)MEM_ALLOC(end_seg_s+1);
	Sci_TextRangeFull tr;
	tr.chrg = cr;
	tr.lpstrText = seg_e;

	m2scintilla(SCI_GETTEXTRANGEFULL, 0, (ui64)&tr);

	for(ui64 i = 0; i < end_seg_s; ++i)
	{
		if(seg_e[i] != '\t' && seg_e[i] != ' ')
		{
			MEM_FREE(seg_e);
			return false;
		}
	}

	MEM_FREE(seg_e);
	return true;
}

bool64 isPosBegLine(ui64 pos, ui64 line)
{
	return pos == m2scintilla(SCI_POSITIONFROMLINE, line);
}

bool64 isBlockComm(ui64 pos, ui64 mode)
{
	Sci_CharacterRangeFull cr; // Including cpMin, excluding cpMax


	const char *bcomm;
	ui64 bcomm_s;

	if(mode & OPENING)
	{
		bcomm = block_beg;
		bcomm_s = block_beg_s;
	}
	else
	{
		bcomm = block_end;
		bcomm_s = block_end_s;
	}

	if(mode & FIND_AFTER)
	{
		cr.cpMin = (Sci_Position)pos;
		cr.cpMax = (Sci_Position)(pos + bcomm_s);
	}
	else
	{
		cr.cpMin = (Sci_Position)(pos - bcomm_s);
		cr.cpMax = (Sci_Position)pos;
	}

	static char ctag_probe[MAX_COMMENT_TAG_SIZE];
	Sci_TextRangeFull tr;
	tr.chrg = cr;
	tr.lpstrText = ctag_probe;

	m2scintilla(SCI_GETTEXTRANGEFULL, 0, (ui64)&tr); // NULL terminated

	return strcmp(bcomm, ctag_probe) == 0;
}

void insertTabs(ui64 amount, ui64 pos)
{
	for(ui64 i = 0; i < amount; ++i)
	{
		m2scintilla(SCI_INSERTTEXT, pos, (ui64)"\t");
	}
}

bool64 fndInSpaceSepArr(const char *fnd, const char *arr) // Find in space (0x20) separated array of strings
{
	const char *fnd_b = fnd;
	const char *arr_cmp = arr;
	bool64 skip = false;
	--arr;
	while(*(++arr) != 0)
	{
		if(*arr == ' ')
		{
			skip = false;
			++arr;
			fnd = fnd_b;
			arr_cmp = arr;
		}
		if(skip)
		{
			continue;
		}

		bool64 match = true;
		while(*arr_cmp != ' ' && *arr != 0 && *fnd != 0)
		{
			if(*arr_cmp != *fnd)
			{
				skip = true;
				match = false;
				break;
			}
			++arr_cmp, ++fnd;
		}

		if(match)
		{
			if((*arr_cmp == ' ' || *arr_cmp == 0) && *fnd == 0)
			{
				return true;
			}

			arr = arr_cmp - 1;
		}
	}

	return false;
}

void wcs2str(char *dest, const wchar_t *src)
{
	while(*src != 0)
	{
		*dest = (char)(*src);
		++dest, ++src;
	}
	*dest = 0;
}




i64 doToggleComment(ui64 sel_s, ui64 sel_e, ui64 sel_i)
{
	if(line_comm_s != 0)
	{
		if(sel_s == sel_e) // No selection, single line mode
		{
			return togSingSelLineComm(sel_s, sel_e, sel_i);
		}

		if(block_beg_s != 0 && block_end_s != 0)
		{
			if(!isLineBeg(sel_s) || !isLineEnd(sel_e)) /* ~~~ Block comment mode ~~~ */
			{
				return togBlockComm(sel_s, sel_e, sel_i);
			}
		}

		return togMultiSelLineComm(sel_s, sel_e, sel_i); // Multiline line comment mode
	}

	if(block_beg_s != 0 && block_end_s != 0) // No line comment tag found, try to use block mode
	{
		return togBlockComm(sel_s, sel_e, sel_i);
	}

	return 0;
}

i64 togSingSelLineComm(ui64 sel_s, ui64 sel_e, ui64 sel_i)
{
	ui64 line = m2scintilla(SCI_LINEFROMPOSITION, sel_s);

	ui64 non_sp = fnd1stNonSp(line);
	ui64 abs_pos = TEND; // Absolute position
	if(non_sp == TEND)
	{
		non_sp = sel_s;
	}
	else
	{
		ui64 line_beg = m2scintilla(SCI_POSITIONFROMLINE, line);
		abs_pos = non_sp + line_beg;
	}

	if(isComment(abs_pos))
	{
		m2scintilla(SCI_DELETERANGE, abs_pos, line_comm_s);
		sel_s -= line_comm_s;
		sel_e -= line_comm_s;

		selects[sel_i].beg = sel_s;
		selects[sel_i].end = sel_e;
		return (i64)line_comm_s * -1;
	}

	m2scintilla(SCI_INSERTTEXT, abs_pos, (ui64)line_comm);
	sel_s += line_comm_s;
	sel_e += line_comm_s;

	selects[sel_i].beg = sel_s;
	selects[sel_i].end = sel_e;
	return (i64)line_comm_s;
}

i64 togMultiSelLineComm(ui64 sel_s, ui64 sel_e, ui64 sel_i)
{
	i64 ch_changed = 0; // Total amount of characters inserted/removed

	ui64 line0 = m2scintilla(SCI_LINEFROMPOSITION, sel_s);
	ui64 line1 = m2scintilla(SCI_LINEFROMPOSITION, sel_e);

	if(fnd1stNonSp(line0) == NFND)
	{
		++line0;
	}
	if(isPosBegLine(sel_e, line1))
	{
		--line1;
	}

	ui64 min_indent = UI64_MAX;
	for(ui64 i = line0; i <= line1; ++i)
	{
		ui64 indent = fnd1stNonSp(i);
		if(indent < min_indent)
		{
			min_indent = indent;
		}
	}

	ui64 h_sp = 0;
	ui64 line0_beg = m2scintilla(SCI_POSITIONFROMLINE, line0);
	ui64 line0_abs_indent = fnd1stNonSp(line0, &h_sp) + line0_beg;
	ui64 line0_abs_pos = line0_beg + min_indent;
	bool64 to_uncomment = isComment(line0_abs_indent);
	ui64 tot_l_com = 0, tot_l_uncom = 0; // Total commented/uncommented lines counters

	if(to_uncomment)
	{
		m2scintilla(SCI_DELETERANGE, line0_abs_indent, line_comm_s);
		ch_changed -= line_comm_s;
		++tot_l_uncom;
	}
	else
	{
		m2scintilla(SCI_INSERTTEXT, line0_abs_pos, (ui64)line_comm);
		ch_changed += line_comm_s;
		++tot_l_com;
	}

	if(h_sp > 0 && sel_s > line0_abs_pos)
	{
		sel_s = to_uncomment ? line0_abs_indent : line0_abs_pos;
	}

	ui64 addt_ins = 0; // Additional inserts
	for(ui64 i = line0+1; i <= line1; ++i)
	{
		ui64 line_beg = m2scintilla(SCI_POSITIONFROMLINE, i);
		ui64 abs_pos = min_indent + line_beg; // Absolute position

		ui64 non_sp = fnd1stNonSp(i, &h_sp);
		if(non_sp == NFND) // Line contains no non-space characters!
		{
			if(min_indent > h_sp)
			{
				ui64 tabs_need = min_indent - h_sp;
				addt_ins += tabs_need;
				insertTabs(tabs_need, line_beg);
				ch_changed += tabs_need;
			}
		}

		ui64 abs_indent = non_sp + line_beg;

		if(to_uncomment)
		{
			if(!isComment(abs_indent))
			{
				continue;
			}

			m2scintilla(SCI_DELETERANGE, abs_indent, line_comm_s);
			ch_changed -= line_comm_s;
			++tot_l_uncom;
		}
		else
		{
			m2scintilla(SCI_INSERTTEXT, abs_pos, (ui64)line_comm);
			ch_changed += line_comm_s;
			++tot_l_com;
		}
	}

	// Account for characters inserted/removed to maintain the same initial selection area
	sel_e = to_uncomment ? sel_e - tot_l_uncom * line_comm_s : sel_e + tot_l_com * line_comm_s;
	sel_e += addt_ins;

	selects[sel_i].beg = sel_s;
	selects[sel_i].end = sel_e;
	return ch_changed;
}

i64 togBlockComm(ui64 sel_s, ui64 sel_e, ui64 sel_i)
{
	i64 ch_changed = 0; // Total amount of characters inserted/removed

	bool64 to_uncomment_beg = false, to_uncomment_end = false;
	ui64 obc_bpos = sel_s, cbc_bpos = sel_e; // Block comment strings beginning positions

	if(isBlockComm(sel_s, FIND_AFTER | OPENING))
	{
		to_uncomment_beg = true;
	}
	else
	{
		if(isBlockComm(sel_s, FIND_BEFORE | OPENING))
		{
			obc_bpos -= block_beg_s;
			to_uncomment_beg = true;
		}
	}

	if(isBlockComm(sel_e, FIND_BEFORE | CLOSING))
	{
		cbc_bpos -= block_end_s;
		to_uncomment_end = true;
	}
	else
	{
		if(isBlockComm(sel_e, FIND_AFTER | CLOSING))
		{
			to_uncomment_end = true;
		}
	}

	if(to_uncomment_beg && to_uncomment_end)
	{
		m2scintilla(SCI_DELETERANGE, obc_bpos, block_beg_s);
		ch_changed -= block_beg_s;
		cbc_bpos -= block_beg_s;
		m2scintilla(SCI_DELETERANGE, cbc_bpos, block_end_s);
		ch_changed -= block_end_s;
		sel_s -= sel_s - obc_bpos;
		sel_e -= block_beg_s;
		sel_e -= sel_e - cbc_bpos;
	}
	else
	{
		m2scintilla(SCI_INSERTTEXT, obc_bpos, (ui64)block_beg);
		ch_changed += block_beg_s;
		cbc_bpos += block_beg_s;
		m2scintilla(SCI_INSERTTEXT, cbc_bpos, (ui64)block_end);
		ch_changed += block_end_s;
		sel_s += block_beg_s;
		sel_e += block_beg_s;
	}

	selects[sel_i].beg = sel_s;
	selects[sel_i].end = sel_e;
	return ch_changed;
}




void spawnMsgWnd()
{
	static WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = msgWndProc;
	wc.lpszClassName = plugin_msg_wnd_class;
	RegisterClassEx(&wc);
	msg_wnd = CreateWindowEx(0, plugin_msg_wnd_class, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
}

#pragma warning( suppress : 4100 )
LRESULT CALLBACK msgWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == NPP_PLUGIN_GET_SCINTILLA)
	{
		updateScintillaHwnd();

		return (LRESULT)scintilla;
	}

	if(msg == NPP_PLUGIN_GET_CUR_FILE_NAME)
	{
		static wchar_t curn[MAX_PATH];
		ui64 buff_id = m2npp(NPPM_GETCURRENTBUFFERID);
		ui64 curn_s = m2npp(NPPM_GETFULLPATHFROMBUFFERID, buff_id, curn);

		static COPYDATASTRUCT data;
		data.dwData = 1;
		data.cbData = ((DWORD)curn_s + 1) * sizeof(wchar_t);
		data.lpData = PVOID(curn);

		SendMessage((HWND)wp, WM_COPYDATA, 0, (LPARAM)&data);

		return TRUE;
	}

	if(msg == WM_COPYDATA)
	{
		COPYDATASTRUCT *data = (COPYDATASTRUCT *)lp;
		return (LRESULT)m2npp(NPPM_DOOPEN, 0, data->lpData);
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

#pragma warning( suppress : 4100 )
DWORD __declspec(nothrow) msgProcThread(LPVOID lp)
{
	spawnMsgWnd();

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}

	return 0;
}



bool64 isSymbolChar(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z' || c == '_');
}

char *getSymbol(ui64 pos)
{
	ui64 line = m2scintilla(SCI_LINEFROMPOSITION, pos);
	ui64 line_beg = m2scintilla(SCI_POSITIONFROMLINE, line);
	ui64 line_pos = pos - line_beg;
	ui64 line_l = m2scintilla(SCI_LINELENGTH, line); // Including any line end characters
	static ui64 dbg_pre_s = strlen(dbg_prefix);
	char *line_b = (char *)MEM_ALLOC(line_l);

	m2scintilla(SCI_GETLINE, line, (ui64)line_b); // The buffer is not terminated by a NUL(0) character

	ui64 bp = line_pos;
	while(bp != UI64_MAX && isSymbolChar(line_b[bp]))
	{
		--bp;
	}

	if(bp == UI64_MAX) // Handle overflow
	{
		bp = 0;
	}
	else
	{
		++bp;
	}

	ui64 ep = line_pos;
	while(ep < line_l && isSymbolChar(line_b[ep]))
	{
		if(line_b[ep] == '_' && ep + dbg_pre_s < line_l)
		{
			if(strncmp(line_b+ep, dbg_prefix, dbg_pre_s) == 0)
			{
				break;
			}

		}

		++ep;
	}
	--ep;

	char *symb_b = NULL;
	ui64 symb_s = ep - bp + 1;

	if(bp > ep) // Caret is at end of the line and/or whitespace at the EOL
	{
		symb_b = (char *)MEM_ALLOC(14);
		strncpy(symb_b, "_DBG_NO_SYMB_", 14);
		goto exit_return;
	}

	symb_b = (char *)MEM_ALLOC(symb_s+1);

	strncpy(symb_b, line_b+bp, symb_s);
	symb_b[symb_s] = 0;

exit_return:
	MEM_FREE(line_b);
	return symb_b;
}

char *getSelection(ui64 beg, ui64 end)
{
	Sci_CharacterRangeFull cr; // Including cpMin, excluding cpMax
	cr.cpMin = (Sci_Position)beg;
	cr.cpMax = (Sci_Position)end;

	char *seg_b = (char *)MEM_ALLOC(end - beg + 1);
	Sci_TextRangeFull tr;
	tr.chrg = cr;
	tr.lpstrText = seg_b;

	m2scintilla(SCI_GETTEXTRANGEFULL, 0, (ui64)&tr);
	return seg_b;
}

ui64 countNextSymbs(ui64 *pos, const txt &t)
{
	ui64 tot_symbs = 0;
	bool64 skip_non_symb = false;
	for(ui64 i = *pos; i < ~t; ++i)
	{
		if(!isSymbolChar(t[i]))
		{
			if(t[i] == ',')
			{
				for(ui64 j = i+1; j < ~t; ++j)
				{
					if(isSymbolChar(t[j]))
					{
						i = j;
						break;
					}
				}

				*pos = i;
				return ++tot_symbs;
			}
			else if(t[i] == ')')
			{
				if(i == *pos) // No symbols found
				{
					*pos = NFND;
					return 0;
				}

				*pos = NFND;
				return ++tot_symbs;
			}

			if(!skip_non_symb)
			{
				++tot_symbs;
				skip_non_symb = true;
			}
		}
		else
		{
			skip_non_symb = false;
		}
	}

	return tot_symbs;
}

ui64 getSymbBeg(ui64 pos, const txt &t)
{
	for(ui64 i = pos; i != UI64_MAX; --i)
	{
		if(!isSymbolChar(t[i]))
		{
			return i+1;
		}
	}

	return 0;
}

ui64 getSymbEnd(ui64 pos, const txt &t)
{
	for(ui64 i = pos; i < ~t; ++i)
	{
		if(!isSymbolChar(t[i]))
		{
			return i-1;
		}
	}

	return ~t - 1;
}

bool64 isFuncDecl(const txt &t, ui64 *symb_pos)
{
	bool64 no_symbols = *symb_pos == NFND;
	
	ui64 last_brak = NFND, last_semi_col = NFND, last_sc = 0;
	for(ui64 i = *symb_pos+1; i < ~t; ++i) // NFND will overflow into 0
	{
		if(t[i] == ')')
		{
			*symb_pos = no_symbols ? i : i-1;
			last_brak = i;
		}
		else if(t[i] == ';')
		{
			*symb_pos = no_symbols ? i-1 : i-2;
			last_semi_col = i;
		}
		else if(isSymbolChar(t[i]))
		{
			last_sc = i;
		}
		else if(i+1 < ~t && t[i] == '/' && t[i+1] == '/')
		{
			break;
		}
	}

	if(last_sc > last_brak) // Modifier found
	{
		*symb_pos = last_sc;
	}

	return (last_semi_col != NFND) && (last_semi_col > last_brak);
}

void nppGoToLine(ui64 line)
{
	m2scintilla(SCI_ENSUREVISIBLE, line);
	m2scintilla(SCI_GOTOLINE, line);

	// Scroll window so that the line is in the middle of view
	LRESULT sl = (LRESULT)m2scintilla(SCI_LINESONSCREEN);

	i64 scr_l = (i64)line - sl/2;
	i64 doc_vl = scr_l < 0 ? 0 : scr_l;

	i64 hidden_ls = 0;
	for(i64 i = (i64)line; i >= doc_vl; --i)
	{
		if(m2scintilla(SCI_GETLINEVISIBLE, (ui64)i) == 0)
		{
			++hidden_ls;
		}
	}

	doc_vl -= hidden_ls;
	WPARAM first_doc_vl = doc_vl < 0 ? 0 : (WPARAM)doc_vl;
	WPARAM first_vis_vl = (WPARAM)m2scintilla(SCI_VISIBLEFROMDOCLINE, first_doc_vl);

	m2scintilla(SCI_SETFIRSTVISIBLELINE, first_vis_vl);
}

void nppGoToPos(ui64 pos)
{
	ui64 line = m2scintilla(SCI_LINEFROMPOSITION);
	nppGoToLine(line);
	m2scintilla(SCI_GOTOPOS, pos);
}

bool64 checkTxt(char *src, const txt &t)
{
	const char *tb = t;
	--src, --tb;
	while(*++src != 0 && *++tb != 0)
	{
		if(*src != *tb)
		{
			return false;
		}
	}

	return true;
}

bool64 checkTxtHasType(char *sbeg, bool64 in_class)
{
	if(*(sbeg - 1) == '\t' && in_class) // Class typeless function
	{
		return true;
	}

	static txt ret = L("return");
	txt symb;
	bool64 type_found = false;
	while(*--sbeg != '\n')
	{
		if(
			*sbeg == ' '	||	*sbeg == '\t'	||
			*sbeg == '&'	||	*sbeg == '*'	||
			*sbeg == ':'	||	*sbeg == '('	||
			*sbeg == ')')
		{
			if(symb == ret) // Return is not a type!
			{
				return false;
			}

			symb = 0;
			continue;
		}

		if(!isSymbolChar(*sbeg))
		{
			return false;
		}

		txti(symb, 0, *sbeg);
		type_found = true;
	}

	return type_found;
}

bool64 semicolInLine(const char *ln)
{
	--ln;
	while(*++ln != '\n' && *ln != 0)
	{
		if(*ln == ';')
		{
			return true;
		}
	}

	return false;
}

ui64 findFuncDefin(char *doc, ui64 bpos, const txt &symb, const txt &args)
{
	char *d = doc;
	d += bpos - 1;
	while(*++d != 0)
	{
		if(checkTxt(d, symb) && checkTxt(d + ~symb + 1, args) && checkTxtHasType(d, false))
		{
			if(*(d + ~symb) != '(' || semicolInLine(d)) // Only function prefix match or semicolon found
			{
				continue;
			}

			ui64 sbeg_p = (ui64)(d - doc);
			ui64 line = m2scintilla(SCI_LINEFROMPOSITION, sbeg_p);

			nppGoToLine(line);
			ui64 colon = txtfe(symb, TEND, ':'); // Don't select class prefix
			ui64 trueb = colon != NFND ? sbeg_p + colon + 1 : sbeg_p;
			m2scintilla(SCI_SETSELECTIONSTART, trueb);
			m2scintilla(SCI_SETSELECTIONEND, sbeg_p + ~symb);
			return line;
		}
	}

	return NFND;
}

ui64 findFuncDecl(char *doc, ui64 epos, const txt &symb, const txt &args)
{
	static const txt cls = L("class");
	bool64 in_class = false;
	char *end = doc + epos;
	char *d = doc;
	--d;
	while(++d != end)
	{
		if(checkTxt(d, cls))
		{
			in_class = true;
		}

		if(checkTxt(d, symb) && checkTxt(d + ~symb + 1, args) && checkTxtHasType(d, in_class))
		{
			if(*(d + ~symb) != '(') // Only function prefix match
			{
				continue;
			}

			ui64 sbeg_p = (ui64)(d - doc);
			ui64 line = m2scintilla(SCI_LINEFROMPOSITION, sbeg_p);

			nppGoToLine(line);
			ui64 colon = txtfe(symb, TEND, ':'); // Don't select class prefix
			ui64 trueb = colon != NFND ? sbeg_p + colon + 1 : sbeg_p;
			m2scintilla(SCI_SETSELECTIONSTART, trueb);
			m2scintilla(SCI_SETSELECTIONEND, sbeg_p + ~symb);
			return line;
		}

		if(d+2 < end && *d == '\n' && *d+1 == '}' && *d+2 == ';')
		{
			in_class = false;
		}
	}

	return NFND;
}

bool64 fileExists(const wtxt &fn)
{
	DWORD attr = GetFileAttributes(fn);
	if(attr == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	return true;
}

void findFuncInOtherFile(const txt &symb, const txt &args)
{
	wtxt path = MAX_PATH;
	ui64 buff_id = m2npp(NPPM_GETCURRENTBUFFERID);
	ui64 path_s = m2npp(NPPM_GETFULLPATHFROMBUFFERID, buff_id, (const wchar_t *)path);
	txtssz(path, path_s);

	ui64 dot = txtfe(path, TEND, '.');
	wtxt file_targ;
	if(dot == NFND)
	{
		file_targ = path + WL(".cpp");
	}
	else
	{
		wtxt ext = txts(path, dot+1, TEND);
		if(ext == 'h')
		{
			file_targ = path;
			txtr(file_targ, dot+1, 3, WL("cpp"));
		}
		else if(ext == WL("cpp"))
		{
			file_targ = path;
			txtr(file_targ, dot+1, 3, 'h');
			if(!fileExists(file_targ))
			{
				txtd(file_targ, dot, TEND);
			}
		}
		else
		{
			return;
		}
	}

	// Create or open File or Device =================================================================
	HANDLE f = CreateFile(
		file_targ,					//   [I]  Name of the file or device to create/open
		GENERIC_READ,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_READ,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_EXISTING,				//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,	//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================

	if(f == INVALID_HANDLE_VALUE)
	{
		return;
	}

	txt file = FILE_READ_BUFF_SZ;
	txt buf = FILE_READ_BUFF_SZ;
	while(ReadFile(f, buf, FILE_READ_BUFF_SZ, *buf, NULL) && ~buf)
	{
		file += buf;
	}

	CloseHandle(f); // Always check the handle!

	static const txt cls = L("class");
	bool64 in_class = false;
	ui64 line_num = 0;
	char *d = file;
	--d;
	while(*++d != 0)
	{
		if(checkTxt(d, cls))
		{
			in_class = true;
		}

		if(checkTxt(d, symb) && checkTxt(d + ~symb + 1, args) && checkTxtHasType(d, in_class))
		{
			m2npp(NPPM_DOOPEN, 0, (const wchar_t *)file_targ);

			ui64 sbeg_p = (ui64)(d - file);

			nppGoToLine(line_num);
			ui64 colon = txtfe(symb, TEND, ':'); // Don't select class prefix
			ui64 trueb = colon != NFND ? sbeg_p + colon + 1 : sbeg_p;
			m2scintilla(SCI_SETSELECTIONSTART, trueb);
			m2scintilla(SCI_SETSELECTIONEND, sbeg_p + ~symb);
			return;
		}
		else if(*d == '\n')
		{
			++line_num;
		}

		if(*(d+2) != 0 && *d == '\n' && *d+1 == '}' && *d+2 == ';')
		{
			in_class = false;
		}
	}
}

bool64 hasType(const txt &line, ui64 sbeg, ui64 lpos_abs)
{
	if(line[sbeg-1] == '\t') // Class typeless functions or function call?
	{
		char *doc = (char *)m2scintilla(SCI_GETCHARACTERPOINTER);
		ui64 i = 0;
		while(i < lpos_abs)
		{
			if(
			i+5 < lpos_abs   &&
			doc[i]   == '\n' &&
			doc[i+1] == 'c'  &&
			doc[i+2] == 'l'  &&
			doc[i+3] == 'a'  &&
			doc[i+4] == 's'  &&
			doc[i+5] == 's')
			{
				return true;
			}
			++i;
		}

		return false;
	}

	static const txt ret = L("return");
	txt symb;
	bool64 type_found = false;
	for(ui64 i = sbeg-1; i != UI64_MAX; --i)
	{
		if(
			line[i] == ' '	||	line[i] == '\t'	||
			line[i] == '&'	||	line[i] == '*'	||
			line[i] == ':'	||	line[i] == '('	||
			line[i] == ')')
		{
			if(symb == ret) // Return is not a type!
			{
				return false;
			}

			symb = 0;
			continue;
		}

		if(!isSymbolChar(line[i]))
		{
			return false;
		}

		txti(symb, 0, line[1]);
		type_found = true;
	}

	return type_found;
}

txt findClassName(char *doc, ui64 sbeg)
{
	txt cls = L("class");
	char *d = doc + sbeg;
	while(--d != doc)
	{
		if(checkTxt(d, cls) && d-1 != doc && *(d-1) == '\n')
		{
			cls = 0;
			char *cn = d + 5;
			while(*++cn != '\r')
			{
				cls += *cn;
			}

			break;
		}
	}

	return cls;
}

//class wtxta : public darr <<<<<< can't jump from funcs in classes with inheritance!
//{
//public:

//wtxta & Clear(); defin cannot be found, but declaration is found SAME WITH UI64A CLASS!

// TODO DISABLE FUNC JUMPER IN NON CPP/NONAME FILES!!!
// TODO TRY TO GET LIST OF ALL OPENED FILES AND IF CPP/H FILE IS OPEN GET BUFFER OF IT INSTEAD OF STUPIDLY OPENING FILE AND READING IT EVERY TIME!!
// TODO SELECTION GETS BROKEN SOMETIMES WHEN JUMPING TO FUNCTION. ENTIRE TEXT FROM DECL. TO DEFINITION IS SELECTED INSTEAD OF DESTINATION FUNC NAME
DWORD __declspec(nothrow) functionJumper(void *scn)
{
	ui64 pos = (ui64)((SCNotification *)scn)->position;

	if(pos == NFND) // Double click in virtual space
	{
		return 1;
	}

	ui64 line_num = (ui64)((SCNotification *)scn)->line;
	//ui32 mods = scn->modifiers;

	updateScintillaHwnd();

	if(!isSymbolChar((char)m2scintilla(SCI_GETCHARAT, pos)))
	{
		return 1;
	}

	ui64 line_beg_p = m2scintilla(SCI_POSITIONFROMLINE, line_num);
	ui64 pos_in_l = pos - line_beg_p;

	txt line = getLine(line_num);

	ui64 sbeg = getSymbBeg(pos_in_l, line);
	ui64 send = getSymbEnd(pos_in_l, line);

	if(sbeg-1 != UI64_MAX && line[sbeg-1] == '~') // Possible destructor!
	{
		if(sbeg-2 != UI64_MAX && (line[sbeg-2] == '\t' || line[sbeg-2] == ':'))
		{
			--sbeg; // Possible bug: a<b?foo():~bar(); ---> But very rare! Bad coding, no spaces
		}
	}

	txt symb = txtsp(line, sbeg, send);

	if(line[send+1] != '(')
	{
		if(symb == L("operator"))
		{
			send = txtf(line, send, '(') - 1;
			symb = txtsp(line, sbeg, send);
		}
		else
		{
			return 1;
		}
	}

	ui64 arg_pos = send + 2; // Skip '('
	ui64 arg_pos_next = arg_pos;
	ui64 symb_num = countNextSymbs(&arg_pos_next, line);
	if(symb_num == 1) // Arguments has no type, function call
	{
		return 1;
	}
	else if(symb_num == 0 && !hasType(line, sbeg, line_beg_p)) // Function call
	{
		return 1;
	}

	// Scintilla ensures there is a NUL character after the text
	char *doc = (char *)m2scintilla(SCI_GETCHARACTERPOINTER);

	if(isFuncDecl(line, &arg_pos_next))
	{
		if(line[0] == '\t') // Class function
		{
			ui64 amigo = txtfr(line, 1, 6, L("friend"));
			if(amigo == NFND) // Member function
			{
				symb = findClassName(doc, sbeg + line_beg_p) + L("::") + symb;
			}
		}

		txt args = txtsp(line, arg_pos, arg_pos_next);
		ui64 targ_line_n = findFuncDefin(doc, arg_pos_next + line_beg_p, symb, args);

		if(targ_line_n == NFND)
		{
			findFuncInOtherFile(symb, args);
		}
	}
	else
	{
		txt args = txtsp(line, arg_pos, arg_pos_next);
		ui64 targ_line_n = findFuncDecl(doc, sbeg + line_beg_p, symb, args);

		if(targ_line_n == NFND)
		{
			findFuncInOtherFile(symb, args);
		}
	}

	return 0;
}

void updateCppParams()
{
	updateScintillaHwnd();
	if((char)m2scintilla(SCI_GETCHARAT, 0) != '/')
	{
		return;
	}

	txt line = getLine(0);

	cpp_console_set = txtf(line, 0, cpp_console) != NFND;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_CONSOLE]._cmdID, cpp_console_set);

	cpp_nopt_set = txtf(line, 0, cpp_nopt) != NFND;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_NOPT]._cmdID, cpp_nopt_set);

	cpp_dbg_set = txtf(line, 0, cpp_dbg) != NFND;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_DBG]._cmdID, cpp_dbg_set);

	cpp_nlaunch_set = txtf(line, 0, cpp_nlaunch) != NFND;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_NLAUNCH]._cmdID, cpp_nlaunch_set);
	
	cpp_asm_set = txtf(line, 0, cpp_asm) != NFND;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_ASM]._cmdID, cpp_asm_set);

	line = getLine(1);

	cpp_dbg_bp_off_set = line == cpp_dbg_bp_off;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[CMD_DBGP_DISABLED]._cmdID, cpp_dbg_bp_off_set);
}

void toggleCppParam(bool64 &cpp_param_set, ui64 cmd_func, const txt &cmd_param_txt)
{
	updateScintillaHwnd();
	if((char)m2scintilla(SCI_GETCHARAT, 0) != '/')
	{
		return;
	}

	cpp_param_set = !cpp_param_set;
	m2npp(NPPM_SETMENUITEMCHECK, (ui64)funcs[cmd_func]._cmdID, cpp_param_set);

	ui64 line_end = m2scintilla(SCI_GETLINEENDPOSITION, 0);

	if(cpp_param_set)
	{
		if(cmd_param_txt == cpp_console)
		{
			line_end = 2;
		}

		m2scintilla(SCI_INSERTTEXT, line_end, cmd_param_txt);
		m2npp(NPPM_SAVECURRENTFILE);
		return;
	}

	ui64 line_l = m2scintilla(SCI_LINELENGTH, 0); // Including any line end characters
	txt line = line_l;
	m2scintilla(SCI_GETLINE, 0, line); // The buffer is not terminated by a NUL(0) character
	txtssz(line, line_l);

	ui64 cmd_pos = txtf(line, 0, cmd_param_txt);
	m2scintilla(SCI_DELETERANGE, cmd_pos, ~cmd_param_txt);
	m2npp(NPPM_SAVECURRENTFILE);
}

txt getLine(ui64 line_i)
{
	ui64 line_l = m2scintilla(SCI_LINELENGTH, line_i); // Including any line end characters
	txt line = line_l;
	m2scintilla(SCI_GETLINE, line_i, line);	// The buffer is not terminated by a NUL(0) character
	txtssz(line, line_l);
	return line;
}

txt getTextRange(ui64 b, ui64 e)
{
	Sci_CharacterRangeFull cr; // Including cpMin, excluding cpMax
	cr.cpMin = (Sci_Position)b;
	cr.cpMax = (Sci_Position)e;

	txt out = e - b;
	txtssz(out, !out-1);

	Sci_TextRangeFull tr;
	tr.chrg = cr;
	tr.lpstrText = out;

	m2scintilla(SCI_GETTEXTRANGEFULL, 0, (ui64)&tr); // NULL terminated

	return out;
}

namespace includeSddbFunc
{
	static txt line; // Global "local" static variable
}
void includeSddb()
{
	using namespace includeSddbFunc;
	if((char)m2scintilla(SCI_GETCHARAT, 0) != '/')
	{
		return;
	}
	
	ui64 i = 1;
	do
	{
		line = getLine(i++);
	} while(txtfr(line, 0, 6, cpp_define) != NFND);

	if(line == cpp_sddb )
	{
		return;
	}

	ui64 line_beg = m2scintilla(SCI_POSITIONFROMLINE, i-1);
	m2scintilla(SCI_INSERTTEXT, line_beg, cpp_sddb);
}

void removeDbgPrints(ui64 s, ui64 e)
{
	static txt dbg = L("_DBG_");
	static ui64a pb; // DBG print begin pos
	static ui64a ps; // DBG print size

	// Scintilla ensures there is a NUL character after the text
	char *doc = (char *)m2scintilla(SCI_GETCHARACTERPOINTER);
	char *d = doc + s - 1;
	char *de = doc + e;
	while(++d != de)
	{
		if(checkTxt(d, dbg))
		{
			char *dbg_prfx = d + ~dbg - 1;
			while(*++dbg_prfx != 0)
			{
				if(*dbg_prfx == ';')
				{
					pb << (ui64)(d - doc);
					ps << (ui64)(dbg_prfx - d + 1);

					d = dbg_prfx;
					break;
				}
			}
		}
	}

	ui64 dtot = 0; // Total amount of deleted characters

	m2scintilla(SCI_BEGINUNDOACTION); // UNDO =========================
	for(ui64 i = 0; i < ~pb; ++i)
	{
		m2scintilla(SCI_DELETERANGE, pb[i] - dtot, ps[i]);
		dtot += ps[i];
	}
	m2scintilla(SCI_ENDUNDOACTION);   // ==============================

	pb.Clear(), ps.Clear();
}

// FIND IN FILES WINDOW ============================================================

struct FindResult
{
	ui16 idx;	// Index size in line
	ui16 p1;	// Size of the part 1 in the result
	ui16 p2;	// Size of the part 2 in the result
	ui16 is_f;	// This result line is a filename
	ui64 pos;	// Absolute position of pattern in file
	txt line;
};

class FindResults : public darr
{
public:
	FindResults()
	{
		ts = 256;
		s = 0;
		d = (FindResult *)Alloc(ts * sizeof(FindResult));
		memset(d, 0, ts * sizeof(FindResult)); // All texts invalidated
	}
	FindResults(const FindResults &o) = delete;

	~FindResults()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			txtfree(d[i].line);
		}
		DARR_FREE(d);
	}

	FindResult & operator[](ui64 i) { return d[i]; }
	const FindResult & operator[](ui64 i) const { return d[i]; }

	FindResults & operator<<(const FindResult &c)
	{
		ui64 ns = s + 1;
		if(ts < ns)
		{
			d = (FindResult *)ReAlloc(d, ns, sizeof(FindResult));
			memset(&d[s], 0, (ts - s) * sizeof(FindResult)); // All new texts invalidated
		}

		d[s] = c;
		s = ns;

		return *this;
	}

	FindResults & Clear()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			txtfree(d[i].line); // Will invalidate texts later
		}

		memset(d, 0, s * sizeof(FindResult)); // All texts invalidated
		s = 0;

		return *this;
	}

private:
	FindResult *d;
};

static FindResults results;
static FindResult result = { 0 };

static HINSTANCE exec_adress;
static ATOM main_class;
static HWND main_wnd;

static double dpi_scale;
static HFONT default_font;
static HBRUSH backgrnd;
static HDC main_dc;

static COLORREF def_back = 0x0C0C0C; // 0x00BBGGRR
static COLORREF hov_back = 0x505050;
static COLORREF def_front = 0xCCCCCC;
static COLORREF index_c = 0x55CCAF;
static COLORREF pattern_c = 0xBA37CC;
static COLORREF file_path_c = 0x49CC37;

static ui64 cellw = 8;
static ui64 cellh = 16;

static ui64 clw, clh; // Client width/height

static ui64 y_scrl, x_scrl;

static ui64 max_line_l; // Maximum line length

bool64 fnd_in_files_wnd_active;

static UINT mwheel_scroll_ln, mwheel_scroll_ch; // System values for mouse wheel scrolling

static RECT work_pix; // Working pixels

static ui64 matches_found;
static txt title_prefix = L("Matches found: ");
static txt title = 128;

static txt pattern;

#define HW(x) (((unsigned int)x) >> 16)
#define LW(x) (((unsigned int)x) & 0xFFFF)
#define HWSIGN(x) ((int)(short)HW(x))
#define LWSIGN(x) ((int)(short)LW(x))
#define DPIS(x) ((ui64)((double)x * dpi_scale)) // Scale value for DPI

static void updateMaxLine(const FindResult &ln)
{
	if(max_line_l < ~ln.line)
	{
		static SCROLLINFO si;
		si.cbSize = sizeof(si);

		max_line_l = ~ln.line;
		si.fMask = SIF_RANGE;
		si.nMin = 0;
		si.nMax = (i32)max_line_l;
		SetScrollInfo(main_wnd, SB_HORZ, &si, TRUE);
		InvalidateRect(main_wnd, NULL, FALSE);
	}
}

static void updateScrollState()
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);

	updateMaxLine(result);

	si.fMask = SIF_RANGE/* | SIF_POS*/;
	si.nMin = 0;
	si.nMax = (i32)~results - 1;
	//si.nPos = si.nMax;
	
	SetScrollInfo(main_wnd, SB_VERT, &si, TRUE);

	if(result.is_f)
	{
		InvalidateRect(main_wnd, NULL, FALSE);
		//ScrollWindow(main_wnd, 0, (i32)cellh, NULL, NULL);
	}
}

static void pushLine()
{
	results << result;

	if(main_wnd == NULL)
	{
		return;
	}

	updateScrollState();
}

txt pure_line = 1023;

void clipLines(i64 min, i64 max)
{
	if(min > (i64)~results)
	{
		return;
	}

	txt to_clip;
	for(ui64 i = (ui64)min; i <= (ui64)max; ++i)
	{
		to_clip += txts(pure_line, results[i].line, txtf(results[i].line, 0, ':') + 2, TEND);
		to_clip += '\r';
		to_clip += '\n';
	}
	txtdl(to_clip), txtdl(to_clip);

	clip(to_clip, main_wnd);
}

static RECT mmove_rc;
static ui64 mx_old, my_old;		// Old values of mouse position
static i64 mover_ln;			// Mouse is hovering over this line
static i64 mover_ln_prev;		// Previous line that was hovered
static i64 min_sel = I64_MAX;	// Minimum selected line index
static i64 max_sel = I64_MIN;	// Maximum selected line index
wtxt f_to_open = MAX_PATH;

void goToPosition()
{
	ui64 pos = results[(ui64)mover_ln].pos;
	for(ui64 i = (ui64)mover_ln; i != UI64_MAX; --i)
	{
		if(results[i].is_f)
		{
			f_to_open = t2u16(results[i].line);
			m2npp(NPPM_DOOPEN, 0, f_to_open);
			ui64 line = m2scintilla(SCI_LINEFROMPOSITION, pos);
			nppGoToLine(line);
			m2scintilla(SCI_SETSEL, pos, pos + ~pattern);
			return;
		}
	}
}

inline void redrawMouseHov(ui64 ln)
{
	mmove_rc.right = (LONG)clw;
	mmove_rc.top = (LONG)((ln - y_scrl) * cellh);
	mmove_rc.bottom = (LONG)(mmove_rc.top + cellh);
	InvalidateRect(main_wnd, &mmove_rc, FALSE);
}

static LRESULT wmDestoryHandler()
{
	DeleteObject(default_font);
	//DeleteObject(backgrnd);
	PostQuitMessage(0);
	return 0;
}

static LRESULT wmKeyUpHandler(WPARAM wp)
{
	switch(wp)
	{
	case VK_ESCAPE:
		DestroyWindow(main_wnd);
		wmDestoryHandler();
		break;
	default:
		break;
	}

	return 0;
}

static LRESULT wmVscrollHandler(HWND wnd, WPARAM wp)
{
	static SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(wnd, SB_VERT, &si);
	y_scrl = (ui64)si.nPos;

	switch(LW(wp))
	{
	case SB_TOP: // User clicked the HOME keyboard key
		si.nPos = si.nMin;
		break;
	case SB_BOTTOM: // User clicked the END keyboard key
		si.nPos = si.nMax;
		break;
	case SB_LINEUP: // User clicked the top arrow
		--si.nPos;
		break;
	case SB_LINEDOWN: // User clicked the bottom arrow
		++si.nPos;
		break;
	case SB_PAGEUP: // User clicked the scroll bar shaft above the scroll box
		si.nPos -= si.nPage;
		break;
	case SB_PAGEDOWN: // User clicked the scroll bar shaft below the scroll box
		si.nPos += si.nPage;
		break;
	case SB_THUMBPOSITION: // User released the scroll box
		break;
	case SB_THUMBTRACK: // User is dragging the scroll box
		//p|"SB_THUMBTRACK:"|HW(wp)|" si.nTrackPos: "|si.nTrackPos|N;
		si.nPos = si.nTrackPos; // 32-bit scroll position instead of 16-bit HW(wp)!
		break;
	default:
		break;
	}

	// Due to adjustments by Windows pos may not be the same as the value set
	si.fMask = SIF_POS;
	SetScrollInfo(wnd, SB_VERT, &si, TRUE);
	GetScrollInfo(wnd, SB_VERT, &si);

	if((ui64)si.nPos != y_scrl)
	{
		ScrollWindow(wnd, 0, ((i32)y_scrl - si.nPos) * (i32)cellh, NULL, NULL);
		UpdateWindow(wnd);
	}

	return 0;
}

static LRESULT wmHscrollHandler(HWND wnd, WPARAM wp)
{
	static SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(wnd, SB_HORZ, &si);
	x_scrl = (ui64)si.nPos;

	switch(LW(wp))
	{
	case SB_LINELEFT: // User clicked the left arrow
		--si.nPos;
		break;
	case SB_LINERIGHT: // User clicked the right arrow
		++si.nPos;
		break;
	case SB_PAGELEFT: // User clicked the scroll bar shaft left of the scroll box
		si.nPos -= si.nPage;
		break;
	case SB_PAGERIGHT: // User clicked the scroll bar shaft right of the scroll box
		si.nPos += si.nPage;
		break;
	case SB_THUMBTRACK: // User is dragging the scroll box
		//p|"SB_THUMBTRACK:"|HW(wp)|" si.nTrackPos: "|si.nTrackPos;
		si.nPos = si.nTrackPos; // 32-bit scroll position instead of 16-bit HW(wp)!
		break;
	default:
		break;
	}

	// Due to adjustments by Windows pos may not be the same as the value set
	si.fMask = SIF_POS;
	SetScrollInfo(wnd, SB_HORZ, &si, TRUE);
	GetScrollInfo(wnd, SB_HORZ, &si);

	if((ui64)si.nPos != x_scrl)
	{
		ScrollWindow(wnd, ((i32)x_scrl - si.nPos) * (i32)cellw, 0, NULL, NULL);
		UpdateWindow(wnd);
	}

	return 0;
}

static LRESULT wmMouseWheelHandler(HWND wnd, WPARAM wp)
{
	static i16 wh_d; // Wheel delta
	static SCROLLINFO si;
	wh_d = (i16)HW(wp);
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;

	i32 sbar, scroll_num;
	if(LW(wp) == MK_SHIFT) // Horizontal mode
	{
		sbar = SB_HORZ, scroll_num = (i32)mwheel_scroll_ch;
	}
	else // Vertical mode
	{
		sbar = SB_VERT, scroll_num = (i32)mwheel_scroll_ln;
	}

	GetScrollInfo(wnd, sbar, &si);
	y_scrl = (ui64)si.nPos;
	if(wh_d < 0)
	{
		si.nPos += (i32)(-wh_d/WHEEL_DELTA * scroll_num);
	}
	else
	{
		si.nPos -= (i32)(wh_d/WHEEL_DELTA * scroll_num);
	}
	SetScrollInfo(wnd, sbar, &si, TRUE);
	ScrollWindow(wnd, ((i32)y_scrl - si.nPos) * (i32)cellh, 0, NULL, NULL);
	UpdateWindow(wnd);

	return 0;
}

static LRESULT wmMouseMoveHandler(WPARAM wp, LPARAM lp)
{
	//i32 x = (i32)LW(lp);
	i32 y = HWSIGN(lp);

	if(wp == MK_RBUTTON)
	{
		mover_ln = y/(i64)cellh + (i64)y_scrl;

		if(mover_ln < min_sel)
		{
			min_sel = mover_ln;
		}
		if(mover_ln > max_sel)
		{
			max_sel = mover_ln;
		}

		redrawMouseHov((ui64)mover_ln);
	}
	else if(wp == MK_LBUTTON)
	{
		mover_ln = y/(i64)cellh + (i64)y_scrl;
		min_sel = mover_ln, max_sel = mover_ln;
		redrawMouseHov((ui64)mover_ln);
		redrawMouseHov((ui64)mover_ln_prev);
		mover_ln_prev = mover_ln;
	}

	return 0;
}

static PAINTSTRUCT ps;
static SCROLLINFO si;
static FindResults &l = results;
static FindResult tabbed_res;
static const txt one_tab_sps = L("    ");
static txt tab_sp;

LRESULT wndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_PAINT:
	{

		main_dc = BeginPaint(wnd, &ps); // BEGIN PAINT ------------------------------

		SelectObject(main_dc, default_font);

		if(ps.rcPaint.right == 0 && ps.rcPaint.bottom == 0)
		{
			EndPaint(wnd, &ps);
			return 0;
		}

		SetBkColor(main_dc, def_back);
		SetTextColor(main_dc, def_front);

		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetScrollInfo(wnd, SB_VERT, &si); // Update vertical scroll position
		y_scrl = (ui64)si.nPos;
		GetScrollInfo(wnd, SB_HORZ, &si); // Update horizontal scroll position
		x_scrl = (ui64)si.nPos;

		ui64 vis_ln_beg = y_scrl + ps.rcPaint.top/cellh;
		ui64 vis_ln_end = y_scrl + ps.rcPaint.bottom/cellh;

		for(ui64 i = vis_ln_beg; i <= vis_ln_end && i < ~l; ++i)
		{
			i32 x = -((i32)x_scrl * (i32)cellw);
			i32 y = (i32)((i - y_scrl) * cellh);

			if(y < ps.rcPaint.top || y > ps.rcPaint.bottom)
			{
				continue;
			}
			
			if((i64)i >= min_sel && (i64)i <= max_sel)
			{
				SetBkColor(main_dc, hov_back);
			}
			else
			{
				SetBkColor(main_dc, def_back);
			}

			if(l[i].is_f) // This line is a file path
			{
				SetTextColor(main_dc, file_path_c);
				TextOutA(main_dc, x, y, l[i].line, (i32)~l[i].line);
				SetTextColor(main_dc, def_front);
				continue;
			}

			FindResult *fr = &l[i];

			ui64 tab = txtf(l[i].line, 0, '\t');
			if(tab != NFND)
			{
				fr = &tabbed_res;
				fr->line = l[i].line;
				fr->idx = l[i].idx;
				fr->p1 = l[i].p1;
				fr->p2 = l[i].p2;
				
				do
				{
					ui64 tab_e = tab;
					while(fr->line[tab_e] == '\t')
					{
						++tab_e;
					}
					--tab_e;

					ui64 tabn = tab_e - tab + 1;
					if(tab_e < (ui64)(fr->idx + fr->p1)) // Tabs was in part 1 of line
					{
						fr->p1 += (ui16)(tabn * 3); // Each tab is replaced by 4 spaces, so 3 addional chars added
					}
					else
					{
						fr->p2 += (ui16)(tabn * 3);
					}
			
					for(ui64 j = 0; j < tabn; ++j)
					{
						tab_sp += one_tab_sps;
					}

					txtr(fr->line, tab, tabn, tab_sp);
					tab = txtf(fr->line, tab_e, '\t');
					txtclr(tab_sp);
				}
				while(tab != NFND);
			}

			const char *ln = (const char *)fr->line;

			SetTextColor(main_dc, index_c);
			TextOutA(main_dc, x, y, ln, fr->idx); // Draw index
			SetTextColor(main_dc, def_front);
			x += (i32)((fr->idx) * cellw), ln += fr->idx;
			TextOutA(main_dc, x, y, ln, fr->p1); // Draw 1st part of line
			SetTextColor(main_dc, pattern_c);
			x += (i32)((fr->p1) * cellw), ln += fr->p1;
			TextOutA(main_dc, x, y, ln, (i32)~pattern); // Draw pattern
			SetTextColor(main_dc, def_front);
			x += (i32)(~pattern * cellw), ln += ~pattern;
			TextOutA(main_dc, x, y, ln, fr->p2); // Draw 2nd part of line
		}

		EndPaint(wnd, &ps); // END PAINT --------------------------------------------
	}	return 0;
	case WM_CREATE:
		updateScrollState();
		return 0;
	case WM_SIZE:
		clw = LW(lp);
		clh = HW(lp);

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = (i32)~results - 1;
		si.nPage = (UINT)(clh/cellh);
		SetScrollInfo(wnd, SB_VERT, &si, TRUE);

		si.nMax = (i32)max_line_l - 1;
		si.nPage = (UINT)(clw/cellw);
		SetScrollInfo(wnd, SB_HORZ, &si, TRUE);
		return 0;
	case WM_VSCROLL:
		return wmVscrollHandler(wnd, wp);
	case WM_HSCROLL:
		return wmHscrollHandler(wnd, wp);
	case WM_KEYDOWN:
		return 0;
	case WM_KEYUP:
		return wmKeyUpHandler(wp);
	case WM_RBUTTONDOWN:
		return wmMouseMoveHandler(wp, lp);
	case WM_RBUTTONUP:
		clipLines(min_sel, max_sel);
		mmove_rc.right = (LONG)clw;
		mmove_rc.top = (LONG)(min_sel * cellh);
		mmove_rc.bottom = (LONG)(max_sel * cellh + cellh);
		InvalidateRect(main_wnd, &mmove_rc, FALSE);
		min_sel = I64_MAX;
		max_sel = I64_MIN;
		return 0;
	case WM_LBUTTONDOWN:
		mover_ln = HWSIGN(lp)/(i64)cellh + (i64)y_scrl;
		min_sel = mover_ln, max_sel = mover_ln;
		redrawMouseHov((ui64)mover_ln);
		return 0;
	case WM_LBUTTONUP:
		min_sel = I64_MAX;
		max_sel = I64_MIN;
		goToPosition();
		redrawMouseHov((ui64)mover_ln);
		redrawMouseHov((ui64)mover_ln_prev);
		return 0;
	case WM_MOUSEWHEEL:
		return wmMouseWheelHandler(wnd, wp);
	case WM_MOUSEMOVE:
		return wmMouseMoveHandler(wp, lp);
	case WM_CLOSE:
		DestroyWindow(wnd);
		return 0;
	case WM_DESTROY:
		return wmDestoryHandler();
	default:
		return DefWindowProc(wnd, msg, wp, lp);
	}
}

static void initGui()
{
	exec_adress = GetModuleHandle(NULL);

	// Create Fake Window to get active monitor's DPI
	HWND wnd = CreateWindowEx(0, L"Button", 0, 0,
		0, 0, 0, 0, 0, 0, exec_adress, 0);
	dpi_scale = (float)GetDpiForWindow(wnd) / 96.0f;
	DestroyWindow(wnd);

	cellw = DPIS(cellw);
	cellh = DPIS(cellh);

	// Create Font ===========================================================================
	default_font = CreateFontA(
		(i32)cellh,				// [I] Height, in logical units
		0,						// [I] Average width, in logical units [0 -> closest match]
		0,						// [I] Angle, between the esc. vect. & the x-ax. of device
		0,						// [I] Angle, between char's base and the x-ax. of device
		0,						// [I] Weight of font in the range 0 - 1000 [0 -> default]
		FALSE,					// [I] Specifies an italic font if set to TRUE
		FALSE,					// [I] Specifies an underlined font if set to TRUE
		FALSE,					// [I] A strikeout font if set to TRUE
		DEFAULT_CHARSET,		// [I] The character set
		OUT_DEFAULT_PRECIS,		// [I] Output precision match h, w, orient., esc., pitch, type
		CLIP_DEFAULT_PRECIS,	// [I] Clipping precision how to clip chars part. outside clip.
		CLEARTYPE_QUALITY,		// [I] Output quality how carefully GDI match to physical font
		DEFAULT_PITCH,			// [I] Pitch and family of the font [chars/25.4mm of text]
		"Consolas");			// [I] Typeface name of the font
	// =========================================================================================

	SystemParametersInfoA(SPI_GETWHEELSCROLLCHARS, 0, &mwheel_scroll_ch, 0);
	SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &mwheel_scroll_ln, 0);
}

static HWND spawnMainWnd()
{
	DWORD style_ex = WS_EX_COMPOSITED;
	DWORD style = WS_THICKFRAME | WS_SYSMENU | WS_VSCROLL | WS_HSCROLL;

	if(main_class == 0)
	{
		HICON ico = LoadIcon(NULL, IDI_INFORMATION);
		backgrnd = CreateSolidBrush(def_back);

		// Register Window Class Atom ==============================================================
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);					// Structure size in bytes
		wc.style = CS_DBLCLKS;							// Class style, any combination
		wc.lpfnWndProc = wndProc;						// Window callback function
		wc.cbClsExtra = 0;								// Extra Class Memory (max 40B)
		wc.cbWndExtra = 0;								// Extra Window Memory (max 40B)
		wc.hInstance = exec_adress;						// Handle to module's instance
		wc.hIcon = ico;									// Def. icon for all windows
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);		// Def. cursor for all windows
		wc.hbrBackground = backgrnd;					// Def. brush for WM_ERASEBKGND
		wc.lpszMenuName = NULL;							// Def. menu name for all w.
		wc.lpszClassName = L"nppFindInFilesWnd";		// Name of the Class (Atom)
		wc.hIconSm = 0;									// Def. small icon for all w.

		main_class = RegisterClassEx(&wc);
		// =========================================================================================
	}

	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_pix, 0);
	i32 w = (i32)cellw * 140;
	i32 h = (i32)cellh * 30;
	i32 x = (work_pix.right - work_pix.left - w)/2;
	i32 y = work_pix.bottom - h;

	title = title_prefix + i2t(matches_found);

	// Create Window ===========================================================================
	main_wnd = CreateWindowExA(
		style_ex,				//   [I] Extended window style
		(LPCSTR)main_class,		// [I|O] Class Atom / Class Atom String
		title,					// [I|O] Window name String (Title)
		style,					//   [I] Window style (WS_OVERLAPPED = 0x0)
		x, y, w, h,				//   [I] PosX, PoxY, Width, Height
		npp_data._nppHandle,	// [I|O] Handle to this window's parent
		NULL,					// [I|O] Handle to menu / Child-window ID
		exec_adress,			// [I|O] Handle to instance of the module
		NULL);					// [I|O] CREATESTRUCT ptr. for lParam of WM_CREATE
	// =========================================================================================

	UpdateWindow(main_wnd);
	ShowWindow(main_wnd, SW_NORMAL);

	return main_wnd;
}

void resetFndState()
{
	matches_found = 0;
	results.Clear();
	y_scrl = 0, x_scrl = 0;
	max_line_l = 0;
	min_sel = I64_MAX;
	max_sel = I64_MIN;
}

#pragma warning( suppress : 4100 )
static DWORD NOTHROW findInFilesMsgThread(LPVOID lp)
{
	fnd_in_files_wnd_active = true;
	initGui();
	Sleep(5); // This might fix bugs when only window frame is spawned
	spawnMainWnd();

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}
	
	//UnregisterClass((LPCWSTR)main_class, exec_adress);
	resetFndState();
	fnd_in_files_wnd_active = false;
	return 0;
}

#define INC_DIRS_S 1
wtxt inc_dirs[] = {
	WL("D:\\P")
};

#define IGN_DIRS_S 3
wtxt ign_dirs[] = {
	WL("D:\\P\\O"),
	WL("D:\\P\\Android"),
	WL("D:\\P\\Git")
};

#define INC_TYPES_S 5
wtxt inc_types[INC_TYPES_S] = {
	WL(""),	// No extension
	WL(".cpp"),
	WL(".h"),
	WL(".c"),
	WL(".cmd")
};

#define IGN_TYPES 0
//wtxt ign_types[IGN_TYPES] = {
//	// Nothing...
//};

//bool64 skip_hidden = true;
//bool64 skip_hidden = false;
//bool64 ext_ignore_case = true;

wtxta files(4096);
wtxta dirs_up(inc_dirs, INC_DIRS_S);	// Upper level of directory tree
wtxta dirs_low(512);					// Lower level of directory tree

#define BUF_SIZE 0xA00000 // 10 485 760 B
txt file_buf = BUF_SIZE;

void getFileExt(const wtxt &fn, wtxt &ext)
{
	ui64 dot = txtfe(fn, TEND, '.');
	
	if(dot == NFND)
	{
		return;
	}

	ui64 sz = (~fn - dot);
	mcpy(ext, (const wchar_t *)fn + dot, sz * sizeof(wchar_t));
	txtssz(ext, sz);

	// No need to check, for now it will speed up the process by being hardcoded default
	//if(ext_ignore_case)
	//{
	t2low(ext);
	//}
}

wtxt star = WL("\\*");
wtxt fnd_all = MAX_PATH;
wtxt full_fn = MAX_PATH;
wtxt ign_dir = MAX_PATH;
wtxt fext = MAX_PATH;
wtxt fn;

void scan(const wtxt &dir)
{
	HANDLE ffind;
	WIN32_FIND_DATAW fdata;

	fnd_all += dir, fnd_all += star;

	// Start files enumeration =================================================================
	ffind = FindFirstFileExW(
		fnd_all,					// [I] LPCSTR Full			dir + path
		FindExInfoBasic,			// [I] FINDEX_INFO_LEVELS	Search accuracy
		&fdata,						// [O] LPVOID				Ouput file data
		FindExSearchNameMatch,		// [I] FINDEX_SEARCH_OPS	Non wildcard filters
		NULL,						// [I] LPVOID				Must be NULL (no support)
		FIND_FIRST_EX_LARGE_FETCH);	// [I] DWORD				Additional search flags
	// =========================================================================================

	if(ffind == INVALID_HANDLE_VALUE)
	{
		//p|PE|" In FindFirstFileExA!"|N;
		return;
	}

	do
	{
		txtsdt(fn, MAX_PATH, 0, fdata.cFileName); // Always set direct manipulated text back to 0!
		txtszu(fn);

		if((~fn == 1 && fn[0] == '.') || (~fn == 2 && fn[0] == '.' && fn[1] == '.'))
		{
			continue;
		}

		if(/*skip_hidden &&*/ fdata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		{
			continue;
		}

		full_fn += dir, full_fn += '\\', full_fn += fn;

		if(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			for(ui64 i = 0; i < IGN_DIRS_S; ++i) // Skip all ignored subdirectories
			{
				if(full_fn == ign_dirs[i])
				{
					goto skip_file;
				}

				txtclr(ign_dir);
			}

			dirs_low << full_fn;
		}
		else
		{
			getFileExt(fn, fext);

			bool64 to_skip = true;
			for(ui64 i = 0; i < INC_TYPES_S; ++i) // Include only selected file types
			{
				if(fext == inc_types[i])
				{
					to_skip = false;
					break;
				}
			}
			
			if(!to_skip)
			{
				files << full_fn;
			}
		}
		
	skip_file:
		txtclr(full_fn);
		txtclr(fext);
	} while(FindNextFileW(ffind, &fdata));
	FindClose(ffind);

	txtclr(fnd_all);
	txtsdt(fn, 0, 0, NULL); // Directly manipulated text set back to 0 before it goes out of scope
}

ui64a shifts = 256;
ui64a rel_shifts = 256;
txt fresult = 1024;
txt file_end = 512;
const txt pad3 = L("   ");
const txt pad2 = L("  ");
const txt pad1 = (txt)' ';

void selectPad(txt &t, ui64 i) // I don't think this can be done any faster...
{
	if(i < 10)
	{
		t = pad3;
	}
	else if(i < 100)
	{
		t = pad2;
	}
	else if(i < 1000)
	{
		t = pad1;
	}
	else
	{
		txtclr(t);
	}
}

void find(const wtxt &file)
{
	// Create or open File or Device =================================================================
	HANDLE f = CreateFile(
		file,						//   [I]  Name of the file or device to create/open
		GENERIC_READ,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_READ,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_EXISTING,				//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,	//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================

	if(f == INVALID_HANDLE_VALUE)
	{
		//p|PE|" Line "|__LINE__|'!'|N|P;
		return;
	}

	LARGE_INTEGER fsz;
	GetFileSizeEx(f, &fsz);

	const char *te, *tb;

	if(fsz.QuadPart > BUF_SIZE)
	{
		goto find_in_massive_file;
	}

	ReadFile(f, file_buf, BUF_SIZE, *file_buf, NULL);

	if(~file_buf < ~pattern)
	{
		CloseHandle(f); // Always check the handle!
		return;
	}

	tb = (const char *)file_buf;
	te = (const char *)file_buf + ~file_buf - ~pattern + 1;

	while(tb != te)
	{
		if(memeq(tb, pattern, ~pattern))
		{
			shifts << (ui64)(tb - (const char *)file_buf);
		}

		++tb;
	}

	if(shifts != empty)
	{
		result.is_f = true;
		result.pos = 0;
		result.line = wt2u8(file);
		pushLine();

		for(ui64 i = 0; i < ~shifts; ++i)
		{
			ui64 idx = i + 1;
			selectPad(fresult, idx);

			result.pos = shifts[i];
			fresult += i2t(idx);
			result.idx = (ui16)~fresult;
			fresult += ':', fresult += ' ';

			ui64 lbeg = shifts[i] == 0 ? 0 : txtfe(file_buf, shifts[i] - 1, '\n') + 1;
			result.p1 = (ui16)(shifts[i] - lbeg + 2); // +2 for [: ]
			ui64 last_nl = txtf(file_buf, shifts[i] + ~pattern, '\n');
			ui64 lend;

			if(last_nl == NFND)
			{
				lend = ~file_buf - 1;
			}
			else if(file_buf[last_nl] == '\r')
			{
				lend = last_nl - 1;
			}
			else
			{
				lend = last_nl;
			}

			result.p2 = (ui16)(lend - lbeg + 3 - result.p1 - ~pattern); // +1 i->size +2 [: ]
			result.is_f = false;
			fresult += txtsp(file_buf, lbeg, lend);
			
			result.line = fresult;

			pushLine();

			++matches_found;

			if(main_wnd != NULL)
			{
				title = title_prefix + i2t(matches_found);
				SetWindowTextA(main_wnd, title);
			}
		}

		shifts.Clear();
	}

	CloseHandle(f); // Always check the handle!
	return;

find_in_massive_file:

	ui64 i_pre = 0;
	ui64 chunks_read = 0;

	while(ReadFile(f, file_buf, BUF_SIZE, *file_buf, NULL) && ~file_buf)
	{
		if(file_end != empty)
		{
			txti(file_buf, 0, file_end);
		}

		if(~file_buf < ~pattern)
		{
			break;
		}

		tb = (const char *)file_buf;
		te = (const char *)file_buf + ~file_buf - ~pattern + 1;

		while(tb != te)
		{
			if(memeq(tb, pattern, ~pattern))
			{
				ui64 rel_shift = (ui64)(tb - (const char *)file_buf);
				rel_shifts << rel_shift;

				if(file_end != empty)
				{
					shifts << BUF_SIZE * chunks_read + rel_shift - (~pattern - 1);
				}
				else
				{
					shifts << rel_shift;
				}
			}

			++tb;
		}

		if(shifts != empty)
		{
			if(file_end == empty)
			{
				result.is_f = true;
				result.pos = 0;
				result.line = wt2u8(file);
				pushLine();
			}

			ui64 i = 0;
			for(; i < ~shifts; ++i)
			{
				ui64 idx = i_pre + i + 1;
				selectPad(fresult, idx);

				result.pos = shifts[i];
				fresult += i2t(idx);
				result.idx = (ui16)~fresult;
				fresult += ':', fresult += ' ';

				ui64 lbeg = rel_shifts[i] == 0 ? 0 : txtfe(file_buf, rel_shifts[i] - 1, '\n') + 1;
				result.p1 = (ui16)(rel_shifts[i] - lbeg + 2); // +2 for [: ]
				ui64 last_nl = txtf(file_buf, rel_shifts[i] + ~pattern, '\n');
				ui64 lend;
				
				if(last_nl == NFND)
				{
					lend = ~file_buf - 1;
				}
				else if(file_buf[last_nl] == '\r')
				{
					lend = last_nl - 1;
				}
				else
				{
					lend = last_nl;
				}

				result.p2 = (ui16)(lend - lbeg + 3 - result.p1 - ~pattern); // +1 i->size +2 [: ]
				result.is_f = false;
				fresult += txtsp(file_buf, lbeg, lend);

				result.line = fresult;

				pushLine();

				++matches_found;

				if(main_wnd != NULL)
				{
					title = title_prefix + i2t(matches_found);
					SetWindowTextA(main_wnd, title);
				}
			}

			i_pre += i;
			shifts.Clear();
			rel_shifts.Clear();
		}

		if(~file_buf < BUF_SIZE)
		{
			continue;
		}

		file_end = txts(file_buf, ~file_buf - ~pattern + 1, ~pattern-1);
		++chunks_read;
	}

	txtclr(file_end);
	CloseHandle(f); // Always check the handle!
}

#pragma warning( suppress : 4100 )
static DWORD NOTHROW findInFilesWorkerThread(LPVOID lp)
{
	while(~dirs_up != 0)
	{
		for(ui64 i = 0; i < ~dirs_up; ++i)
		{
			scan(dirs_up[i]);
		}

		dirs_low.MoveInto(dirs_up);
	}

	for(ui64 i = 0; i < ~files; ++i)
	{
		find(files[i]);
	}

	RedrawWindow(main_wnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE); // Redraw non-client area
	return 0;
}

void doFindInFiles(const txt &pat)
{
	pattern = pat;
	
	if(!fnd_in_files_wnd_active)
	{
		CreateThread(NULL, 0, findInFilesMsgThread, NULL, 0, NULL);
	}
	else
	{
		resetFndState();
	}

	CreateThread(NULL, 0, findInFilesWorkerThread, (LPVOID)&pat, 0, NULL);
}




// POSSIBLE TODO
//On Windows, the message-passing scheme used to communicate between the container and Scintilla is mediated by the operating system SendMessage
// function and can lead to bad performance when calling intensively.To avoid this overhead, Scintilla provides messages that allow you to call
// the Scintilla message function directly.The code to do this in C/C++ is of the form :
//
// #include "Scintilla.h"
// SciFnDirect pSciMsg = (SciFnDirect)SendMessage(hSciWnd, SCI_GETDIRECTFUNCTION, 0, 0);
// sptr_t pSciWndData = (sptr_t)SendMessage(hSciWnd, SCI_GETDIRECTPOINTER, 0, 0);
// 
// // now a wrapper to call Scintilla directly
// sptr_t CallScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
// 	return pSciMsg(pSciWndData, iMessage, wParam, lParam);
// }
// 
// SciFnDirect, SciFnDirectStatus, sptr_t anduptr_t are declared in Scintilla.h.hSciWnd is the window handle returned when you created the Scintilla window.
// 
// While faster, this direct calling will cause problems if performed from a different thread to the native thread of the Scintilla window
// in which case  SendMessage(hSciWnd, SCI_ *, wParam, lParam) should be used to synchronize with the window's thread.
// 
// This feature also works on other platforms but has less impact on speed.
// 
// On Windows, Scintilla exports a function called Scintilla_DirectFunction that can be used the same as the function returned
// by SCI_GETDIRECTFUNCTION.This  saves you the call to SCI_GETDIRECTFUNCTION andthe need to call Scintilla indirectly via the function pointer.




//static bool64 conalloced;
//if(!conalloced)
//{
//	AllocConsole();
//	FILE *s = freopen("CONIN$", "r", stdin);
//	s = freopen("CONOUT$", "w", stdout);
//	s = freopen("CONOUT$", "w", stderr);
//
//	HANDLE chand = GetStdHandle(STD_OUTPUT_HANDLE);
//	CONSOLE_CURSOR_INFO inf;
//	inf.dwSize = 100;
//	inf.bVisible = FALSE;
//	SetConsoleCursorInfo(chand, &inf);
//	conalloced = true;
//}

//#define NDBG
//#ifndef NDBG
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//char *big_buf = (char *)VirtualAlloc(0, 10000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//
//ui64 bbp = 0;
//for(ui64 i = 0; i < langs_s; ++i)
//{
//	bbp += sprintf(big_buf+bbp, "N[%s] C[%s] OBC[%s] CBC[%s]\n", langs[i].n, langs[i].c, langs[i].bcb, langs[i].bce);
//}
////////////////////////////////////// ASCII
//SIZE_T l = strlen(big_buf) + 1;
//HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, l);
//memcpy(GlobalLock(hmem), big_buf, l);
//GlobalUnlock(hmem);
//OpenClipboard(NULL);
//EmptyClipboard(); // Using this with OpenClipboard(NULL) should not work, but it does! Magick?
//SetClipboardData(CF_TEXT, hmem);
//CloseClipboard();
////////////////////////////////////// UNICODE
//wchar_t buf[100];
//wsprintf(buf, L"%d", id);
//
//SIZE_T l = wcslen(buf) + 1;
//HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, l * sizeof(wchar_t));
//memcpy(GlobalLock(hmem), buf, l * sizeof(wchar_t));
//GlobalUnlock(hmem);
//OpenClipboard(NULL);
//EmptyClipboard(); // Using this with OpenClipboard(NULL) should not work, but it does! Magick?
//SetClipboardData(CF_UNICODETEXT, hmem);
//CloseClipboard();
//
//VirtualFree(big_buf, 0, MEM_RELEASE);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//#ifndef NDBG
//txtp tp;
//tp|CL|'['|pos|']'|N|'['|line_num|']'|N;
//MessageBoxA(NULL, *tp, DBG_LINE, MB_OK);
//#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
//char buf[500];
//sprintf(buf, "bcomm[%s] ctag[%s]", bcomm, ctag_probe);
//#ifndef NDBG
//MessageBoxA(NULL, buf, "DEBUG", MB_OK); ///////////////////////////////////////
//#endif
//#undef NDBG

//////////////////////////////////////
// 
//
//DWORD __declspec(nothrow) msgBoxThread(LPVOID lp)
//{
//	//MessageBoxAA(NULL, (const char *)lp, "DEBUG MSG", MB_OK);
//	VirtualFree(lp, 0, MEM_RELEASE);
//	return 0;
//}
//
//
//void asyncMsgBox(const char *msg)
//{
//	ui64 s = strlen(msg);
//	char *m = (char *)VirtualAlloc(0, s+1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//	memcpy(m, msg, s+1);
//	CreateThread(NULL, 0, msgBoxThread, (LPVOID)m, NULL, NULL);
//}
// 
// 
//////////////////////////////////////





// ONLY WORKS WITH NORMAL TEXT I.E. NO LEXER ENABLED!!!!!!!!!!/////////////////////////////////////////////
	//updateScintillaHwnd();
	//
	//
	//ui64 pos = m2scintilla(SCI_GETSELECTIONEND);
	//
	//static ui64 i = 32;
	//
	////m2scintilla(SCI_STYLESETFORE, i, 0xFFFFFFFF);
	//
	////ui64 endstylepos = m2scintilla(SCI_STYLEGETFORE);
	//
	//ui64 endstylepos = m2scintilla(SCI_GETENDSTYLED);
	//ui64 lexer = m2scintilla(SCI_GETLEXER);
	//
	////m2scintilla(SCI_SETILEXER, 0, 1);
	//ui32 lang_t = L_TEXT;
	//m2npp(NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&lang_t);
	//m2npp(NPPM_SETCURRENTLANGTYPE, 0, L_TEXT);
	//
	//printf("F[%llX] B[%llX] EP[%lld] LI[%lld] I%lld\r", m2scintilla(SCI_STYLEGETFORE, i), m2scintilla(SCI_STYLEGETBACK, i), endstylepos, lexer, i);
	//
	//m2scintilla(SCI_STARTSTYLING, pos);
	//m2scintilla(SCI_SETSTYLING, 5, i++);
	//
	//
	////m2npp(NPPM_SETCURRENTLANGTYPE, 0, lang_t);
	//
	//
	////m2scintilla(SCI_SETILEXER, 0, lexer);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////