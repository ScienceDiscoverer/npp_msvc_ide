// SLIB
#include <txta>
#include <math>
#include <clip>
#include <wtxtp>

namespace winapi
{
	#include <stdlib.h>
}

#include "sddb"

#pragma comment( lib, "user32")
#pragma comment( lib, "gdi32" )

#define SCI_GOTOLINE				2024 // (line line)
#define SCI_SETSELECTIONSTART		2142 // (position anchor)
#define SCI_SETSELECTIONEND			2144 // (position caret)
#define SCI_POSITIONFROMLINE		2167 // (line line) → position
#define SCI_VISIBLEFROMDOCLINE		2220 // (line docLine) → line
#define SCI_GETLINEVISIBLE			2228 // (line line) → bool
#define SCI_ENSUREVISIBLE			2232 // (line line)
#define SCI_LINELENGTH				2350 // (line line) → position
#define SCI_LINESONSCREEN			2370 // → line
#define SCI_SETFIRSTVISIBLELINE		2613 // (line displayLine)

#define NPPMSG				(WM_USER + 1000)
#define NPPM_MENUCOMMAND	(NPPMSG + 48)		// void NPPM_MENUCOMMAND(0, int cmdID)

#define    IDM    							40000
#define    IDM_SEARCH    					(IDM + 3000)
#define    IDM_SEARCH_UNMARKALLEXT5			(IDM_SEARCH + 31)
#define    IDM_SEARCH_MARKONEEXT5			(IDM_SEARCH + 66)

#ifdef UNICODE
#define TOWSTR(x) L ## x
#else
#define TOWSTR(x) x
#endif

#define APP_GUID "-{AAA6D28F-DF14-4570-A273-21DD2708F5AD}"
#define REGISTER_MESSAGE(n) \
	static const UINT n = RegisterWindowMessage(TOWSTR(#n APP_GUID))

#define HW(x) (((unsigned int)x) >> 16)
#define LW(x) (((unsigned int)x) & 0xFFFF)
#define HWSIGN(x) ((int)(short)HW(x))
#define LWSIGN(x) ((int)(short)LW(x))
#define DPIS(x) ((ui64)((double)x * dpi_scale)) // Scale value for DPI

// For txt/wtxt debugging:
//{DWORD bw;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "ns " + i2t(ns), (DWORD)~("ns " + i2t(ns)), &bw, NULL);}

/*
DEBUGGING RANDOM HEAP CORRUPTION CRUSH

When I see a call stack that looks like yours the most common cause is heap corruption. A double free or attempting to free a pointer that was never allocated can have similar call stacks. Since you characterize the crash as inconsistent that makes heap corruption the more likely candidate. Double frees and freeing unallocated pointers tend to crash consistently in the same place. To hunt down issues like this I usually:

    Install Debugging Tools for Windows
    Open a command prompt with elevated privileges
    Change directory to the directory that Debugging Tools for Windows is installed in. C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\gflags.exe
    Enable full page heap by running gflags.exe -p /enable applicationName.exe /full
    Launch application with debugger attached and recreate the issue.
    Disable full page heap for the application by running gflags.exe -p /disable applicationName.exe

Running the application with full page heap places an inaccessible page at the end of each allocation so that the program stops immediately if it accesses memory beyond the allocation. This is according to the page GFlags and PageHeap. If a buffer overflow is causing the heap corruption this setting should cause the debugger to break when the overflow occurs..

Make sure to disable page heap when you are done debugging. Running under full page heap can greatly increase memory pressure on an application by making every heap allocation consume an entire page.
*/

/*
EXCEPTION_ACCESS_VIOLATION
	The thread tried to read from or write to a virtual address for which it does not have the appropriate access.

EXCEPTION_ARRAY_BOUNDS_EXCEEDED
	The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.

EXCEPTION_BREAKPOINT
	A breakpoint was encountered.

EXCEPTION_DATATYPE_MISALIGNMENT
	The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.

EXCEPTION_FLT_DENORMAL_OPERAND
	One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.

EXCEPTION_FLT_DIVIDE_BY_ZERO
	The thread tried to divide a floating-point value by a floating-point divisor of zero.

EXCEPTION_FLT_INEXACT_RESULT
	The result of a floating-point operation cannot be represented exactly as a decimal fraction.

EXCEPTION_FLT_INVALID_OPERATION
	This exception represents any floating-point exception not included in this list.

EXCEPTION_FLT_OVERFLOW
	The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.

EXCEPTION_FLT_STACK_CHECK
	The stack overflowed or underflowed as the result of a floating-point operation.

EXCEPTION_FLT_UNDERFLOW
	The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.

EXCEPTION_ILLEGAL_INSTRUCTION
	The thread tried to execute an invalid instruction.

EXCEPTION_IN_PAGE_ERROR
	The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.

EXCEPTION_INT_DIVIDE_BY_ZERO
	The thread tried to divide an integer value by an integer divisor of zero.

EXCEPTION_INT_OVERFLOW
	The result of an integer operation caused a carry out of the most significant bit of the result.

EXCEPTION_INVALID_DISPOSITION
	An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.

EXCEPTION_NONCONTINUABLE_EXCEPTION
	The thread tried to continue execution after a noncontinuable exception occurred.

EXCEPTION_PRIV_INSTRUCTION
	The thread tried to execute an instruction whose operation is not allowed in the current machine mode.

EXCEPTION_SINGLE_STEP
	A trace trap or other single-instruction mechanism signaled that one instruction has been executed.

EXCEPTION_STACK_OVERFLOW
	The thread used up its stack.
	
Data Execution Prevention (DEP) is a Microsoft security feature that monitors and protects certain pages or regions of memory, preventing them from executing (usually malicious) code. When DEP is enabled, all data regions are marked as non-executable by default.
*/

//--------------------------------------------------------------------
// CPP_EXCEPTION
// PARAMS NUM: 4
// PARAM0        SOME INTERNAL VALUE: 0x19930520
// PARAM1      POINTER TO THROWN OBJ: 0x14FA30
// PARAM2      POINTER TO OBJECT INF: 0x7FFE68B246B0 ---->
// PARAM3  DLL/EXE THROWER HINSTANCE: 0x7FFE68AC0000
//      +-----+
//  --> |0    |
//      +-----+
// 	 |4D0C |
//      +-----+
// 	 |0    |
//      +-----+     +-----+
//      |646D0|---->|3    |
//      +-----+     +-----+     +-----+
// 	             |646F0|---->|0    |
//                  +-----+     +-----+     +----------------+
//                              |84578|---->|0               |
//                              +-----+     +----------------+
//                                          |7FFE68B1C218    |
//                                          +----------------+
//                                          |5F74756F56413F2E|
//                                          +----------------+
//                                     ..... _ t u o V A ? .
// 									
// .?AVout_of_range@std@@
// CURRENT EXE HIN:  0x0000000140000000
// UCRTBASE HIN:     0x00007FFE99D50000
// VCRUNTIME140 HIN: 0x00007FFE95040000
// STD MSVCP140 HIN: 0x00007FFE68AC0000
// CONTINUE EXECUTION:  NOT POSSIBLE
// INSTRUCTION ADDRESS: 0x00007FFE99F54FD9
//--------------------------------------------------------------------

void pexcept(_EXCEPTION_RECORD *er, HANDLE proc)
{
	if(proc == NULL)
	{
		proc = GetCurrentProcess();
	}
	
	static HANDLE heap = GetProcessHeap();
	
	DWORD c = er->ExceptionCode;
	
	p|R|"<---"|D|N;
	p|"--------------------------------------------------------------------"|R|N;
	switch(c)
	{
	case STILL_ACTIVE:
		p|"STILL_ACTIVE"|N;
		break;
	case EXCEPTION_ACCESS_VIOLATION:
		if(er->ExceptionInformation[0] == 0)
		{
			p|"READ ";
		}
		else if(er->ExceptionInformation[0] == 1)
		{
			p|"WRITE ";
		}
		else if(er->ExceptionInformation[0] == 8)
		{
			p|"DEP "; // Data Execution Prevention
		}
		p|"ACCESS_VIOLATION"|N;
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		p|"DATATYPE_MISALIGNMENT"|N;
		break;
	case EXCEPTION_BREAKPOINT:
		p|"BREAKPOINT"|N;
		break;
	case EXCEPTION_SINGLE_STEP:
		p|"SINGLE_STEP"|N;
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		p|"ARRAY_BOUNDS_EXCEEDED"|N;
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		p|"FLOAT_DENORMAL_OPERAND"|N;
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		p|"FLOAT_DIVIDE_BY_ZERO"|N;
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		p|"FLOAT_INEXACT_RESULT"|N;
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		p|"FLOAT_INVALID_OPERATION"|N;
		break;
	case EXCEPTION_FLT_OVERFLOW:
		p|"FLOAT_OVERFLOW"|N;
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		p|"FLOAT_STACK_CHECK"|N;
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		p|"FLOAT_UNDERFLOW"|N;
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		p|"INTEGER_DIVIDE_BY_ZERO"|N;
		break;
	case EXCEPTION_INT_OVERFLOW:
		p|"INTEGER_OVERFLOW"|N;
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		p|"PRIVILEGED_INSTRUCTION"|N;
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		if(er->ExceptionInformation[0] == 0)
		{
			p|"READ ";
		}
		else if(er->ExceptionInformation[0] == 1)
		{
			p|"WRITE ";
		}
		else if(er->ExceptionInformation[0] == 8)
		{
			p|"DEP "; // Data Execution Prevention
		}
		p|"IN_PAGE_ERROR"|D|N;
		p|"DATA VADDRESS:       "|er->ExceptionInformation[1]|N;
		p|"NTSTATUS CAUSE:      "|er->ExceptionInformation[2]|N;
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		p|"ILLEGAL_INSTRUCTION"|N;
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		p|"NONCONTINUABLE_EXCEPTION"|N;
		break;
	case EXCEPTION_STACK_OVERFLOW:
		p|"STACK_OVERFLOW"|N;
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		p|"INVALID_DISPOSITION"|N;
		break;
	case EXCEPTION_GUARD_PAGE:
		p|"GUARD_PAGE_VIOLATION"|N;
		break;
	case EXCEPTION_INVALID_HANDLE:
		p|"INVALID_HANDLE"|N;
		break;
	case EXCEPTION_HEAP_CORRUPTION:
		p|"HEAP_CORRUPTION"|N;
		break;
	// case EXCEPTION_POSSIBLE_DEADLOCK:
		// p|"POSSIBLE_DEADLOCK"|N;
		// break;
	case CONTROL_C_EXIT:
		p|"CONTROL_C_EXIT"|N;
		break;
	case EXCEPTION_CPP_LOWERCASE:
	case EXCEPTION_CPP_UPPERCASE:
	{
		p|"CPP_EXCEPTION"|D|N;
		p|"PARAMS NUM: "|er->NumberParameters|N;
		for(ui64 i = 0; i < er->NumberParameters; ++i)
		{
			p|"PARAM"|SP(2)|i|DP|" ";
			switch(i)
			{
			case 0:
				p|"      SOME INTERNAL VALUE";
				break;
			case 1:
				p|"    POINTER TO THROWN OBJ";
				break;
			case 2:
				p|"    POINTER TO OBJECT INF";
				break;
			case 3:
				p|"DLL/EXE THROWER HINSTANCE";
				break;
			}
			
			p|": 0x"|H|er->ExceptionInformation[i]|N;
		}
		
		char *class_name = NULL;
		__try
		{
			ui64 hinst = er->ExceptionInformation[3];
			DWORD *obj_inf = (DWORD *)er->ExceptionInformation[2];
			//p|H|obj_inf[0]|N;
			//p|obj_inf[1]|N;
			//p|obj_inf[2]|N;
			//p|obj_inf[3]|N;
			// p|"-----------------------------"|N;
			obj_inf = (DWORD *)(hinst + obj_inf[3]);
			// p|obj_inf[0]|N;
			// p|obj_inf[1]|N;
			// p|"-----------------------------"|N;
			obj_inf = (DWORD *)(hinst + obj_inf[1]);
			// p|obj_inf[0]|N;
			// p|obj_inf[1]|N;
			// p|"-----------------------------"|N;
			ui64 *class_inf = (ui64 *)(hinst + obj_inf[1]);
			// p|class_inf[0]|N;
			// p|class_inf[1]|N;
			// p|class_inf[2]|N;
			class_name = (char *)(class_inf + 2);
		}
		__except(1)
		{
			class_name = (char *)"CRASH WHILE TRYING TO GET!";
		}
		
		p|"CPP EXPT CLASS NAME: "|class_name|H|N;
		p|"CURRENT EXE HIN:     "|"0x"|H|GetModuleHandle(NULL)|N;
		p|"UCRTBASE HIN:        "|"0x"|H|LoadLibrary(L"ucrtbase.dll")|N;
		p|"VCRUNTIME140 HIN:    "|"0x"|H|LoadLibrary(L"vcruntime140.dll")|N;
		p|"STD MSVCP140 HIN:    "|"0x"|H|LoadLibrary(L"msvcp140.dll")|N;
	}	break;
	// CUSTOM EXCEPTIONS ----------------------------------------------------------------------------
	case EXCEPTION_TXT_BUFF_OVERRUN:
	{
		p|"EXCEPTION_TXT_BUFF_OVERRUN"|D|N;
		
		void *buff = NULL;
		if(er->ExceptionInformation[1] == EXCEPT_TXT_UTF8)
		{
			SIZE_T s_orig = er->ExceptionInformation[2] + 1, s;
			
			bool64 size_trunced = false;
			if(s_orig > 1024)
			{
				s = 1025;
				size_trunced = true;
			}
			else
			{
				s = s_orig;
			}
			
			buff = HeapAlloc(heap, 0, s);
			ReadProcessMemory(proc, (LPCVOID)er->ExceptionInformation[0], buff, s, NULL);
			char * char_buff = (char *)buff;
			char_buff[s-1] = 0;
			p|"   "|V|I|Y|"TEXT"|V|I|D|": "|V|I|D|char_buff|V|I|D;
			if(size_trunced)
			{
				p|"...... "|s_orig - s|" CHARACTERS TRUNCATED ......";
			}
			p|N;
		}
		else
		{
			SIZE_T s_orig = (er->ExceptionInformation[2] + 1), s;
			
			bool64 size_trunced = false;
			if(s_orig > 1024)
			{
				s = 1025;
				size_trunced = true;
			}
			else
			{
				s = s_orig;
			}
			
			buff = HeapAlloc(heap, 0, s * sizeof(wchar_t));
			ReadProcessMemory(proc, (LPCVOID)er->ExceptionInformation[0], buff, s * sizeof(wchar_t), NULL);
			
			wchar_t * wchar_buff = (wchar_t *)buff;
			wchar_buff[s-1] = 0;
			
			p|"   "|V|I|Y|"TEXT"|V|I|D|": "|V|I|D|wchar_buff|V|I|D;
			if(size_trunced)
			{
				p|"...... "|s_orig - s|" CHARACTERS TRUNCATED ......";
			}
			p|N;
		}
		HeapFree(heap, 0, buff);
		
		p|Y|"TEXT SIZE: "|YD|er->ExceptionInformation[2]|D|N;
		p|Y|" POSITION: "|YD|er->ExceptionInformation[3]|D|N;
	}	break;
	case EXCEPTION_T2I_NON_NUMBER:
	case EXCEPTION_H2I_NON_NUMBER:
	case EXCEPTION_T2I_OVERFLOW:
	case EXCEPTION_H2I_OVERFLOW:
	{
		if(c == EXCEPTION_T2I_NON_NUMBER)
		{
			p|"EXCEPTION_T2I_NON_NUMBER";
		}
		else if(c == EXCEPTION_H2I_NON_NUMBER)
		{
			p|"EXCEPTION_H2I_NON_NUMBER";
		}
		else if(c == EXCEPTION_T2I_OVERFLOW)
		{
			p|"EXCEPTION_T2I_OVERFLOW";
		}
		else
		{
			p|"EXCEPTION_H2I_OVERFLOW";
		}
		
		p|D|N;
		
		void *buff = NULL;
		if(er->ExceptionInformation[1] == EXCEPT_TXT_UTF8)
		{
			SIZE_T s = er->ExceptionInformation[2] + 1;
			buff = HeapAlloc(heap, 0, s);
			ReadProcessMemory(proc, (LPCVOID)er->ExceptionInformation[0], buff, s, NULL);
			p|V|I|Y|"TEXT"|V|I|D|": "|V|I|D|(const char *)buff|V|I|D|N;
		}
		else
		{
			SIZE_T s = (er->ExceptionInformation[2] + 1) * sizeof(wchar_t);
			buff = HeapAlloc(heap, 0, s);
			ReadProcessMemory(proc, (LPCVOID)er->ExceptionInformation[0], buff, s, NULL);
			
			wchar_t *t = (wchar_t *)buff - 1;
			wchar_t *te = (wchar_t *)buff + s/2;
			while(++t != te)
			{
				if(*t > 0xFF)
				{
					*t = '@'; // Stops console output from breaking up
				}
			}
			
			p|V|I|Y|"TEXT"|V|I|D|": "|V|I|D|(const wchar_t *)buff|V|I|D|N;
		}
		HeapFree(heap, 0, buff);
	}	break;
	case EXCEPTION_TXTDP_P1_LESS_P0:
		p|"EXCEPTION_TXTDP_P1_LESS_P0"|D|N;
		goto p1_less_then_p0_pos_display;
	case EXCEPTION_TXTSP_P1_LESS_P0:
		p|"EXCEPTION_TXTSP_P1_LESS_P0"|D|N;
		goto p1_less_then_p0_pos_display;
	case EXCEPTION_TXTRP_P1_LESS_P0:
		p|"EXCEPTION_TXTRP_P1_LESS_P0"|D|N;
	p1_less_then_p0_pos_display:
		p|"POSITION 0: "|er->ExceptionInformation[0]|N;
		p|"POSITION 1: "|er->ExceptionInformation[1]|N;
		break;
	case EXCEPTION_DARR_OUT_OF_RANGE:
		p|"EXCEPTION_DARR_OUT_OF_RANGE"|N;
		break;
	case EXCEPTION_BINF_OUT_OF_RANGE:
		p|"EXCEPTION_BINF_OUT_OF_RANGE"|N;
		break;
	case EXCEPTION_TXTA_OUT_OF_RANGE:
		p|"EXCEPTION_TXTA_OUT_OF_RANGE"|N;
		break;
	case EXCEPTION_WTXTA_OUT_OF_RANGE:
		p|"EXCEPTION_WTXTA_OUT_OF_RANGE"|N;
		break;
	case EXCEPTION_UI64A_OUT_OF_RANGE:
		p|"EXCEPTION_UI64A_OUT_OF_RANGE"|N;
		break;
	case EXCEPTION_ASSERT_FAIL:
		p|"EXCEPTION_ASSERT_FAIL "|D|"AT LINE "|C|er->ExceptionInformation[0]|R|N;
		break;
	case EXCEPTION_MATH_MPOW_OVERFLOW:
		p|"EXCEPTION_MATH_MPOW_OVERFLOW"|N;
		break;
	default:
		p|"UNKNOWN_EXCEPTION ["|H|c|"]"|N;
		break;
	}
	
	p|D|"CONTINUE EXECUTION:  ";
	if(er->ExceptionFlags == EXCEPTION_NONCONTINUABLE)
	{
		p|R|"NOT ";
	}
	else
	{
		p|G;
	}
	
	p|"POSSIBLE"|D|N;
	
	p|"INSTRUCTION ADDRESS: 0x"|H|er->ExceptionAddress|N;
		
	if(er->ExceptionRecord != NULL)
	{
		p|"CHAINED EXCEPTION!"|N;
		pexcept(er->ExceptionRecord, proc);
	}
	else
	{
		p|"--------------------------------------------------------------------"|N;
	}
}

// Defined values for the exception filter expression
// EXCEPTION_EXECUTE_HANDLER      1
// EXCEPTION_CONTINUE_SEARCH      0
// EXCEPTION_CONTINUE_EXECUTION (-1)
int efilter(_EXCEPTION_POINTERS *ep)
{
	p|DC;
	pexcept(ep->ExceptionRecord, NULL);
	p|G|"C"|D|"ONTINUE | ";
	p|C|"H"|D|"ANDLE | ";
	p|R|"I"|D|"GNORE";
	//p|"C ----> TRY TO EXECUTIE FAULTY INSTRUCTION AGAIN"|N;
	//p|"H ----> ABORT TRY BLOCK AND EXECUTE HANDLER, CONTINUE AFTER THE BLOCK"|N;
	//p|"I ----> IGNORE EXCEPTION AND PASS IT TO OTHER\\DEFAULT HANDLER"|N;
	
	while(1)
	{
		DWORD cmod;
		GetConsoleMode(_dbg_std_in_, &cmod);
		SetConsoleMode(_dbg_std_in_, cmod & 0xFFFFFFF9); // ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)
		char ch;
		static DWORD bw;
		ReadFile(_dbg_std_in_, &ch, 1, &bw, NULL);
		SetConsoleMode(_dbg_std_in_, cmod);
		
		switch(ch)
		{
		case 'c':
		case 'C':
			return EXCEPTION_CONTINUE_EXECUTION;
		case 'h':
		case 'H':
			return EXCEPTION_EXECUTE_HANDLER;
		case 'i':
		case 'I':
			return EXCEPTION_CONTINUE_SEARCH;
		default:
			break;
		}
	}
}

LARGE_INTEGER file_log_fmove_dist;

LARGE_INTEGER sw_stime[SWMAX];
LARGE_INTEGER sw_etime[SWMAX];
ui64 sw_timing[SWMAX];
ui64 sw_timings[SWMAX];
ui64 sw_id;
bool sw_first_print = true;

SW_FUNC sw_fs[0x10];

txt sw2txt(ui64 i)
{
	if(sw_timings[i] == 0)
	{
		dbgmb(L("sw2txt recieved ZERO timings! SW ID: ") + i2t(i));
		return L("");
	}

	ui64 timing = sw_timing[i]/sw_timings[i];

	return ticks2txt(timing);
}

txt fsw2txt(ui64 timing, ui64 ttot)
{
	txtp text_perc;
	float perc = ttot ? (float)timing/(float)ttot * 100.0f : 0.0f;
	// My std printer lib does not support floats yet, (-_-) yes.. yes...
	text_perc|(ui64)perc|".";
	text_perc|ui64(perc * 100.0f - float((ui64)perc) * 100.0f)|"% ";
	
	txtp text;
	text|SPN(8)|*text_perc|ticks2txt(timing);

	return *text;
}

void swDiff(ui64 i_left, ui64 i_right)
{
	ui64 timing_l = sw_timing[i_left]/sw_timings[i_left];;
	ui64 timing_r = sw_timing[i_right]/sw_timings[i_right];
	
	
	if(timing_l == 0 || timing_r == 0)
	{
		p|'x'|0;
		return;
	}
	
	txtp tp;
	tp|'x';
	
	float perc = (timing_l > timing_r ? (float)timing_l/(float)timing_r : (float)timing_r/(float)timing_l);
	// My std printer lib does not support floats yet, (-_-) yes.. yes...
	tp|(ui64)perc|".";
	tp|SP(2)|SPC('0')|ui64(perc * 100.0f - float((ui64)perc) * 100.0f);
	
	p|TAC|(timing_l > timing_r ? G : R)|*tp|D;
}

HANDLE _dbg_std_in_ = GetStdHandle(STD_INPUT_HANDLE);
HANDLE _dbg_std_out_ = GetStdHandle(STD_OUTPUT_HANDLE);

ULONG_PTR _dbg_assert_line;

REGISTER_MESSAGE(NPP_PLUGIN_GET_SCINTILLA);
const wchar_t *dbg_plugin_msg_wnd_class = L"SdppNppPluginMsgWindow";

HWND dbg_nppw, dbg_scintilla, dbg_plugin_wnd;

txta marked_files;
txt dbg_cur_file;

void nppMarkLine(ui64 line)
{
	// Find exact coordinates/dimentions of the line
	LRESULT line_pos = SendMessage(dbg_scintilla, SCI_POSITIONFROMLINE, (WPARAM)line-1, 0);
	LRESULT line_size = SendMessage(dbg_scintilla, SCI_LINELENGTH, (WPARAM)line-1, 0);
	
	// Select the line
	SendMessage(dbg_scintilla, SCI_SETSELECTIONSTART, (WPARAM)line_pos, 0);
	SendMessage(dbg_scintilla, SCI_SETSELECTIONEND, WPARAM(line_pos + line_size), 0);
	
	// Style the line with Notepad++'s 5th Style
	SendMessage(dbg_nppw, NPPM_MENUCOMMAND, 0, IDM_SEARCH_MARKONEEXT5);
}

void dbgNppGoToLine(ui64 line)
{
	SendMessage(dbg_scintilla, SCI_ENSUREVISIBLE, (WPARAM)line-1, 0);
	SendMessage(dbg_scintilla, SCI_GOTOLINE, (WPARAM)line-1, 0);
	
	// Scroll window so that the line is in the middle of view
	LRESULT sl = SendMessage(dbg_scintilla, SCI_LINESONSCREEN, 0, 0);
	
	i64 scr_l = (i64)line - 1 - sl/2;
	i64 doc_vl = scr_l < 0 ? 0 : scr_l;
	
	i64 hidden_ls = 0;
	for(i64 i = (i64)line-1; i >= doc_vl; --i)
	{
		if(SendMessage(dbg_scintilla, SCI_GETLINEVISIBLE, (WPARAM)i, 0) == 0)
		{
			++hidden_ls;
		}
	}
	
	doc_vl -= hidden_ls;
	WPARAM first_doc_vl = doc_vl < 0 ? 0 : (WPARAM)doc_vl;
	WPARAM first_vis_vl = (WPARAM)SendMessage(dbg_scintilla, SCI_VISIBLEFROMDOCLINE, first_doc_vl, 0);
	
	SendMessage(dbg_scintilla, SCI_SETFIRSTVISIBLELINE, first_vis_vl, 0);
}

void updateNppInterface()
{
	if(dbg_scintilla == NULL)
	{
		dbg_plugin_wnd = FindWindowEx(HWND_MESSAGE, NULL, dbg_plugin_msg_wnd_class, NULL);
		dbg_scintilla = (HWND)SendMessage(dbg_plugin_wnd, NPP_PLUGIN_GET_SCINTILLA, 0, 0);
		dbg_nppw = GetParent(dbg_scintilla);
	}
}

void _dbgSelLine(ui64 ln)
{
	updateNppInterface();
	marked_files.AddUnique(dbg_cur_file);
	
	SendMessage(dbg_nppw, NPPM_MENUCOMMAND, 0, IDM_SEARCH_UNMARKALLEXT5);
	
	nppMarkLine(ln);
	dbgNppGoToLine(ln);
}

void _dbgSelLineNoMark(ui64 ln)
{
	updateNppInterface();
	dbgNppGoToLine(ln);
}

void _dbgHighlightLine(ui64 ln)
{
	updateNppInterface();
	marked_files.AddUnique(dbg_cur_file);
	
	nppMarkLine(ln);
}

void _dbgOpenFile(const char *fn)
{
	updateNppInterface();
	dbg_cur_file = cstr({ fn, strl(fn) });
	
	wtxt fname = t2u16(dbg_cur_file);
	static COPYDATASTRUCT data;
	data.dwData = 0;
	data.cbData = ((DWORD)~fname + 1) * sizeof(wchar_t);
	data.lpData = PVOID((const wchar_t *)fname);
	
	SendMessage(dbg_plugin_wnd, WM_COPYDATA, 0, (LPARAM)&data);
}

void _dbgCleanSelects()
{
	updateNppInterface();
	SendMessage(dbg_nppw, NPPM_MENUCOMMAND, 0, IDM_SEARCH_UNMARKALLEXT5);
}

void _dbgDeselectAll()
{
	updateNppInterface();
	
	for(ui64 i = 0; i < ~marked_files; ++i)
	{
		wtxt fname = t2u16(marked_files[i]);
		static COPYDATASTRUCT data;
		data.dwData = 0;
		data.cbData = ((DWORD)~fname + 1) * sizeof(wchar_t);
		data.lpData = PVOID((const wchar_t *)fname);
		
		SendMessage(dbg_plugin_wnd, WM_COPYDATA, 0, (LPARAM)&data);
		SendMessage(dbg_nppw, NPPM_MENUCOMMAND, 0, IDM_SEARCH_UNMARKALLEXT5);
	}
	
	marked_files.Clear();
}

void _dbgSetFocusToNpp()
{
	AllowSetForegroundWindow(ASFW_ANY);
	SetFocus(dbg_nppw);
	SetForegroundWindow(dbg_nppw);
}

BOOL NOTHROW dbgCleanSelects(DWORD ctrl_sig)
{
	if(ctrl_sig != CTRL_CLOSE_EVENT)
	{
		return FALSE;
	}
	
	_dbgCleanSelects();
	
	return FALSE; // Handlers are added in a list FILO. Def. first. Returning FALSE calls next in a list
}

void _dbgSetCleanSelectsHandler()
{
	SetConsoleCtrlHandler(dbgCleanSelects, TRUE);
}

i32 NOTHROW dbgCleanSelectsOnExit()
{
	_dbgCleanSelects();
	return 0;
}

struct DbgLogLine
{
	ui64 varp; // End of variable name, start of variable value
	wtxt line;
};

class DbgLogLines : public darr
{
public:
	DbgLogLines()
	{ 
		ts = 128;
		s = 0;
		d = (DbgLogLine *)Alloc(ts * sizeof(DbgLogLine));
		memset(d, 0, ts * sizeof(DbgLogLine)); // All texts invalidated
	}
	DbgLogLines(const DbgLogLines &o) = delete;

	~DbgLogLines()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			txtfree(d[i].line);
		}
		DARR_FREE(d);
	}

	DbgLogLine & operator[](ui64 i) { return d[i]; }
	const DbgLogLine & operator[](ui64 i) const { return d[i]; }

	DbgLogLines & operator<<(const DbgLogLine &c)
	{
		ui64 ns = s + 1;
		if(ts < ns)
		{
			d = (DbgLogLine *)ReAlloc(d, ns, sizeof(DbgLogLine));
			memset(&d[s], 0, (ts - s) * sizeof(DbgLogLine)); // All new texts invalidated
		}

		d[s] = c;
		s = ns;

		return *this;
	}

private:
	DbgLogLine *d;
};

static DbgLogLines lines;
txtp dbgp;

HANDLE soft_breakpoint_event = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE dbg_wnd_spawn_event = CreateEvent(NULL, TRUE, FALSE, NULL);

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
static COLORREF var_name_c = 0x55CCAF;
static COLORREF var_brak_c = 0xBA37CC;
static COLORREF separator_c = 0x49CC37;
static COLORREF bpoint_c = 0x1F0FC5;
static COLORREF bpoint_lc = 0xAF912B;

static ui64 cellw = 8;
static ui64 cellh = 16;

static ui64 clw, clh; // Client width/height

static ui64 y_scrl, x_scrl;

static ui64 max_line_l; // Maximum line length

bool64 dbg_wnd_active;

static LARGE_INTEGER var_ps, var_pe; // Variables print start/end times

static UINT mwheel_scroll_ln, mwheel_scroll_ch; // System values for mouse wheel scrolling

static i64 mx_old, my_old;		// Old values of mouse position
static i64 mover_ln;			// Mouse is hovering over this line
static i64 min_sel = I64_MAX;	// Minimum selected line index
static i64 max_sel = I64_MIN;	// Maximum selected line index

static wtxt separator = WL("-----------------------------------------------------------------");

DbgLogLine line = { 0, 0 };
static ui64 max_name = 0; // Variable name with maximum number of characters

static bool64 ctrl_down;
static bool64 enter_down;

static RECT work_pix; // Working pixels
static ui64 snap_pix = 24;
static bool64 snapping;

// Update _pVarName() filters if new special line added!
#define SEPARATOR		0xFFFFFFFFFFFFFFFF
#define VALUE_LINE		0xFFFFFFFFFFFFFFFE
#define VALUE_LAST_LINE	0xFFFFFFFFFFFFFFFD
#define BREAKPOINT		0xFFFFFFFFFFFFFFFC

static void findMaxLineAndUpdate()
{
	for(ui64 i = 0; i < ~lines; ++i)
	{
		if(max_line_l < ~lines[i].line)
		{
			max_line_l = ~lines[i].line;
		}
	}
	
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;
	si.nMin = 0;
	si.nMax = (i32)max_line_l;
	SetScrollInfo(main_wnd, SB_HORZ, &si, TRUE);
	InvalidateRect(main_wnd, NULL, FALSE);
}

static void updateMaxLine(const DbgLogLine &ln)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	
	if(max_line_l < ~ln.line)
	{
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
	
	updateMaxLine(line);
	
	si.fMask = SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = (i32)~lines - 1;
	si.nPos = si.nMax;
	
	SetScrollInfo(main_wnd, SB_VERT, &si, TRUE);
	ScrollWindow(main_wnd, 0, (i32)cellh, NULL, NULL);
	
	//InvalidateRect(main_wnd, NULL, FALSE);
	//UpdateWindow(main_wnd);
}

static void pushLine()
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	
	lines << line;
	
	if(main_wnd == NULL)
	{
		return;
	}
	
	updateScrollState();
}

static bool enableSnapCheck()
{
	static RECT rec;
	GetWindowRect(main_wnd, &rec);
	rec.right -= rec.left;
	rec.bottom -= rec.top;
	
	i64 wx = rec.left, wy = rec.top, ww = rec.right, wh = rec.bottom;
	i64 sx = work_pix.left, sy = work_pix.top, sxr = work_pix.right, syb = work_pix.bottom;
	i64 sp = (i64)snap_pix;
	
	return !((wx - sx < sp) || (wy - sy < sp) || (sxr - wx - ww < sp) || (syb - wy - wh < sp));
}

void _softBp(ui64 bp_line)
{
	line.line = WL("BREAKPOINT ");
	line.line += i2wt(bp_line);
	line.varp = BREAKPOINT;
	
	pushLine();
	
	static bool con_exit_handler_set = false;
	if(!con_exit_handler_set)
	{
		if(GetConsoleWindow() != NULL)
		{
			_dbgSetCleanSelectsHandler();
			winapi::onexit(dbgCleanSelectsOnExit);
		}
		else
		{
			winapi::onexit(dbgCleanSelectsOnExit);
		}
		con_exit_handler_set = true;
	}
	
	if(!enter_down)
	{
		_dbgSelLine(bp_line);
		ResetEvent(soft_breakpoint_event);
		WaitForSingleObject(soft_breakpoint_event, INFINITE);
	}
}

void _softBpNj(ui64 bp_line)
{
	if(soft_breakpoint_event == NULL)
	{
		soft_breakpoint_event = CreateEventA(NULL, TRUE, FALSE, NULL);
	}
	
	line.line = WL("BREAKPOINT ");
	line.line += i2wt(bp_line);
	line.varp = BREAKPOINT;
	
	pushLine();
	
	if(!enter_down)
	{
		ResetEvent(soft_breakpoint_event);
		WaitForSingleObject(soft_breakpoint_event, INFINITE);
	}
}

static void clipLines(i64 min, i64 max)
{
	if(min > (i64)~lines)
	{
		return;
	}
	
	wtxt to_clip;
	for(ui64 i = (ui64)min; i <= (ui64)max; ++i)
	{
		if(lines[i].varp == SEPARATOR)
		{
			to_clip += separator;
		}
		else
		{
			to_clip += lines[i].line;
		}
		
		if(i != (ui64)max)
		{
			to_clip += '\n';
		}
	}
	
	clip(to_clip, main_wnd);
}

static LRESULT wmDestoryHandler()
{
	_dbgCleanSelects();
	DeleteObject(default_font);
	DeleteObject(backgrnd);
	PostQuitMessage(0);
	return 0;
}

static void commitSudoku()
{
	//TerminateProcess(GetCurrentProcess(), 0); // For maximum brutality!
	ExitProcess(0);
}

static LRESULT wmKeyUpHandler(WPARAM wp)
{
	switch(wp)
	{
	case VK_ESCAPE:
		DestroyWindow(main_wnd);
		wmDestoryHandler();
		break;
	case VK_BACK:
		DestroyWindow(main_wnd);
		wmDestoryHandler();
		commitSudoku();
		break;
	case 0x41: // A
		if(ctrl_down)
		{
			clipLines(0, (i64)~lines-1);
			InvalidateRect(main_wnd, NULL, FALSE);
			COLORREF tmp = def_back;
			def_back = hov_back;
			hov_back = tmp;
			UpdateWindow(main_wnd);
			Sleep(100);
			hov_back = def_back;
			def_back = tmp;
			InvalidateRect(main_wnd, NULL, FALSE);
			UpdateWindow(main_wnd);
		}
		break;
	case VK_CONTROL:
		ctrl_down = false;
		break;
	case VK_RETURN:
		enter_down = false;
		break;
	default:
		SetEvent(soft_breakpoint_event);
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
	i16 wh_d; // Wheel delta
	SCROLLINFO si;
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

static LRESULT wmMouseMoveHandler(HWND wnd, WPARAM wp, LPARAM lp)
{
	i32 x = LWSIGN(lp), y = HWSIGN(lp);
	if(wp == MK_LBUTTON)
	{
		RECT rec;
		GetWindowRect(wnd, &rec);
		rec.right -= rec.left;
		rec.bottom -= rec.top;
		
		static bool xmove = true, ymove = true;
		
		if(snapping)
		{
			i64 wx = rec.left, wy = rec.top, ww = rec.right, wh = rec.bottom;
			i64 sx = work_pix.left, sy = work_pix.top, sxr = work_pix.right, syb = work_pix.bottom;
			i64 sp = (i64)snap_pix;
			
			if(wx - sx < sp)
			{
				rec.left = work_pix.left;
				xmove = false;
			}
			if(wy - sy < sp)
			{
				rec.top = work_pix.top;
				ymove = false;
			}
			if(sxr - wx - ww < sp)
			{
				rec.left = work_pix.right - rec.right;
				xmove = false;
			}
			if(syb - wy - wh < sp)
			{
				rec.top = work_pix.bottom - rec.bottom;
				ymove = false;
			}
		}
		else
		{
			xmove = true, ymove = true;
		}
		
		if(xmove)
		{
			rec.left += (LONG)(x - mx_old);
		}
		if(ymove)
		{
			rec.top += (LONG)(y - my_old);
		}
		
		MoveWindow(wnd, rec.left, rec.top, rec.right, rec.bottom, FALSE);
		return 0;
	}
	else if(wp == MK_RBUTTON)
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
		
		InvalidateRect(wnd, NULL, FALSE);
	}
	
	return 0;
}

static PAINTSTRUCT ps;
static SCROLLINFO si;
static DbgLogLines &l = lines;

LRESULT CALLBACK dbgWndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
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
		
		//////////////CRITICAL//SECTION//////////////
		for(ui64 i = vis_ln_beg; i <= vis_ln_end && i < ~l; ++i)
		{
			if((i64)i >= min_sel && (i64)i <= max_sel)
			{
				SetBkColor(main_dc, hov_back);
			}
			else
			{
				SetBkColor(main_dc, def_back);
			}
			
			i32 x = -((i32)x_scrl * (i32)cellw);
			i32 y = (i32)((i - y_scrl) * cellh);
			
			if(l[i].varp == SEPARATOR)
			{
				SetTextColor(main_dc, separator_c);
				TextOut(main_dc, x, y, separator, (i32)~separator);
				SetTextColor(main_dc, def_front);
				continue;
			}
			else if(l[i].varp == VALUE_LINE)
			{
				SetTextColor(main_dc, def_front);
				TextOut(main_dc, x, y, l[i].line, (i32)~l[i].line);
				continue;
			}
			else if(l[i].varp == VALUE_LAST_LINE)
			{
				SetTextColor(main_dc, def_front);
				TextOut(main_dc, x, y, l[i].line, (i32)~l[i].line - 1);
				SetTextColor(main_dc, var_brak_c);
				TextOut(main_dc, x + (i32)((~l[i].line - 1) * cellw), y, L"]", 1);
				continue;
			}
			else if(l[i].varp == BREAKPOINT)
			{
				SetTextColor(main_dc, bpoint_c);
				TextOut(main_dc, x, y, l[i].line, 11); // [BREAKPOINT ] = 11
				SetTextColor(main_dc, bpoint_lc);
				TextOut(main_dc, x + 11 * (i32)cellw, y, (wchar_t *)l[i].line + 11, (i32)~l[i].line - 11);
				continue;
			}
			
			wchar_t *var_val = (wchar_t *)l[i].line + l[i].varp + 1;
			i32 var_val_s = (i32)~l[i].line - (i32)l[i].varp - 2; // -1 coz varp is index
			
			SetTextColor(main_dc, var_name_c);
			TextOut(main_dc, x, y, l[i].line, (i32)l[i].varp);
			SetTextColor(main_dc, var_brak_c);
			TextOut(main_dc, x + (i32)((l[i].varp) * cellw), y, L"[", 1);
			SetTextColor(main_dc, def_front);
			
			if(l[i].line[~l[i].line-1] == ']') // No newlines was found in the variable value
			{
				TextOut(main_dc, x + (i32)((l[i].varp + 1) * cellw), y, var_val, var_val_s);
				SetTextColor(main_dc, var_brak_c);
				TextOut(main_dc, x + (i32)((~l[i].line - 1) * cellw), y, L"]", 1);
				continue;
			}
			
			++var_val_s; // Account for missing closing ']'
			TextOut(main_dc, x + (i32)((l[i].varp + 1) * cellw), y, var_val, var_val_s);
		}
		//////////////CRITICAL//SECTION//////////////
		
		EndPaint(wnd, &ps); // END PAINT --------------------------------------------
	}	return 0;
	case WM_CREATE: // After window is created, but before it is visible
		SetEvent(dbg_wnd_spawn_event); // Only allow base program to continue when DBG window is spawned
		return 0;
	case WM_SHOWWINDOW: // Window is about to be hidden or shown.
		updateScrollState();
		findMaxLineAndUpdate();
		return 0;
	case WM_SIZE:
		clw = LW(lp);
		clh = HW(lp);
		
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = (i32)~lines - 1;
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
		if(wp == VK_CONTROL)
		{
			ctrl_down = true;
		}
		else if(wp == VK_RETURN)
		{
			enter_down = true;
			SetEvent(soft_breakpoint_event);
		}
		return 0;
	case WM_KEYUP:
		return wmKeyUpHandler(wp);
	case WM_RBUTTONDOWN:
		return wmMouseMoveHandler(wnd, wp, lp);
	case WM_RBUTTONUP:
		clipLines(min_sel, max_sel);
		min_sel = I64_MAX;
		max_sel = I64_MIN;
		InvalidateRect(wnd, NULL, FALSE);
		return 0;
	case WM_LBUTTONDOWN:
		snapping = enableSnapCheck();
		mx_old = LWSIGN(lp), my_old = HWSIGN(lp);
		return 0;
	case WM_LBUTTONUP:
		snapping = false;
		return 0;
	case WM_MOUSEWHEEL:
		return wmMouseWheelHandler(wnd, wp);
	case WM_MOUSEMOVE:
		return wmMouseMoveHandler(wnd, wp, lp);
	case WM_CLOSE:
		DestroyWindow(wnd);
		return 0;
	case WM_DESTROY:
		return wmDestoryHandler();
	default:
		return DefWindowProc(wnd, msg, wp, lp);
	}
}

void dbgInitGui()
{
	exec_adress = GetModuleHandle(NULL);

	// Create Fake Window to get active monitor's DPI
	HWND wnd = CreateWindowEx(0, L"Button", 0, 0,
		0, 0, 0, 0, 0, 0, exec_adress, 0);
	dpi_scale = (float)GetDpiForWindow(wnd) / 96.0f;
	DestroyWindow(wnd);
	
	cellw = DPIS(cellw);
	cellh = DPIS(cellh);
	
	snap_pix = DPIS(snap_pix);
	
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

HWND dbgSpawnDbgWnd()
{
	HICON ico = LoadIcon(exec_adress, L"MAINICON");
	backgrnd = CreateSolidBrush(def_back);
	DWORD style_ex = WS_EX_COMPOSITED | WS_EX_TOPMOST;
	DWORD style = WS_POPUP | WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL;
	
	// Register Window Class Atom ==============================================================
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);					// Structure size in bytes
	wc.style = CS_DBLCLKS;							// Class style, any combination
	wc.lpfnWndProc = dbgWndProc;					// Window callback function
	wc.cbClsExtra = 0;								// Extra Class Memory (max 40B)
	wc.cbWndExtra = 0;								// Extra Window Memory (max 40B)
	wc.hInstance = exec_adress;						// Handle to module's instance
	wc.hIcon = ico;									// Def. icon for all windows
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);		// Def. cursor for all windows
	wc.hbrBackground = backgrnd;					// Def. brush for WM_ERASEBKGND
	wc.lpszMenuName = NULL;							// Def. menu name for all w.
	wc.lpszClassName = L"DbgLogSuperWindow";		// Name of the Class (Atom)
	wc.hIconSm = 0;									// Def. small icon for all w.

	main_class = RegisterClassEx(&wc);
	// =========================================================================================
	
	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_pix, 0);
	i32 w = (i32)cellw * 100;
	i32 h = (i32)cellh * 30;
	i32 x = work_pix.left;
	i32 y = work_pix.bottom - h;
	
	// Create Window ===========================================================================
	main_wnd = CreateWindowEx(
		style_ex,				//   [I] Extended window style
		(LPCWSTR)main_class,	// [I|O] Class Atom / Class Atom String
		NULL,					// [I|O] Window name String (Title)
		style,					//   [I] Window style (WS_OVERLAPPED = 0x0)
		x, y, w, h,				//   [I] PosX, PoxY, Width, Height
		NULL,					// [I|O] Handle to this window's parent
		NULL,					// [I|O] Handle to menu / Child-window ID
		exec_adress,			// [I|O] Handle to instance of the module
		NULL);					// [I|O] CREATESTRUCT ptr. for lParam of WM_CREATE
	// =========================================================================================
	
	UpdateWindow(main_wnd);
	ShowWindow(main_wnd, SW_NORMAL);
	
	DWORD cur_thread = GetCurrentThreadId();
	static DWORD fg_pid = 0;
	DWORD fg_thread = GetWindowThreadProcessId(GetForegroundWindow(), &fg_pid);
	
	AttachThreadInput(cur_thread, fg_thread, TRUE);
	AllowSetForegroundWindow(ASFW_ANY);
	SetFocus(main_wnd);
	SetForegroundWindow(main_wnd);
	SetWindowPos(main_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	//SetWindowPos(main_wnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	AttachThreadInput(cur_thread, fg_thread, FALSE);

	return main_wnd;
}

#pragma warning( suppress : 4100 )
static DWORD __declspec(nothrow) msgProcThread(LPVOID lp)
{
	dbg_wnd_active = true;
	dbgInitGui();
	dbgSpawnDbgWnd();
	
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}
	
	UnregisterClass((LPCWSTR)main_class, exec_adress);
	dbg_wnd_active = false;
	return 0;
}

void _dbgWnd()
{
	if(!dbg_wnd_active)
	{
		ResetEvent(dbg_wnd_spawn_event);
		CreateThread(NULL, 0, msgProcThread, NULL, 0, NULL);
		WaitForSingleObject(dbg_wnd_spawn_event, INFINITE);
	}
}

void _pVarName()
{
	QueryPerformanceCounter(&var_pe);
	
	if((var_pe.QuadPart - var_ps.QuadPart)/(1000000/sw_ns_per_tick) > 1000 && ~lines != 0) // 1 second
	{
		DbgLogLine sep_line = { SEPARATOR, 0 };
		lines << sep_line;
	}
	
	ui64 s = ~*dbgp;
	if(max_name < s)
	{
		if(~lines != 0)
		{
			ui64 sp2ins = s - max_name;
			wtxtp sp;
			sp|S(sp2ins);
			
			for(ui64 i = ~lines-1; i != UI64_MAX; --i)
			{
				if(lines[i].varp == SEPARATOR) // This makes different indent for groups possible
				{
					break;
				}
				
				if(lines[i].varp == BREAKPOINT) // Skip breakpoints from indenting
				{
					continue;
				}
				
				txti(lines[i].line, 0, *sp);
				updateMaxLine(lines[i]);
				
				if(lines[i].varp != VALUE_LINE && lines[i].varp != VALUE_LAST_LINE)
				{
					lines[i].varp += sp2ins;
				}
			}
		}
		
		max_name = s;
	}
	
	ui64 sp2ins = max_name - s;
	if(sp2ins != 0)
	{
		wtxtp sp;
		sp|S(sp2ins);
		line.line = *sp + t2u16(*dbgp) + WL("  [");
	}
	else
	{
		line.line = t2u16(*dbgp) + WL("  [");
	}
	
	line.varp = ~line.line - 1;
	
	dbgp|CL;
}

wchar_t np2u(char c) // Non-printable ASCII to unicode
{
	wchar_t out = 0x2776;
	switch(c)
	{	
	case 0x00: out = 0;				break;	// NULL 0
	case 0x01:						break;  // SOH 	❶
	case 0x02: out += 1;			break;  // STX 	❷
	case 0x03: out += 2;			break;  // ETX	❸
	case 0x04: out += 3;			break;  // EOT	❹
	case 0x05: out += 4;			break;  // ENQ	❺
	case 0x06: out += 5;			break;  // ACK	❻
	case 0x07: out += 6;			break;  // BEL	❼
	case 0x08: out += 7;			break;  // BS	❽
	case 0x09: out += 8;			break;  // HT	❾
	case 0x0A: out += 9;			break;  // LF	❿
	case 0x0B: out = 0x24EB;		break;  // VT	⓫
	case 0x0C: out = 0x24EB + 1;	break;  // FF	⓬
	case 0x0D: out = 0x24EB + 2;	break;  // CR	⓭
	case 0x0E: out = 0x24EB + 3;	break;  // SO	⓮
	case 0x0F: out = 0x24EB + 4;	break;  // SI	⓯
	case 0x10: out = 0x24EB + 5;	break;  // DLE	⓰
	case 0x11: out = 0x24EB + 6;	break;  // DC1	⓱
	case 0x12: out = 0x24EB + 7;	break;  // DC2	⓲
	case 0x13: out = 0x24EB + 8;	break;  // DC3	⓳
	case 0x14: out = 0x24EB + 9;	break;  // DC4	⓴
	case 0x15: out = 0x2460;		break;  // NAK	①
	case 0x16: out = 0x2460 + 1;	break;  // SYN	②
	case 0x17: out = 0x2460 + 2;	break;  // ETB	③
	case 0x18: out = 0x2460 + 3;	break;  // CAN	④
	case 0x19: out = 0x2460 + 4;	break;  // EM	⑤
	case 0x1A: out = 0x2460 + 5;	break;  // SUB	⑥
	case 0x1B: out = 0x2460 + 6;	break;  // ESC	⑦
	case 0x1C: out = 0x2460 + 7;	break;  // FS	⑧
	case 0x1D: out = 0x2460 + 8;	break;  // GS	⑨
	case 0x1E: out = 0x2460 + 9;	break;  // RS	⑩
	case 0x1F: out = 0x2460 + 10;	break;  // US	⑪
	default:
		break;
	}
	
	return out;
}

void _pVarVal()
{
	static wtxt val;
	static wtxtp sp;
	val = t2u16(*dbgp);
	
	ui64 pre_nl = 0; // Previous newline
	for(ui64 i = 0; i < ~val; ++i)
	{
		if(val[i] == '\n') // Linux newline
		{
			if(~line.line == 0)
			{
				line.varp = VALUE_LINE; // Indacate that this is pure value
				sp|S(max_name + 3); // +3 for this: (L"  [")
				sp|txtsp(val, pre_nl+1, i);
			}
			else
			{
				sp|txtsp(val, pre_nl, i);
			}
			
			wtxt out_line = *sp;
			if(val[i-1] == '\r') // Windows newline
			{
				txtd(out_line, ~out_line-2, 2);
			}
			else
			{
				out_line[~out_line-1] = 0;
				txtssz(out_line, ~out_line-1);
			}
			
			line.line += out_line;
			
			pushLine();
			
			txtclr(line.line);
			pre_nl = i;
			sp|CL;
		}
		else if(val[i] < 0x20)
		{
			if(val[i] == '\r' && i+1 < ~val && val[i+1] == '\n')
			{
				continue;
			}
			
			val[i] = np2u((char)val[i]);
		}
	}
	
	if(pre_nl == 0)
	{
		val += ']';
		line.line += val;
	}
	else
	{
		line.varp = VALUE_LAST_LINE; // Indacate that this is last pure value line
		sp|S(max_name + 3); // +3 for this: (L"  [")
		
		if(pre_nl+1 < ~val)
		{
			sp|txts(val, pre_nl+1, TEND);
		}
		
		sp|']';
		line.line += *sp;
		sp|CL;
	}
	
	pushLine();
	
	dbgp|CL;
	QueryPerformanceCounter(&var_ps);
}

void _p(ui64 ui)
{
	dbgp|ui;
}

void _p(ui32 ui)
{
	dbgp|ui;
}

void _p(uli32 ui)
{
	dbgp|ui;
}

void _p(ui16 ui)
{
	dbgp|ui;
}

void _p(i64 i)
{
	dbgp|i;
}

void _p(i32 i)
{
	dbgp|i;
}

void _p(li32 i)
{
	dbgp|i;
}

void _p(i16 i)
{
	dbgp|i;
}

void _p(wchar_t wc)
{
	dbgp|wt2u8((wtxt)wc);
}

void _p(char c)
{
	dbgp|c;
}

void _p(bool b)
{
	dbgp|b;
}

void _p(const void *vp)
{
	dbgp|vp;
}

void _p(const char *ccp)
{
	dbgp|ccp;
}

void _p(const wchar_t *ccp)
{
	dbgp|wt2u8(cwstr({ ccp, strl(ccp) }));
}

void _p(const txt& t)
{
	dbgp|t;
}

void _p(const wtxt& t)
{
	dbgp|wt2u8(t);
}

