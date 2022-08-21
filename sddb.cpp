// SLIB
#include "sddb"

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

/* --------------------------------------------------------------------
CPP_EXCEPTION
PARAMS NUM: 4
PARAM0        SOME INTERNAL VALUE: 0x19930520
PARAM1      POINTER TO THROWN OBJ: 0x14FA30
PARAM2      POINTER TO OBJECT INF: 0x7FFE68B246B0 ---->
PARAM3  DLL/EXE THROWER HINSTANCE: 0x7FFE68AC0000
     +-----+
 --> |0    |
     +-----+
	 |4D0C |
     +-----+
	 |0    |
     +-----+     +-----+
     |646D0|---->|3    |
     +-----+     +-----+     +-----+
	             |646F0|---->|0    |
                 +-----+     +-----+     +----------------+
                             |84578|---->|0               |
                             +-----+     +----------------+
                                         |7FFE68B1C218    |
                                         +----------------+
                                         |5F74756F56413F2E|
                                         +----------------+
                                    ..... _ t u o V A ? .
									
.?AVout_of_range@std@@
CURRENT EXE HIN:  0x0000000140000000
UCRTBASE HIN:     0x00007FFE99D50000
VCRUNTIME140 HIN: 0x00007FFE95040000
STD MSVCP140 HIN: 0x00007FFE68AC0000
CONTINUE EXECUTION:  NOT POSSIBLE
INSTRUCTION ADDRESS: 0x00007FFE99F54FD9
-------------------------------------------------------------------- */

WORD def_color = getDefColors();
DWORD cur_def_sz = getDefCurSize();

void pexcept(_EXCEPTION_RECORD *er, HANDLE proc)
{
	if(proc == NULL)
	{
		proc = GetCurrentProcess();
	}
	
	static HANDLE heap = GetProcessHeap();
	
	DWORD c = er->ExceptionCode;
	
	std::cout << red << "<---" << def << std::endl;
	std::cout << "--------------------------------------------------------------------" << red << std::endl;
	switch(c)
	{
	case STILL_ACTIVE:
		std::cout << "STILL_ACTIVE" << std::endl;
		break;
	case EXCEPTION_ACCESS_VIOLATION:
		if(er->ExceptionInformation[0] == 0)
		{
			std::cout << "READ ";
		}
		else if(er->ExceptionInformation[0] == 1)
		{
			std::cout << "WRITE ";
		}
		else if(er->ExceptionInformation[0] == 8)
		{
			std::cout << "DEP "; // Data Execution Prevention
		}
		std::cout << "ACCESS_VIOLATION" << std::endl;
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		std::cout << "DATATYPE_MISALIGNMENT" << std::endl;
		break;
	case EXCEPTION_BREAKPOINT:
		std::cout << "BREAKPOINT" << std::endl;
		break;
	case EXCEPTION_SINGLE_STEP:
		std::cout << "SINGLE_STEP" << std::endl;
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		std::cout << "ARRAY_BOUNDS_EXCEEDED" << std::endl;
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		std::cout << "FLOAT_DENORMAL_OPERAND" << std::endl;
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		std::cout << "FLOAT_DIVIDE_BY_ZERO" << std::endl;
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		std::cout << "FLOAT_INEXACT_RESULT" << std::endl;
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		std::cout << "FLOAT_INVALID_OPERATION" << std::endl;
		break;
	case EXCEPTION_FLT_OVERFLOW:
		std::cout << "FLOAT_OVERFLOW" << std::endl;
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		std::cout << "FLOAT_STACK_CHECK" << std::endl;
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		std::cout << "FLOAT_UNDERFLOW" << std::endl;
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		std::cout << "INTEGER_DIVIDE_BY_ZERO" << std::endl;
		break;
	case EXCEPTION_INT_OVERFLOW:
		std::cout << "INTEGER_OVERFLOW" << std::endl;
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		std::cout << "PRIVILEGED_INSTRUCTION" << std::endl;
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		if(er->ExceptionInformation[0] == 0)
		{
			std::cout << "READ ";
		}
		else if(er->ExceptionInformation[0] == 1)
		{
			std::cout << "WRITE ";
		}
		else if(er->ExceptionInformation[0] == 8)
		{
			std::cout << "DEP "; // Data Execution Prevention
		}
		std::cout << "IN_PAGE_ERROR" << def << std::endl;
		std::cout << "DATA VADDRESS:       "	<< er->ExceptionInformation[1] << std::endl;
		std::cout << "NTSTATUS CAUSE:      "	<< er->ExceptionInformation[2] << std::endl;
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		std::cout << "ILLEGAL_INSTRUCTION" << std::endl;
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		std::cout << "NONCONTINUABLE_EXCEPTION" << std::endl;
		break;
	case EXCEPTION_STACK_OVERFLOW:
		std::cout << "STACK_OVERFLOW" << std::endl;
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		std::cout << "INVALID_DISPOSITION" << std::endl;
		break;
	case EXCEPTION_GUARD_PAGE:
		std::cout << "GUARD_PAGE_VIOLATION" << std::endl;
		break;
	case EXCEPTION_INVALID_HANDLE:
		std::cout << "INVALID_HANDLE" << std::endl;
		break;
	// case EXCEPTION_POSSIBLE_DEADLOCK:
		// std::cout << "POSSIBLE_DEADLOCK" << std::endl;
		// break;
	case CONTROL_C_EXIT:
		std::cout << "CONTROL_C_EXIT" << std::endl;
		break;
	case EXCEPTION_CPP_LOWERCASE:
	case EXCEPTION_CPP_UPPERCASE:
	{
		std::cout << "CPP_EXCEPTION" << def << std::endl;
		std::cout << "PARAMS NUM: " << er->NumberParameters << std::endl;
		for(ui64 i = 0; i < er->NumberParameters; ++i)
		{
			std::cout << std::dec << "PARAM" << std::setw(2) << std::left << i << " ";
			switch(i)
			{
			case 0:
				std::cout << "      SOME INTERNAL VALUE";
				break;
			case 1:
				std::cout << "    POINTER TO THROWN OBJ";
				break;
			case 2:
				std::cout << "    POINTER TO OBJECT INF";
				break;
			case 3:
				std::cout << "DLL/EXE THROWER HINSTANCE";
				break;
			}
			
			std::cout << ": 0x" << std::hex
				<< std::uppercase << er->ExceptionInformation[i] << std::endl;
		}

		ui64 hinst = er->ExceptionInformation[3];
		DWORD *obj_inf = (DWORD *)er->ExceptionInformation[2];
		// std::cout << std::hex	<< std::uppercase << obj_inf[0] << std::endl;
		// std::cout << obj_inf[1] << std::endl;
		// std::cout << obj_inf[2] << std::endl;
		// std::cout << obj_inf[3] << std::endl;
		// std::cout << "-----------------------------" << std::endl;
		obj_inf = (DWORD *)(hinst + obj_inf[3]);
		// std::cout << obj_inf[0] << std::endl;
		// std::cout << obj_inf[1] << std::endl;
		// std::cout << "-----------------------------" << std::endl;
		obj_inf = (DWORD *)(hinst + obj_inf[1]);
		// std::cout << obj_inf[0] << std::endl;
		// std::cout << obj_inf[1] << std::endl;
		// std::cout << "-----------------------------" << std::endl;
		ui64 *class_inf = (ui64 *)(hinst + obj_inf[1]);
		// std::cout << class_inf[0] << std::endl;
		// std::cout << class_inf[1] << std::endl;
		// std::cout << class_inf[2] << std::endl;
		char *class_name = (char *)(class_inf + 2);
		
		std::cout << "CPP EXPT CLASS NAME: " << class_name << std::hex << std::uppercase << std::endl;
		std::cout << "CURRENT EXE HIN:     " << "0x" << GetModuleHandle(NULL) << std::endl;
		std::cout << "UCRTBASE HIN:        " << "0x" << LoadLibrary(L"ucrtbase.dll") << std::endl;
		std::cout << "VCRUNTIME140 HIN:    " << "0x" << LoadLibrary(L"vcruntime140.dll") << std::endl;
		std::cout << "STD MSVCP140 HIN:    " << "0x" << LoadLibrary(L"msvcp140.dll") << std::endl;
	}	break;
	// CUSTOM EXCEPTIONS ----------------------------------------------------------------------------
	case EXCEPTION_TXTI_BUFF_OVERRUN:
	case EXCEPTION_TXTO_BUFF_OVERRUN:
	{
		std::cout << (c == EXCEPTION_TXTI_BUFF_OVERRUN ? "EXCEPTION_TXTI_BUFF_OVERRUN" :
			"EXCEPTION_TXTO_BUFF_OVERRUN") << def << std::endl;
		
		void *buff = NULL;
		if(er->ExceptionInformation[1] == EXCEPT_TXT_UTF8)
		{
			SIZE_T s = er->ExceptionInformation[2] + 1;
			buff = HeapAlloc(heap, 0, s);
			ReadProcessMemory(proc, (LPCVOID)er->ExceptionInformation[0], buff, s, NULL);
			std::cout << "   |TEXT|: |" << (const char *)buff << '|' << std::endl;
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
			
			std::wcout << "   |TEXT|: |" << (const wchar_t *)buff << '|' << std::endl;
		}
		HeapFree(heap, 0, buff);
		
		std::cout << "TEXT SIZE: " << std::dec << er->ExceptionInformation[2] << std::endl;
		std::cout << " POSITION: " << std::dec << er->ExceptionInformation[3] << std::endl;
	}	break;
	case EXCEPTION_T2I_NON_NUMBER:
	case EXCEPTION_H2I_NON_NUMBER:
	case EXCEPTION_T2I_OVERFLOW:
	case EXCEPTION_H2I_OVERFLOW:
	{
		if(c == EXCEPTION_T2I_NON_NUMBER)
		{
			std::cout << "EXCEPTION_T2I_NON_NUMBER";
		}
		else if(c == EXCEPTION_H2I_NON_NUMBER)
		{
			std::cout << "EXCEPTION_H2I_NON_NUMBER";
		}
		else if(c == EXCEPTION_T2I_OVERFLOW)
		{
			std::cout << "EXCEPTION_T2I_OVERFLOW";
		}
		else
		{
			std::cout << "EXCEPTION_H2I_OVERFLOW";
		}
		
		std::cout << def << std::endl;
		
		void *buff = NULL;
		if(er->ExceptionInformation[1] == EXCEPT_TXT_UTF8)
		{
			SIZE_T s = er->ExceptionInformation[2] + 1;
			buff = HeapAlloc(heap, 0, s);
			ReadProcessMemory(proc, (LPCVOID)er->ExceptionInformation[0], buff, s, NULL);
			std::cout << "|TEXT|: |" << (const char *)buff << '|' << std::endl;
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
			
			std::wcout << "|TEXT|: |" << (const wchar_t *)buff  << '|' << std::endl;
		}
		HeapFree(heap, 0, buff);
	}	break;
	default:
		std::cout << "UNKNOWN_EXCEPTION [" << std::hex << std::uppercase << c << "]" << std::endl;
		break;
	}
	
	std::cout << def << "CONTINUE EXECUTION:  ";
	if(er->ExceptionFlags == EXCEPTION_NONCONTINUABLE)
	{
		std::cout << red << "NOT";
	}
	else
	{
		std::cout << gre;
	}
	
	std::cout << "POSSIBLE" << def << std::endl;
	
	std::cout << "INSTRUCTION ADDRESS: 0x" << std::hex << std::uppercase
		<< er->ExceptionAddress << std::endl;
		
	if(er->ExceptionRecord != NULL)
	{
		std::cout << "CHAINED EXCEPTION!" << std::endl;
		pexcept(er->ExceptionRecord, proc);
	}
	else
	{
		std::cout << "--------------------------------------------------------------------" << std::endl;
	}
}

// Defined values for the exception filter expression
// EXCEPTION_EXECUTE_HANDLER      1
// EXCEPTION_CONTINUE_SEARCH      0
// EXCEPTION_CONTINUE_EXECUTION (-1)
int efilter(_EXCEPTION_POINTERS *ep)
{
	fkcursor();
	pexcept(ep->ExceptionRecord, NULL);
	std::cout << gre << "C" << def << "ONTINUE | ";
	std::cout << cay << "H" << def << "ANDLE | ";
	std::cout << red << "I" << def << "GNORE";
	//std::cout << "C ----> TRY TO EXECUTIE FAULTY INSTRUCTION AGAIN" << std::endl;
	//std::cout << "H ----> ABORT TRY BLOCK AND EXECUTE HANDLER, CONTINUE AFTER THE BLOCK" << std::endl;
	//std::cout << "I ----> IGNORE EXCEPTION AND PASS IT TO OTHER\\DEFAULT HANDLER" << std::endl;
	
	while(1)
	{
		int ch = _getch();
		okcursor();
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

LARGE_INTEGER sw_stime[0x20];
LARGE_INTEGER sw_etime[0x20];
SW_FUNC sw_fs[0x10];

ui64 sw_ns_per_tick;
LARGE_INTEGER sw_freq;

std::string swStr(ui64 i)
{
	swInit();
	
	ui64 s;
	ui64 ms;
	ui64 us;
	ui64 ns;

	ui64 timing = ui64(sw_etime[i].QuadPart - sw_stime[i].QuadPart);

	s = timing/sw_freq.QuadPart;
	ms = timing/(1000000/sw_ns_per_tick) - s * 1000;
	us = timing/(1000/sw_ns_per_tick) - s * 1000000 - ms * 1000;
	ns = timing * sw_ns_per_tick - s * 1000000000 - ms * 1000000 - us * 1000;
	// This math will overflow when timing reach 584.9424 years.
	// Should be totaly enough for profiling!
	
	std::stringstream ss;
	if(!s)
	{
		if(!ms)
		{
			if(!us)
			{
				ss << ns << " ns";
			}
			else
			{
				ss << us << '.' << ns << " us";
			}
		}
		else
		{
			ss << ms << '.' << us << '\'' << std::setw(3) << std::setfill('0') << ns << " ms";
		}
	}
	else
	{
		if(s < 60)
		{
			ss << s << '.' << ms << " s ";
		}
		else if(s < 3600)
		{
			ss << s/60 << ':' << s%60 << " m ";
		}
		else if(s < 86400)
		{
			ss << s/3600 << ':' << (s%3600)/60 << ':' << s%3600%60 << " h ";
		}
		else
		{
			ss << s/86400 << " d " << (s%86400)/3600 << " h ";
		}
	}

	return ss.str();
}

std::string swT2S(ui64 timing, ui64 ttot)
{
	swInit();
	
	ui64 s;
	ui64 ms;
	ui64 us;
	ui64 ns;

	s = timing/sw_freq.QuadPart;
	ms = timing/(1000000/sw_ns_per_tick) - s * 1000;
	us = timing/(1000/sw_ns_per_tick) - s * 1000000 - ms * 1000;
	ns = timing * sw_ns_per_tick - s * 1000000000 - ms * 1000000 - us * 1000;
	// This math will overflow when timing reach 584.9424 years.
	// Should be totaly enough for profiling!
	
	std::stringstream ssperc;
	float perc = ttot ? (float)timing/(float)ttot * 100.0f : 0.0f;
	ssperc << std::setprecision(2) << std::fixed << perc << "%";
	
	std::stringstream ss;
	ss << std::setw(8) << std::left << ssperc.str();
	
	if(!s)
	{
		if(!ms)
		{
			if(!us)
			{
				ss << ns << " ns";
			}
			else
			{
				ss << us << '.' << ns << " us";
			}
		}
		else
		{
			ss << ms << '.' << us << '\'' << std::setw(3) << std::setfill('0') << ns << " ms";
		}
	}
	else
	{
		if(s < 60)
		{
			ss << s << '.' << ms << " s ";
		}
		else if(s < 3600)
		{
			ss << s/60 << ':' << s%60 << " m ";
		}
		else if(s < 86400)
		{
			ss << s/3600 << ':' << (s%3600)/60 << ':' << s%3600%60 << " h ";
		}
		else
		{
			ss << s/86400 << " d " << (s%86400)/3600 << " h ";
		}
	}

	return ss.str();
}

void swDiff(ui64 i_left, ui64 i_right)
{
	ui64 timing_l = ui64(sw_etime[i_left].QuadPart - sw_stime[i_left].QuadPart);
	ui64 timing_r = ui64(sw_etime[i_right].QuadPart - sw_stime[i_right].QuadPart);
	
	std::stringstream ss;
	ss << "    x" << std::setprecision(2) << std::fixed;
	ss << (timing_l > timing_r ? (float)timing_l/(float)timing_r : (float)timing_r/(float)timing_l);
	
	std::cout << (timing_l > timing_r ? gre : red) << std::setw(12) << std::left << ss.str() << def << "    ";
}