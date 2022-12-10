// CONSOLE
//#include <iostream>
//#include <sstream>
#include <sddb>
#include <dbghelp.h>

#pragma comment( lib, "dbghelp" )
#pragma comment( lib, "user32" )

// TODO https://docs.microsoft.com/en-us/windows/win32/debug/debugging-a-running-process
// TODO To cause a breakpoint exception in another process, use the DebugBreakProcess function.
// TODO Implement fully custom StackWalk
// TODO Implement custom PDB reader OR custom symbol exporting
// TODO Visualise full process\thread memory map with DLLs, stack and heap
// TODO Implement jump to source code via click

// void DebugBreak(); Causes a breakpoint exception to occur in the current process. This allows the calling thread
// to signal the debugger to handle the exception. But why, if I can use RaiseException?!
// REASON: //KERNELBASE.dll!RaiseException+0x69 VS KERNELBASE.dll!DebugBreak+0x3 --> many useless code in RaiseExc!

#define COL_END SetConsoleTextAttribute(oh, col_def);
	
#define COL_THREAD SetConsoleTextAttribute(oh, COL_CAY);
#define COL_DLL SetConsoleTextAttribute(oh, COL_VIO);
#define COL_PROC SetConsoleTextAttribute(oh, COL_BLU);
#define COL_STR SetConsoleTextAttribute(oh, COL_YEL);
#define COL_DESPAWN SetConsoleTextAttribute(oh, COL_RED);
#define COL_LOAD_SUCC SetConsoleTextAttribute(oh, COL_GRE);
#define COL_LOAD_FAIL SetConsoleTextAttribute(oh, COL_RED);

#define COL_TRACE_PLUS SetConsoleTextAttribute(oh, COL_VIO);
#define COL_TRACE_OFFSET SetConsoleTextAttribute(oh, COL_BLU);
#define COL_TRACE_SYMB SetConsoleTextAttribute(oh, COL_YEL);
#define COL_TRACE_LINE SetConsoleTextAttribute(oh, COL_CAY);

using namespace std;

WORD col_def;

DWORD decision(DWORD chance)
{
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	
	fkcursor();
	wcout << cay << "H" << def << "ANDLE | ";
	wcout << gre << "I" << def << "GNORE [";
	
	if(chance)
	{
		wcout << gre << "FIRST CHANCE" << def;
	}
	else
	{
		wcout << red << "FINAL CHANCE" << def;
	}
	
	cout << ']' << endl;
	
	while(1)
	{
		
		int ch = _getch();
		
		switch(ch)
		{
		case 'h':
		case 'H':
			okcursor();
			return DBG_CONTINUE;
		case 'i':
		case 'I':
			okcursor();
			return DBG_EXCEPTION_NOT_HANDLED;
		default:
			break;
		}
	}
}

typedef struct _DLL_LIST_ITEM
{
	LPWSTR n; // Name
	LPVOID b; // Base address
	DWORD en; // Number of exceptions that happned in this process
	DWORD id;
	struct _DLL_LIST_ITEM *next;
} DLL_LIST_ITEM;

void dllAttach(DLL_LIST_ITEM **root, LPCWSTR name, LPVOID base, DWORD id)
{
	DLL_LIST_ITEM *head = *root;
	
	if(head != NULL)
	{
		while(head->next != NULL)
		{
			head = head->next;
		}
	}
	
	static HANDLE heap = GetProcessHeap();
	LPVOID nh = NULL;
	if(name != NULL)
	{
		SIZE_T l = wcslen(name) + 1;
		nh = HeapAlloc(heap, 0, l * sizeof(wchar_t));
		CopyMemory(nh, name, l * sizeof(wchar_t));
	}
	
	DLL_LIST_ITEM tmp;
	tmp.n = (LPWSTR)nh;
	tmp.b = base;
	tmp.en = 0;
	tmp.id = id;
	tmp.next = NULL;
	LPVOID ih = HeapAlloc(heap, 0, sizeof(DLL_LIST_ITEM));
	CopyMemory(ih, &tmp, sizeof(DLL_LIST_ITEM));
	
	if(head == NULL)
	{
		*root = (DLL_LIST_ITEM *)ih;
	}
	else
	{
		head->next = (DLL_LIST_ITEM *)ih;
	}
}

void dllDestoryList(DLL_LIST_ITEM **root)
{
	DLL_LIST_ITEM *cur = *root;
	while(cur != NULL)
	{
		DLL_LIST_ITEM *tmp = cur;
		static HANDLE heap = GetProcessHeap();
		HeapFree(heap, 0, cur->n);
		HeapFree(heap, 0, cur);
		cur = tmp->next;
	}
	*root = NULL;
}

DLL_LIST_ITEM *dlls;
DLL_LIST_ITEM *threads;
DLL_LIST_ITEM *procs;

HANDLE fndHandle(DWORD id, DLL_LIST_ITEM *what)
{
	DLL_LIST_ITEM *itm = what;
	while(itm != 0)
	{
		if(itm->id == id)
		{
			return (HANDLE)itm->b;
		}
		
		itm = itm->next;
	}
	
	return NULL;
}

LPWSTR fndProcName(DWORD id)
{
	DLL_LIST_ITEM *itm = procs;
	while(itm != 0)
	{
		if(itm->id == id)
		{
			return itm->n;
		}
		
		itm = itm->next;
	}
	
	return NULL;
}

DWORD * getProcExceptStat(DWORD id)
{
	DLL_LIST_ITEM *itm = procs;
	while(itm != 0)
	{
		if(itm->id == id)
		{
			return &itm->en;
		}
		
		itm = itm->next;
	}
	
	static DWORD dummy;
	dummy = 0;
	return &dummy;
}

LPCSTR stripPath(LPCSTR path)
{
	SIZE_T l = strlen(path);
	LPCSTR lslash = path+l; // Null wchar aka. Path End
	while(*(--lslash) != L'\\' && lslash >= path)
	{
		;
	}
	
	return lslash + 1;
}

void checkSymType(SYM_TYPE t)
{
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	
	COL_LOAD_SUCC
	switch(t)
	{
	case SymCoff: /* = 0 */
		wcout << " COFF LOADED!";
		break;
	case SymCv:
		wcout << " CODEVIEW LOADED!";
		break;
	case SymDeferred:
		wcout << " LOADING DEFERRED!";
		break;
	case SymDia:
		wcout << " DIA LOADED!";
		break;
	case SymExport:
		wcout << " DLL EXPORT LOADED!";
		break;
	case SymPdb:
		wcout << " PDB LOADED!";
		break;
	case SymSym:
		wcout << " SYM LOADED!";
		break;
	case SymVirtual:
		wcout << " SYM VIRTUAL BS!";
		break;
	case NumSymTypes:
		wcout << " NUMSYMTYPES BS!";
		break;
	case SymNone:
		COL_LOAD_FAIL
		wcout << " NO SYMBOLS FOUND...";
		break;
	default:
		break;
	}
	COL_END
	wcout << endl;
}

BOOL __declspec(nothrow) readProcMemCallback(HANDLE ph, DWORD64 baddr, PVOID buff, DWORD sz, LPDWORD bread)
{
	return ReadProcessMemory(ph, (LPCVOID)baddr, buff, sz, (SIZE_T *)bread);
}

int wmain(int argc, wchar_t **argv)
{
	if(argc < 2 || argc > 2)
	{
		return 1;
	}
	
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	static CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(oh, &csbi);
	col_def = csbi.wAttributes;
	COORD buf = csbi.dwSize;
	buf.Y = 9000;
	SetConsoleScreenBufferSize(oh, buf);
	
	SetConsoleTitle(L"SDDBG");
	
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	
	// Create Process ========================================================================================
	BOOL res = CreateProcessW(
		argv[1],				//  [I|O]  Name of the module to be executed, that's it
		NULL,					// [IO|O]  Command line to be exectued, searches PATH, adds extention
		NULL,					//  [I|O]  Sec. Attrib. for inhereting new process by child processes
		NULL,					//  [I|O]  Sec. Attrib. for inhereting new thread by child processes
		FALSE,					//    [I]  New proc. inherits each inheritable handle
		CREATE_NEW_CONSOLE |
		DEBUG_PROCESS,			//    [I]  Process creation flags
		NULL,					//  [I|O]  Ptr to environment block of new process (inherit if NULL)
		NULL,					//  [I|O]  Full path to current directory for the process
		&si,					//    [I]  Ptr to STARTUPINFO struct, if dwFlags = 0, def. values used
		&pi);					//    [O]  Ptr to PROCESS_INFORMATION struct with new proc identification info
	// =======================================================================================================
	
	//DEBUG_ONLY_THIS_PROCESS 0x00000002 ---> To ignore child procs created by debugee
	
	DEBUG_EVENT de;
	DWORD continue_status = DBG_EXCEPTION_NOT_HANDLED;
	//typedef struct _DEBUG_EVENT {
	//    DWORD dwDebugEventCode;
	//    DWORD dwProcessId;
	//    DWORD dwThreadId;
	//    union {
	//        EXCEPTION_DEBUG_INFO Exception;
	//        CREATE_THREAD_DEBUG_INFO CreateThread;
	//        CREATE_PROCESS_DEBUG_INFO CreateProcessInfo;
	//        EXIT_THREAD_DEBUG_INFO ExitThread;
	//        EXIT_PROCESS_DEBUG_INFO ExitProcess;
	//        LOAD_DLL_DEBUG_INFO LoadDll;
	//        UNLOAD_DLL_DEBUG_INFO UnloadDll;
	//        OUTPUT_DEBUG_STRING_INFO DebugString;
	//        RIP_INFO RipInfo;
	//    } u;
	//} DEBUG_EVENT, *LPDEBUG_EVENT;
	
	while(1)
	{
		WaitForDebugEventEx(&de, INFINITE);
		
		switch(de.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:      /* 1 */
		{
			//typedef struct _EXCEPTION_DEBUG_INFO {
			//    EXCEPTION_RECORD ExceptionRecord;
			//    DWORD dwFirstChance;
			//} EXCEPTION_DEBUG_INFO, *LPEXCEPTION_DEBUG_INFO;
			
			HANDLE hp = fndHandle(de.dwProcessId, procs);
			
			pexcept(&de.u.Exception.ExceptionRecord, hp);
			
			HANDLE ht = fndHandle(de.dwThreadId, threads);
			wcout << "PROCESS: " << fndProcName(de.dwProcessId) << " (" << dec << de.dwProcessId
				<< ") THREAD: " << de.dwThreadId << endl << endl;
			
			// Skip regs/stack listing for first info. exception generated by kernel for each process
			if(de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT &&
				++(*getProcExceptStat(de.dwProcessId)) == 1)
			{
				continue_status = DBG_CONTINUE;
				break;
			}
			
			HWND wnd = GetConsoleWindow();
			SetForegroundWindow(wnd);

			// No idea what are this...
			// CONTEXT_SEGMENTS -> SegDs, SegEs, SegFs, and SegGs
			// CONTEXT_FLOATING_POINT -> Xmm0-Xmm15.
			// CONTEXT_DEBUG_REGISTERS -> Dr0-Dr3 and Dr6-Dr7.
			// Full list:
			// CONTEXT_CONTROL         (CONTEXT_AMD64 | 0x00000001L)
			// CONTEXT_INTEGER         (CONTEXT_AMD64 | 0x00000002L)
			// CONTEXT_SEGMENTS        (CONTEXT_AMD64 | 0x00000004L)
			// CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x00000008L)
			// CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x00000010L)
			// CONTEXT_FULL            (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)
			// CONTEXT_ALL             (CONTEXT_CONTROL | CONTEXT_INTEGER | 
            // CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)
		
			CONTEXT cx;
			memset(&cx, 0x11, sizeof(cx));
			cx.ContextFlags = CONTEXT_ALL;
			res = GetThreadContext(ht, &cx);
			if(!res)
			{
				perr();
				wcout <<  "GETTHREADCONTEXT FAILED!" << endl;
			}
			
			wcout << "RIP  0x" << hex << uppercase << setw(16) << setfill(L'0') <<
				cx.Rip << "  RSP  0x" << setw(16) << setfill(L'0') << cx.Rsp << endl;
			wcout << "EFL  0x" << setw(8) << setfill(L'0') << cx.EFlags << endl << endl;
			
			wcout << "RAX  0x" << setw(16) << setfill(L'0') << cx.Rax << "  RBX  0x" <<
				setw(16) << setfill(L'0') << cx.Rbx << endl;
			wcout << "RCX  0x" << setw(16) << setfill(L'0') << cx.Rcx << "  RDX  0x" <<
				setw(16) << setfill(L'0') << cx.Rdx << endl;
			wcout << "RSI  0x" << setw(16) << setfill(L'0') << cx.Rsi << "  RDI  0x" <<
				setw(16) << setfill(L'0') << cx.Rdi << endl;
			wcout << "RBP  0x" << setw(16) << setfill(L'0') << cx.Rbp << endl << endl;
			
			wcout << "R08  0x" << setw(16) << setfill(L'0') << cx.R8 << "  R09  0x" <<
				setw(16) << setfill(L'0') << cx.R9 << endl;
			wcout << "R10  0x" << setw(16) << setfill(L'0') << cx.R10 << "  R11  0x" <<
				setw(16) << setfill(L'0') << cx.R11 << endl;
			wcout << "R12  0x" << setw(16) << setfill(L'0') << cx.R12 << "  R13  0x" <<
				setw(16) << setfill(L'0') << cx.R13 << endl;
			wcout << "R14  0x" << setw(16) << setfill(L'0') << cx.R14 << "  R15  0x" <<
				setw(16) << setfill(L'0') << cx.R15 << endl << endl;
			
			/* typedef struct _tagSTACKFRAME64 {
				ADDRESS64   AddrPC;               // program counter
				ADDRESS64   AddrReturn;           // return address
				ADDRESS64   AddrFrame;            // frame pointer
				ADDRESS64   AddrStack;            // stack pointer
				ADDRESS64   AddrBStore;           // backing store pointer
				PVOID       FuncTableEntry;       // pointer to pdata/fpo or NULL
				DWORD64     Params[4];            // possible arguments to the function
				BOOL        Far;                  // WOW far call
				BOOL        Virtual;              // is this a virtual frame?
				DWORD64     Reserved[3];
				KDHELP64    KdHelp;
			} STACKFRAME64, *LPSTACKFRAME64;
			
			typedef struct _tagADDRESS64 {
				DWORD64       Offset;
				WORD          Segment;
				ADDRESS_MODE  Mode;
			} ADDRESS64, *LPADDRESS64;
			
			AddrMode1616 0
			AddrMode1632 1
			AddrModeReal 2
			AddrModeFlat 3 */
			
			/* typedef struct _IMAGEHLP_MODULE64 {
			 DWORD    SizeOfStruct;
			 DWORD64  BaseOfImage;
			 DWORD    ImageSize;
			 DWORD    TimeDateStamp;
			 DWORD    CheckSum;
			 DWORD    NumSyms;
			 SYM_TYPE SymType;
			 CHAR     ModuleName[32];
			 CHAR     ImageName[256];
			 CHAR     LoadedImageName[256];
			 CHAR     LoadedPdbName[256];
			 DWORD    CVSig;
			 CHAR     *CVData[MAX_PATH  3];
			 DWORD    PdbSig;
			 GUID     PdbSig70;
			 DWORD    PdbAge;
			 BOOL     PdbUnmatched;
			 BOOL     DbgUnmatched;
			 BOOL     LineNumbers;
			 BOOL     GlobalSymbols;
			 BOOL     TypeInfo;
			 BOOL     SourceIndexed;
			 BOOL     Publics;
			 DWORD    MachineType;
			 DWORD    Reserved;
			} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64; */
			
			/* typedef struct _SYMBOL_INFO {
			 ULONG   SizeOfStruct;
			 ULONG   TypeIndex;
			 ULONG64 Reserved[2];
			 ULONG   Index;
			 ULONG   Size;
			 ULONG64 ModBase;
			 ULONG   Flags;
			 ULONG64 Value;
			 ULONG64 Address;
			 ULONG   Register;
			 ULONG   Scope;
			 ULONG   Tag;
			 ULONG   NameLen;
			 ULONG   MaxNameLen;
			 CHAR    Name[1];
			} SYMBOL_INFO, *PSYMBOL_INFO; */
			
			STACKFRAME64 sf;  // MB COZ MIDDLE MEMBE OF ADDRPC IS NOT SET TO 0??!?! MBMB
			sf.AddrPC.Offset = cx.Rip;
			sf.AddrPC.Mode = AddrModeFlat;
			sf.AddrFrame.Offset = cx.Rbp;
			sf.AddrFrame.Mode = AddrModeFlat;
			sf.AddrStack.Offset = cx.Rsp;
			sf.AddrStack.Mode = AddrModeFlat;
			
			SYMBOL_INFO *psi;
			psi = (SYMBOL_INFO *)HeapAlloc(GetProcessHeap(), 0,
				sizeof(SYMBOL_INFO) + (MAX_SYM_NAME - 1) * sizeof(wchar_t));
			memset(psi, 0x0, sizeof(SYMBOL_INFO) + (MAX_SYM_NAME - 1) * sizeof(wchar_t));
			psi->SizeOfStruct = sizeof(SYMBOL_INFO);
			psi->MaxNameLen = MAX_SYM_NAME;
			DWORD64 displace;
			
			wcout << "STACK TRACE" << endl;
			
			do
			{
				IMAGEHLP_MODULE64 mi;
				mi.SizeOfStruct = sizeof(mi);
				res = SymGetModuleInfo64(
					hp,
					(DWORD64)sf.AddrPC.Offset,
					&mi);
				
				SymFromAddr(hp, sf.AddrPC.Offset, &displace, psi);
				
				
				wcout << "0x" << hex << uppercase << setw(16) << setfill(L'0')
					<< psi->Address;
				COL_TRACE_PLUS
				wcout << "+";
				COL_TRACE_OFFSET
				wcout << "0x" << displace;
				COL_END
				wcout << "|" << stripPath(mi.LoadedImageName) << "|";
				COL_TRACE_SYMB
				wcout << psi->Name;
				COL_END
				wcout << "|";
				
				IMAGEHLP_LINE64 ln;
				ln.SizeOfStruct = sizeof(ln);
				DWORD displace0;
				res = SymGetLineFromAddr64(hp, sf.AddrPC.Offset, &displace0, &ln);
				
				if(res)
				{
					wcout << stripPath(ln.FileName) << "(" << dec;
					COL_TRACE_LINE
					wcout << ln.LineNumber;
					COL_END
					wcout << ")";
				}
				
				wcout << endl;
				
				res = StackWalk64(
					IMAGE_FILE_MACHINE_AMD64,
					hp,
					ht,
					&sf,
					&cx,
					readProcMemCallback,
					SymFunctionTableAccess64,
					SymGetModuleBase64,
					NULL);
				if(!res)
				{
					break;
				}
			}
			while(sf.AddrReturn.Offset != 0);
				
			HeapFree(GetProcessHeap(), 0, psi);
			wcout << endl;
		}
			
			continue_status = decision(de.u.Exception.dwFirstChance);
			break;
		case CREATE_THREAD_DEBUG_EVENT:  /* 2 */
			//typedef struct _CREATE_THREAD_DEBUG_INFO {
			//  HANDLE                 hThread;
			//  LPVOID                 lpThreadLocalBase;
			//  LPTHREAD_START_ROUTINE lpStartAddress;
			//} CREATE_THREAD_DEBUG_INFO, *LPCREATE_THREAD_DEBUG_INFO;
			dllAttach(&threads, NULL, (LPVOID)de.u.CreateThread.hThread, de.dwThreadId);
			COL_THREAD;
			wcout << "THREAD SPAWNED";
			COL_END;
			wcout << " BY " << fndProcName(de.dwProcessId) << ": "
				<< "0x" << de.u.CreateThread.lpThreadLocalBase << " ID: " << dec << de.dwThreadId << endl;
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		case CREATE_PROCESS_DEBUG_EVENT: /* 3 */
			//typedef struct _CREATE_PROCESS_DEBUG_INFO {
			//  HANDLE                 hFile;
			//  HANDLE                 hProcess;
			//  HANDLE                 hThread;
			//  LPVOID                 lpBaseOfImage;
			//  DWORD                  dwDebugInfoFileOffset;
			//  DWORD                  nDebugInfoSize;
			//  LPVOID                 lpThreadLocalBase;
			//  LPTHREAD_START_ROUTINE lpStartAddress;
			//  LPVOID                 lpImageName;
			//  WORD                   fUnicode;
			//} CREATE_PROCESS_DEBUG_INFO, *LPCREATE_PROCESS_DEBUG_INFO;
			
			
			//PROCESS SPAWNED SUCCESSFULLY!
			//PROCESS SPAWNED: 27096 000000000014F140 PDB LOADED! DBG: 0
			
			COL_PROC
			wcout << "PROCESS SPAWNED";
			COL_END
			wcout << ": ";
		{
			wchar_t nm[MAX_PATH];
			GetFinalPathNameByHandle(de.u.CreateProcessInfo.hFile, nm, MAX_PATH, 0);
			wcout << nm + 4 << " ID: " << dec << de.dwProcessId; // Skip '\\?\' syntax
			
			SymInitialize(de.u.CreateProcessInfo.hProcess, NULL, FALSE);
			SymLoadModuleExW(
				de.u.CreateProcessInfo.hProcess,
				NULL,
				nm,
				NULL,
				(DWORD64)de.u.CreateProcessInfo.lpBaseOfImage,
				0,
				NULL,
				0);
				
			IMAGEHLP_MODULE64 mi;
			mi.SizeOfStruct = sizeof(mi);
			res = SymGetModuleInfo64(
				de.u.CreateProcessInfo.hProcess,
				(DWORD64)de.u.CreateProcessInfo.lpBaseOfImage,
				&mi);
			
			if(res)
			{
				checkSymType(mi.SymType);
			}
		
			// de.u.CreateProcessInfo.nDebugInfoSize -> Useless. Probably legacy info (always 0)
			CloseHandle(de.u.CreateProcessInfo.hFile);
			COL_THREAD
			wcout << "MAIN THREAD";
			COL_END
			wcout << ": " << "0x" << de.u.CreateProcessInfo.lpThreadLocalBase 
				<< " ID: " << dec << de.dwThreadId << endl;
			dllAttach(&threads, NULL, (LPVOID)de.u.CreateProcessInfo.hThread, de.dwThreadId);
			
			SIZE_T l = wcslen(nm);
			LPCWSTR lslash = nm+l; // Null wchar aka. Path End
			while(*(--lslash) != L'\\' && lslash >= nm)
			{
				;
			}
			
			dllAttach(&procs, lslash+1, (LPVOID)de.u.CreateProcessInfo.hProcess, de.dwProcessId);
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
		}	break;
		case EXIT_THREAD_DEBUG_EVENT:    /* 4 */
			//typedef struct _EXIT_THREAD_DEBUG_INFO {
			//  DWORD dwExitCode;
			//} EXIT_THREAD_DEBUG_INFO, *LPEXIT_THREAD_DEBUG_INFO;
			COL_DESPAWN
			wcout << "THREAD DIED";
			COL_END
			wcout << " IN "  << fndProcName(de.dwProcessId) << ": "
				<< dec << de.dwThreadId << " C: " << de.u.ExitThread.dwExitCode << endl;
			
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		case EXIT_PROCESS_DEBUG_EVENT:   /* 5 */
			//typedef struct _EXIT_PROCESS_DEBUG_INFO {
			//  DWORD dwExitCode;
			//} EXIT_PROCESS_DEBUG_INFO, *LPEXIT_PROCESS_DEBUG_INFO;
			COL_DESPAWN
			wcout << "PROCESS DIED";
			COL_END
			wcout << ": " << fndProcName(de.dwProcessId) << " C: "
				<< de.u.ExitProcess.dwExitCode << endl;
			
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		case LOAD_DLL_DEBUG_EVENT:       /* 6 */
			//typedef struct _LOAD_DLL_DEBUG_INFO {
			//  HANDLE hFile;
			//  LPVOID lpBaseOfDll;
			//  DWORD  dwDebugInfoFileOffset;
			//  DWORD  nDebugInfoSize;
			//  LPVOID lpImageName;
			//  WORD   fUnicode;
			//} LOAD_DLL_DEBUG_INFO, *LPLOAD_DLL_DEBUG_INFO;
			COL_DLL
			wcout << "DLL LOADED";
			COL_END
			wcout << " BY " << fndProcName(de.dwProcessId) << ": "
				<< "0x" << hex << uppercase << de.u.LoadDll.lpBaseOfDll << ' ';
		{
			wchar_t name[MAX_PATH];
			GetFinalPathNameByHandle(de.u.LoadDll.hFile, name, MAX_PATH, 0);
			wcout << name + 4; // Skip '\\?\' syntax
			dllAttach(&dlls, name + 4, de.u.LoadDll.lpBaseOfDll, 0);
			
			HANDLE hp = fndHandle(de.dwProcessId, procs);
			
			SymLoadModuleExW(
				hp,
				NULL,
				name,
				NULL,
				(DWORD64)de.u.LoadDll.lpBaseOfDll,
				0,
				NULL,
				0);
				
			IMAGEHLP_MODULE64 mi;
			mi.SizeOfStruct = sizeof(mi);
			res = SymGetModuleInfo64(
				hp,
				(DWORD64)de.u.LoadDll.lpBaseOfDll,
				&mi);
				
			if(res)
			{
				checkSymType(mi.SymType);
			}
			
			CloseHandle(de.u.CreateProcessInfo.hFile);
			
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
		}	break;
		case UNLOAD_DLL_DEBUG_EVENT:     /* 7 */
		{
			//typedef struct _UNLOAD_DLL_DEBUG_INFO {
			//  LPVOID lpBaseOfDll;
			//} UNLOAD_DLL_DEBUG_INFO, *LPUNLOAD_DLL_DEBUG_INFO;
			COL_DESPAWN
			wcout << "DLL UNLOADED";
			COL_END
			wcout << " BY " << fndProcName(de.dwProcessId) << ": ";
			DLL_LIST_ITEM *dll = dlls;
			while(dll != NULL)
			{
				if(dll->b == de.u.UnloadDll.lpBaseOfDll)
				{
					wcout << dll->n;
					break;
				}
				dll = dll->next;
			}
			wcout << endl;
		}
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		case OUTPUT_DEBUG_STRING_EVENT:  /* 8 */
		{
			//typedef struct _OUTPUT_DEBUG_STRING_INFO {
			//  LPSTR lpDebugStringData;
			//  WORD  fUnicode;
			//  WORD  nDebugStringLength;
			//} OUTPUT_DEBUG_STRING_INFO, *LPOUTPUT_DEBUG_STRING_INFO;
			SIZE_T s = de.u.DebugString.fUnicode ? 
				de.u.DebugString.nDebugStringLength :
				de.u.DebugString.nDebugStringLength * sizeof(wchar_t);
			LPVOID ds = HeapAlloc(GetProcessHeap(), 0, s);
			memset(ds, 0x0, s);
			
			HANDLE ph = fndHandle(de.dwProcessId, procs);
			
			ReadProcessMemory(ph, de.u.DebugString.lpDebugStringData, ds, s, NULL);
			
			if(de.u.DebugString.fUnicode)
			{
				COL_STR
				wcout << "DEBUG WSTR";
				COL_END
				wcout << " BY " << fndProcName(de.dwProcessId) << ": |" << (LPWSTR)ds  << "|" << endl;
			}
			else
			{
				COL_STR
				wcout << "DEBUG WSTR";
				COL_END
				wcout << " BY " << fndProcName(de.dwProcessId) << ": |" << (LPSTR)ds  << "|" << endl;
			}
			
			HeapFree(GetProcessHeap(), 0, ds);
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
		}	break;
		case RIP_EVENT:                  /* 9 */
			COL_DESPAWN
			wcout << "RIP_EVENT ---> IGNORING!" << endl;
			COL_END
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		default:
			break;
		}
		
		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, continue_status);
	}
	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	dllDestoryList(&dlls);
	dllDestoryList(&threads);
	
	return 0;
}


//DBG_CONTINUE 0x00010002L

// If the thread specified by the dwThreadId parameter previously reported an EXCEPTION_DEBUG_EVENT debugging event, the function stops all exception processing and continues the thread and the exception is marked as handled. For any other debugging event, this flag simply continues the thread.

//DBG_EXCEPTION_NOT_HANDLED 0x80010001L

// If the thread specified by dwThreadId previously reported an EXCEPTION_DEBUG_EVENT debugging event, the function continues exception processing. If this is a first-chance exception event, the search and dispatch logic of the structured exception handler is used; otherwise, the process is terminated. For any other debugging event, this flag simply continues the thread.

//DBG_REPLY_LATER 0x40010001L

// Supported in Windows 10, version 1507 or above, this flag causes dwThreadId to replay the existing breaking event after the target continues. By calling the SuspendThread API against dwThreadId, a debugger can resume other threads in the process and later return to the breaking.

// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//
// CONTEXT_CONTROL specifies SegSs, Rsp, SegCs, Rip, and EFlags.
//
// CONTEXT_INTEGER specifies Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi, and R8-R15.
//
// CONTEXT_SEGMENTS specifies SegDs, SegEs, SegFs, and SegGs.
//
// CONTEXT_FLOATING_POINT specifies Xmm0-Xmm15.
//
// CONTEXT_DEBUG_REGISTERS specifies Dr0-Dr3 and Dr6-Dr7.
//

//typedef struct DECLSPEC_ALIGN(16) DECLSPEC_NOINITALL _CONTEXT {
//
//    //
//    // Register parameter home addresses.
//    //
//    // N.B. These fields are for convience - they could be used to extend the
//    //      context record in the future.
//    //
//
//    DWORD64 P1Home;
//    DWORD64 P2Home;
//    DWORD64 P3Home;
//    DWORD64 P4Home;
//    DWORD64 P5Home;
//    DWORD64 P6Home;
//
//    //
//    // Control flags.
//    //
//
//    DWORD ContextFlags;
//    DWORD MxCsr;
//
//    //
//    // Segment Registers and processor flags.
//    //
//
//    WORD   SegCs;
//    WORD   SegDs;
//    WORD   SegEs;
//    WORD   SegFs;
//    WORD   SegGs;
//    WORD   SegSs;
//    DWORD EFlags;
//
//    //
//    // Debug registers
//    //
//
//    DWORD64 Dr0;
//    DWORD64 Dr1;
//    DWORD64 Dr2;
//    DWORD64 Dr3;
//    DWORD64 Dr6;
//    DWORD64 Dr7;
//
//    //
//    // Integer registers.
//    //
//
//    DWORD64 Rax;
//    DWORD64 Rcx;
//    DWORD64 Rdx;
//    DWORD64 Rbx;
//    DWORD64 Rsp;
//    DWORD64 Rbp;
//    DWORD64 Rsi;
//    DWORD64 Rdi;
//    DWORD64 R8;
//    DWORD64 R9;
//    DWORD64 R10;
//    DWORD64 R11;
//    DWORD64 R12;
//    DWORD64 R13;
//    DWORD64 R14;
//    DWORD64 R15;
//
//    //
//    // Program counter.
//    //
//
//    DWORD64 Rip;
//
//    //
//    // Floating point state.
//    //
//
//    union {
//        XMM_SAVE_AREA32 FltSave;
//        struct {
//            M128A Header[2];
//            M128A Legacy[8];
//            M128A Xmm0;
//            M128A Xmm1;
//            M128A Xmm2;
//            M128A Xmm3;
//            M128A Xmm4;
//            M128A Xmm5;
//            M128A Xmm6;
//            M128A Xmm7;
//            M128A Xmm8;
//            M128A Xmm9;
//            M128A Xmm10;
//            M128A Xmm11;
//            M128A Xmm12;
//            M128A Xmm13;
//            M128A Xmm14;
//            M128A Xmm15;
//        } DUMMYSTRUCTNAME;
//    } DUMMYUNIONNAME;
//
//    //
//    // Vector registers.
//    //
//
//    M128A VectorRegister[26];
//    DWORD64 VectorControl;
//
//    //
//    // Special debug control registers.
//    //
//
//    DWORD64 DebugControl;
//    DWORD64 LastBranchToRip;
//    DWORD64 LastBranchFromRip;
//    DWORD64 LastExceptionToRip;
//    DWORD64 LastExceptionFromRip;
//} CONTEXT, *PCONTEXT;