 cl /EHsc hello.cpp
 
To compile a program that has additional source code files, enter them all on the command line, like:
cl /EHsc file1.cpp file2.cpp file3.cpp

The /EHsc command-line option instructs the compiler to enable standard C++ exception handling behavior. Without it, thrown exceptions can result in undestroyed objects and resource leaks.

When you supply additional source files, the compiler uses the first input file to create the program name. In this case, it outputs a program called file1.exe. To change the name to program1.exe, add an /out linker option:
cl /EHsc file1.cpp file2.cpp file3.cpp /link /out:program1.exe

And to catch more programming mistakes automatically, we recommend you compile by using either the /W3 or /W4 warning level option:
cl /W4 /EHsc file1.cpp file2.cpp file3.cpp /link /out:program1.exe


--------------------------------------IDEAL-NPP-MIN-COMPILER--------------------------------------

cl /Wall /diagnostics:caret /O2 /Oi /GL /Gy /D _WINDOWS /D _UNICODE /D UNICODE /D _CRT_SECURE_NO_WARNINGS /Gm- /EHsc /MD /GS- /Zc:wchar_t /Zc:forScope /Zc:inline /permissive- /nologo SOURCE.cpp /link /SUBSYSTEM:WINDOWS /LTCG /DYNAMICBASE:NO /MACHINE:X64

--------------------------------------------------------------------------------------------------
TO ADD MANIFEST, ADD IT TO RESOURCE.RC!
echo MANIFEST 24 "h2o.manifest">> ico\res.rc (RT_MANIFEST == 24)
--------------------------------------------------------------------------------------------------

// Link Is This Needed?
//kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
///MANIFEST "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'" /manifest:embed +++ DPI AWARE MB

DEF PROJECT RELEASE
cl /c /Zi /W3 /WX- /diagnostics:column /sdl /O2 /Oi /GL /D _CRT_SECURE_NO_WARNINGS /D NDEBUG /D _WINDOWS /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /permissive- /Fo"x64\Release\\" /Fd"x64\Release\vc143.pdb" /external:W3 /Gd /TP /FC /errorReport:prompt /nologo conmsgbox.cpp data.cpp main.cpp

DEF PROJECT DEBUG
cl /c /ZI /JMC /W3 /WX- /diagnostics:column /sdl /Od /D _CRT_SECURE_NO_WARNINGS /D _DEBUG /D _WINDOWS /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /permissive- /Fo"x64\Debug\\" /Fd"x64\Debug\vc143.pdb" /external:W3 /Gd /TP /FC /errorReport:prompt /nologo conmsgbox.cpp data.cpp main.cpp
1>conmsgbox.cpp

┌─────────────┬───────────┐
│   Release   │   Debug   │
├─────────────┼───────────┤
│ /Zi         │ /ZI /JMC  │
│ /O2 /Oi /GL │ /Od       │
│ /D NDEBUG   │ /D _DEBUG │
│ /MD /Gy     │ /MDd      │
└─────────────┴───────────┘

/Fo"x64\Release\\" /Fd"x64\Release\vc143.pdb"
/Fo"x64\Debug\\" /Fd"x64\Debug\vc143.pdb

NOTE: /D _CRT_SECURE_NO_WARNINGS --> I ADDED MANUALLY, DOES NOT EXIST IN EMPTY PROJECT!

Incremental Linker Version 14.32.31329.0
link "/OUT:C:\Users\pc\source\repos\shellsymlnk\x64\Release\shellsymlnk.exe" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'" /manifest:embed "/PDB:C:\Users\pc\source\repos\shellsymlnk\x64\Release\shellsymlnk.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG:incremental "/LTCGOUT:x64\Release\shellsymlnk.iobj" /TLBID:1 /DYNAMICBASE /NXCOMPAT "/IMPLIB:C:\Users\pc\source\repos\shellsymlnk\x64\Release\shellsymlnk.lib" /MACHINE:X64 x64\Release\shellsymlnk.res

Incremental Linker Version 14.32.31329.0
link "/OUT:C:\Users\pc\source\repos\shellsymlnk\x64\Debug\shellsymlnk.exe" /INCREMENTAL "/ILK:x64\Debug\shellsymlnk.ilk" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'" /manifest:embed /DEBUG "/PDB:C:\Users\pc\source\repos\shellsymlnk\x64\Debug\shellsymlnk.pdb" /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT "/IMPLIB:C:\Users\pc\source\repos\shellsymlnk\x64\Debug\shellsymlnk.lib" /MACHINE:X64 x64\Debug\shellsymlnk.res


┌────────────────────┬──────────────────────────┐
│ /OPT:REF /OPT:ICF  │ /INCREMENTAL "/ILK:path" │
├────────────────────┼──────────────────────────┤
│ /LTCG:incremental  │                          │
│ "/LTCGOUT:path"    │ "/PDB:path"              │
└────────────────────┴──────────────────────────┘


DLL
cl /c /Zi /W3 /WX- /diagnostics:column /sdl /O2 /Oi /GL /D _CRT_SECURE_NO_WARNINGS /D NDEBUG /D _CONSOLE /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /permissive- /Fo"x64\Release\\" /Fd"x64\Release\vc143.pdb" /external:W3 /Gd /TP /FC /errorReport:prompt hook.cpp

cl /c /ZI /JMC /W3 /WX- /diagnostics:column /sdl /Od /D _CRT_SECURE_NO_WARNINGS /D _DEBUG /D _CONSOLE /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /permissive- /Fo"x64\Debug\\" /Fd"x64\Debug\vc143.pdb" /external:W3 /Gd /TP /FC /errorReport:prompt hook.cpp


Incremental Linker Version 14.32.31329.0
link "/OUT:C:\Users\pc\source\repos\spyhook\x64\Release\hook.dll" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST "/MANIFESTUAC:level='asInvoker' uiAccess='false'" /manifest:embed /DEBUG "/PDB:C:\Users\pc\source\repos\spyhook\x64\Release\hook.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG:incremental "/LTCGOUT:x64\Release\hook.iobj" /TLBID:1 /DYNAMICBASE /NXCOMPAT "/IMPLIB:C:\Users\pc\source\repos\spyhook\x64\Release\hook.lib" /MACHINE:X64 /DLL x64\Release\hook.obj

Incremental Linker Version 14.32.31329.0
"/OUT:C:\Users\pc\source\repos\spyhook\x64\Debug\hook.dll" /INCREMENTAL "/ILK:x64\Debug\hook.ilk" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST "/MANIFESTUAC:level='asInvoker' uiAccess='false'" /manifest:embed /DEBUG "/PDB:C:\Users\pc\source\repos\spyhook\x64\Debug\hook.pdb" /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT "/IMPLIB:C:\Users\pc\source\repos\spyhook\x64\Debug\hook.lib" /MACHINE:X64 /DLL x64\Debug\hook.obj

------------------------------------------MAIN-ARGS-DETAILS------------------------------------------

/c compile only, no link

/D<name>{=|#}<text> define macro
You can use this symbol together with #if or #ifdef to compile source code conditionally. The symbol definition remains in effect until it's redefined in the code, or is undefined in the code by an #undef directive.
/D has the same effect as a #define directive at the beginning of a source code file. The difference is that /D strips quotation marks on the command line, and a #define directive keeps them. You can have whitespace between the /D and the symbol. There can't be whitespace between the symbol and the equals sign, or between the equals sign and any value assigned. By default, the value associated with a symbol is 1. For example, /D name is equivalent to /D name=1. In the example at the end of this article, the definition of TEST is shown to print 1.

/EH
a
Enables standard C++ stack unwinding. Catches both structured (asynchronous) and standard C++ (synchronous) exceptions when you use catch(...) syntax. /EHa overrides both /EHs and /EHc arguments.
s
Enables standard C++ stack unwinding. Catches only standard C++ exceptions when you use catch(...) syntax. Unless /EHc is also specified, the compiler assumes that functions declared as extern "C" may throw a C++ exception.
c
When used with /EHs, the compiler assumes that functions declared as extern "C" never throw a C++ exception. It has no effect when used with /EHa (that is, /EHca is equivalent to /EHa). /EHc is ignored if /EHs or /EHa aren't specified.
r
Tells the compiler to always generate runtime termination checks for all noexcept functions. By default, runtime checks for noexcept may be optimized away if the compiler determines the function calls only non-throwing functions. This option gives strict C++ conformance at the cost of some extra code. /EHr is ignored if /EHs or /EHa aren't specified.
-
Clears the previous option argument. For example, /EHsc- is interpreted as /EHs /EHc-, and is equivalent to /EHs.
/EH arguments may be specified separately or combined, in any order. If more than one instance of the same argument is specified, the last one overrides any earlier ones. For example, /EHr- /EHc /EHs is the same as /EHscr-, and /EHscr- /EHr has the same effect as /EHscr.

/Z7 enable old-style debug info place all db info in .obj files
/Zi enable debugging information placed in separate .pdb file that is timestamped in .obj files
/ZI enable Edit and Continue debug info same as /zi but can edit continue
/JMC Specifies compiler support for native Just My Code debugging in the Visual Studio debugger. This option supports the user settings that allow Visual Studio to step over system, framework, library, and other non-user calls, and to collapse those calls in the call stack window.

/O2 maximum optimizations (favor speed)
/Oi[-] enable intrinsic functions. If a function is an intrinsic, the code for that function is usually inserted inline, avoiding the overhead of a function call and allowing highly efficient machine instructions to be emitted for that function. An intrinsic is often faster than the equivalent inline assembly, because the optimizer has a built-in knowledge of how many intrinsics behave, so some optimizations can be available that are not available when inline assembly is used. Also, the optimizer can expand the intrinsic differently, align buffers differently, or make other adjustments depending on the context and arguments of the call.
/GL enable link-time code generation. Whole program optimization allows the compiler to perform optimizations with information on all modules in the program. Without whole program optimization, optimizations are performed on a per-module (compiland) basis. Requiers linker option /LTCG
/Od disable optimizations (default)
/Os (Favor Small Code) minimizes the size of EXEs and DLLs by instructing the compiler to favor size over speed. The compiler can reduce many C and C++ constructs to functionally similar sequences of machine code. Occasionally these differences offer tradeoffs of size versus speed. The /Os and /Ot options allow you to specify a preference for one over the other:
/Ot (Favor Fast Code) maximizes the speed of EXEs and DLLs by instructing the compiler to favor speed over size. /Ot is the default when optimizations are enabled. The compiler can reduce many C and C++ constructs to functionally similar sequences of machine code. Occasionally, these differences offer tradeoffs of size versus speed. The /Ot option is implied by the /O2 (Maximize speed) option. The /O2 option combines several options to produce faster code.

/MD link with MSVCRT.LIB (runtime linking with MVCRUNTIME.DLL)
/MDd link with MSVCRTD.LIB debug lib (runtime linking with debug MVCRUNTIME.DLL)
/Gy[-] separate functions for linker. /Gy ensures that all functions are packaged in their own COMDAT. The main advantage of this is that if you have identical functions the linker can collapse them all down into one actual piece of code ("COMDAT folding"). This can have very large impacts when you have many identical functions, which is often the case when you write modern C++ that is heavy on templates. Aside from the smaller size of the resulting executable due to COMDAT folding and elimination of unreferenced COMDATs, there's no other effect of /Gy. To be specific, it doesn't help in reducing interproject dependencies. The cost is a slight increase in compilation time (similar to other optimizer flags). Usually not something you'll notice. The purpose of a COMDAT section is to allow "duplicate" sections to be defined in multiple object files. Normally, if the same symbol is defined in multiple object files, the linker will report errors. This can cause problems for some C++ language features, like templates, that may instantiate the same symbols in different cpp files. COMDAT sections are used to get around this. When a section is marked as a COMDAT in an object file, it also specifies a flag that indicates how conflicts should be resolved. There are a bunch of options, including "just pick anyone you like", "make sure all dups. are the same size", "make sure all dups. have the same content", "pick the largest one", etc. See the COFF spec for a complete list. In any case, unlike what other answers said, there's no requirement, one way or the other, on what the contents of a COMDAT section has to be. They can contain one procedure, many procedures, data, or any combination of both code and data.

/Gm[-] enable minimal rebuild. /Gm is deprecated. It may not trigger a build for certain kinds of header file changes. You may safely remove this option from your projects. To improve build times, we recommend you use precompiled headers and incremental and parallel build options instead. For a list of deprecated compiler options, see the Deprecated and Removed Compiler Options section in Compiler Options Listed by Category. The compiler stores dependency information between source files and class definitions in the project's .idb file during the first compile. (Dependency information tells which source file is dependent on which class definition, and which .h file the definition is located in.) Subsequent compiles use the information stored in the .idb file to determine whether a source file needs to be compiled, even if it includes a modified .h file.
Note
Minimal rebuild relies on class definitions not changing between include files. Class definitions must be global for a project (there should be only one definition of a given class), because the dependency information in the .idb file is created for the entire project. If you have more than one definition for a class in your project, disable minimal rebuild. Because the incremental linker does not support the Windows metadata included in .obj files by using the /ZW (Windows Runtime Compilation) option, the /Gm option is incompatible with /ZW.

/GS[-] enable security checks. Detects some buffer overruns that overwrite a function's return address, exception handler address, or certain types of parameters. Causing a buffer overrun is a technique used by hackers to exploit code that does not enforce buffer size restrictions. /GS is on by default. If you expect your application to have no security exposure, use /GS-. For more information about suppressing buffer overrun detection, see safebuffers.

/Zc:wchar_t wchar_t is the native type, not a typedef. If /Zc:wchar_t is on, wchar_t is a keyword for a built-in integral type in code compiled as C++. If /Zc:wchar_t- (with a minus sign) is specified, or in code compiled as C, wchar_t is not a built-in type. Instead, wchar_t is defined as a typedef for unsigned short in the canonical header stddef.h. (The Microsoft implementation defines it in another header that is included by stddef.h and other standard headers.) We do not recommend /Zc:wchar_t- because the C++ standard requires that wchar_t be a built-in type. Using the typedef version can cause portability problems. If you upgrade from earlier versions of Visual Studio and encounter compiler error C2664 because the code is trying to implicitly convert a wchar_t to unsigned short, we recommend that you change the code to fix the error, instead of setting /Zc:wchar_t-. The /Zc:wchar_t option is on by default in C++ compilations, and is ignored in C compilations. The /permissive- option does not affect /Zc:wchar_t.

/Zc:forScope[-] enforce Standard C++ for scoping rules. Standard behavior is to let a for loop's initializer go out of scope after the for loop. Under /Zc:forScope- and /Ze, the for loop's initializer remains in scope until the local scope ends. The /Zc:forScope option is on by default. /Zc:forScope is not affected when the /permissive- option is specified. The /Zc:forScope- option is deprecated and will be removed in a future release. Use of /Zc:forScope- generates deprecation warning D9035. The compiler can compile C code in C++ compilation mode. This behavior is the default for source files that don't have a .c extension, and when the /Tp or /TP option is specified. In C++ compilation mode, the compiler supports those parts of the ISO C99 and C11 standards that have been incorporated into the C++ standard. Almost all C code is also valid C++ code. A small number of C keywords and code constructs aren't valid C++ code, or are interpreted differently in C++. The compiler behaves according to the C++ standard in these cases. In C++ compilation mode, the /Za option may cause unexpected behavior and isn't recommended.

inline[-] remove unreferenced function or data if it is WUT? Removes unreferenced data or functions that are COMDATs, or that only have internal linkage. Under /Zc:inline, the compiler specifies that translation units with inline data or functions must also include their definitions. When /Zc:inline is specified, the compiler doesn't emit symbol information for unreferenced COMDAT functions or data. Or, for data or functions that have internal linkage only. This optimization simplifies some of the work the linker does in release builds, or when you specify the /OPT:REF linker option. This compiler optimization can significantly reduce .obj file size and improve linker speeds. The compiler option isn't enabled when you disable optimizations (/Od). Or, when you specify /GL (Whole Program Optimization). By default, this option is off (/Zc:inline-) in command-line builds. The /permissive- option doesn't enable /Zc:inline. /OPT:REF eliminates functions and data that are never referenced; /OPT:NOREF keeps functions and data that are never referenced. When /OPT:REF is enabled, LINK removes unreferenced packaged functions and data, known as COMDATs. This optimization is known as transitive COMDAT elimination. The /OPT:REF option also disables incremental linking. Inlined functions and member functions defined inside a class declaration are always COMDATs. All of the functions in an object file are made into COMDATs if it is compiled by using the /Gy option. To place const data in COMDATs, you must declare it by using __declspec(selectany). For information about how to specify data for removal or folding, see selectany.

/permissive[-] enable some nonconforming code to compile (feature set subject to change) (on by default). You can use the /permissive- compiler option to specify standards-conforming compiler behavior. This option disables permissive behaviors, and sets the /Zc compiler options for strict conformance. In the IDE, this option also makes the IntelliSense engine underline non-conforming code. The /permissive- option uses the conformance support in the current compiler version to determine which language constructs are non-conforming. The /permissive- option sets the /Zc:referenceBinding, /Zc:strictStrings, /Zc:rvalueCast and /Zc:ternary  options to conforming behavior.

/Gd, the default setting, specifies the __cdecl calling convention for all functions except C++ member functions and functions that are marked __stdcall, __fastcall, or __vectorcall.
/Gr specifies the __fastcall calling convention for all functions except C++ member functions, functions named main, and functions that are marked __cdecl, __stdcall, or __vectorcall. All __fastcall functions must have prototypes. This calling convention is only available in compilers that target x86, and is ignored by compilers that target other architectures.
/Gz specifies the __stdcall calling convention for all functions except C++ member functions, functions named main, and functions that are marked __cdecl, __fastcall, or __vectorcall. All __stdcall functions must have prototypes. This calling convention is only available in compilers that target x86, and is ignored by compilers that target other architectures.
/Gv specifies the __vectorcall calling convention for all functions except C++ member functions, functions named main, functions with a vararg variable argument list, or functions that are marked with a conflicting __cdecl, __stdcall, or __fastcall attribute. This calling convention is only available on x86 and x64 architectures that support /arch:SSE2 and above, and is ignored by compilers that target the ARM architecture.

/sdl enable additional security features and warnings. Enables recommended Security Development Lifecycle (SDL) checks. These checks change security-relevant warnings into errors, and set additional secure code-generation features. /sdl enables a superset of the baseline security checks provided by /GS and overrides /GS-. By default, /sdl is off. /sdl- disables the additional security checks.
When /sdl is enabled, the compiler generates code that does these checks at run time:
Enables the strict mode of /GS run-time buffer overrun detection, equivalent to compiling with #pragma strict_gs_check(push, on).
Does limited pointer sanitization. In expressions that don't involve dereferences and in types that have no user-defined destructor, pointer references are set to a non-valid address after a call to delete. This sanitization helps to prevent the reuse of stale pointer references.
Initializes class member pointers. Automatically initializes class members of pointer type to nullptr on object instantiation (before the constructor runs). It helps prevent the use of uninitialized pointers that the constructor doesn't explicitly initialize.

/FC use full pathnames in diagnostics. Causes the compiler to display the full path of source code files passed to the compiler in diagnostics. Without /FC, the compiler output looks similar to this diagnostic text:
compiler_option_FC.cpp(5): error C2143: syntax error: missing ';' before '}'
With /FC, the compiler output looks similar to this diagnostic text:
C:\test\compiler_option_FC.cpp(5): error C2143: syntax error: missing ';' before '}'
/FC is also needed if you want to see the full path of a file name when using the __FILE__ macro.

/fp:contract
The /fp:contract option allows the compiler to generate floating-point contractions when you specify the /fp:precise and /fp:except options. A contraction is a machine instruction that combines floating-point operations, such as Fused-Multiply-Add (FMA). FMA, defined as a basic operation by IEEE-754, doesn't round the intermediate product before the addition, so the result can differ from separate multiplication and addition operations. Since it's implemented as a single instruction, it can be faster than separate instructions. The speed comes at a cost of bitwise exact results, and an inability to examine the intermediate value. By default, the /fp:fast option enables /fp:contract. The /fp:contract option isn't compatible with /fp:strict. The /fp:contract option is new in Visual Studio 2022.
/fp:precise
By default, the compiler uses /fp:precise behavior. Under /fp:precise, the compiler preserves the source expression ordering and rounding properties of floating-point code when it generates and optimizes object code for the target machine. The compiler rounds to source code precision at four specific points during expression evaluation: at assignments, typecasts, when floating-point arguments get passed to a function call, and when a function call returns a floating-point value. Intermediate computations may be performed at machine precision. Typecasts can be used to explicitly round intermediate computations. The compiler doesn't perform algebraic transformations on floating-point expressions, such as reassociation or distribution, unless it can guarantee the transformation produces a bitwise identical result. Expressions that involve special values (NaN, +infinity, -infinity, -0.0) are processed according to IEEE-754 specifications. For example, x != x evaluates to true if x is NaN. Floating-point contractions aren't generated by default under /fp:precise. This behavior is new in Visual Studio 2022. Previous compiler versions could generate contractions by default under /fp:precise. The compiler generates code intended to run in the default floating-point environment. It also assumes the floating-point environment isn't accessed or modified at runtime. That is, it assumes the code: leaves floating-point exceptions masked, doesn't read or write floating-point status registers, and doesn't change rounding modes. If your floating-point code doesn't depend on the order of operations and expressions in your floating-point statements (for example, if you don't care whether a * b + a * c is computed as (b + c) * a or 2 * a as a + a), consider the /fp:fast option, which can produce faster, more efficient code. If your code both depends on the order of operations and expressions, and accesses or alters the floating-point environment (for example, to change rounding modes or to trap floating-point exceptions), use /fp:strict.
/fp:strict
The /fp:strict has behavior similar to /fp:precise, that is, the compiler preserves the source ordering and rounding properties of floating-point code when it generates and optimizes object code for the target machine, and observes the standard when handling special values. The program may also safely access or modify the floating-point environment at runtime. Under /fp:strict, the compiler generates code that allows the program to safely unmask floating-point exceptions, read or write floating-point status registers, or change rounding modes. It rounds to source code precision at four specific points during expression evaluation: at assignments, typecasts, when floating-point arguments get passed to a function call, and when a function call returns a floating-point value. Intermediate computations may be performed at machine precision. Typecasts can be used to explicitly round intermediate computations. The compiler doesn't make any algebraic transformations on floating-point expressions, such as reassociation or distribution, unless it can guarantee the transformation produces a bitwise identical result. Expressions that involve special values (NaN, +infinity, -infinity, -0.0) are processed according to IEEE-754 specifications. For example, x != x evaluates to true if x is NaN. Floating-point contractions aren't generated under /fp:strict. /fp:strict is computationally more expensive than /fp:precise because the compiler must insert extra instructions to trap exceptions and allow programs to access or modify the floating-point environment at runtime. If your code doesn't use this capability, but requires source code ordering and rounding, or relies on special values, use /fp:precise. Otherwise, consider using /fp:fast, which can produce faster and smaller code.
/fp:fast
The /fp:fast option allows the compiler to reorder, combine, or simplify floating-point operations to optimize floating-point code for speed and space. The compiler may omit rounding at assignment statements, typecasts, or function calls. It may reorder operations or make algebraic transforms, for example, by use of associative and distributive laws. It may reorder code even if such transformations result in observably different rounding behavior. Because of this enhanced optimization, the result of some floating-point computations may differ from the ones produced by other /fp options. Special values (NaN, +infinity, -infinity, -0.0) may not be propagated or behave strictly according to the IEEE-754 standard. Floating-point contractions may be generated under /fp:fast. The compiler is still bound by the underlying architecture under /fp:fast, and more optimizations may be available through use of the /arch option. Under /fp:fast, the compiler generates code intended to run in the default floating-point environment and assumes the floating-point environment isn't accessed or modified at runtime. That is, it assumes the code: leaves floating-point exceptions masked, doesn't read or write floating-point status registers, and doesn't change rounding modes. /fp:fast is intended for programs that don't require strict source code ordering and rounding of floating-point expressions, and don't rely on the standard rules for handling special values such as NaN. If your floating-point code requires preservation of source code ordering and rounding, or relies on standard behavior of special values, use /fp:precise. If your code accesses or modifies the floating-point environment to change rounding modes, unmask floating-point exceptions, or check floating-point status, use /fp:strict.
/fp:except
The /fp:except option generates code to ensures that any unmasked floating-point exceptions are raised at the exact point at which they occur, and that no other floating-point exceptions are raised. By default, the /fp:strict option enables /fp:except, and /fp:precise doesn't. The /fp:except option isn't compatible with /fp:fast. The option can be explicitly disabled by use of /fp:except-.

/ZW enable WinRT language extensions. FREAKIN BS! Compiles source code to support Microsoft C++ component extensions C++/CX for the creation of Universal Windows Platform (UWP) apps (BOOOO!!!). When you use /ZW to compile, always specify /EHsc as well.

/J default char type is unsigned. Changes the default char type from signed char to unsigned char, and the char type is zero-extended when it is widened to an int type. If a char value is explicitly declared as signed, the /J option does not affect it, and the value is sign-extended when it is widened to an int type. The /J option defines _CHAR_UNSIGNED, which is used with #ifndef in the LIMITS.h file to define the range of the default char type. ANSI C and C++ do not require a specific implementation of the char type. This option is useful when you are working with character data that will eventually be translated into a language other than English.


/w 	Suppresses all compiler warnings.
/W0
/W1
/W2
/W3
/W4 	Specifies the level of warnings to be generated by the compiler. Valid warning levels range from 0 to 4:
/W0 suppresses all warnings. It's equivalent to /w.
/W1 displays level 1 (severe) warnings. /W1 is the default setting in the command-line compiler.
/W2 displays level 1 and level 2 (significant) warnings.
/W3 displays level 1, level 2, and level 3 (production quality) warnings. /W3 is the default setting in the IDE.
/W4 displays level 1, level 2, and level 3 warnings, and all level 4 (informational) warnings that aren't off by default. We recommend that you use this option to provide lint-like warnings. For a new project, it may be best to use /W4 in all compilations. This option helps ensure the fewest possible hard-to-find code defects.
/Wall 	Displays all warnings displayed by /W4 and all other warnings that /W4 doesn't include—for example, warnings that are off by default. For more information, see Compiler warnings that are off by default.
/Wv[:version] 	Displays only warnings introduced in the version compiler version and earlier. You can use this option to suppress new warnings in code when you migrate to a newer version of the compiler. It lets you maintain your existing build process while you fix them. The optional parameter version takes the form nn[.mm[.bbbbb]], where nn is the major version number, mm is the optional minor version number, and bbbbb is the optional build number of the compiler. For example, use /Wv:17 to display only warnings introduced in Visual Studio 2012 (major version 17) or earlier. That is, it displays warnings from any version of the compiler that has a major version number of 17 or less. It suppresses warnings introduced in Visual Studio 2013 (major version 18) and later. By default, /Wv uses the current compiler version number, and no warnings are suppressed. For information about which warnings are suppressed by compiler version, see Compiler warnings by compiler version.
/WX 	Treats all compiler warnings as errors. For a new project, it may be best to use /WX in all compilations; resolving all warnings ensures the fewest possible hard-to-find code defects.
The linker also has a /WX option. For more information, see /WX (Treat linker warnings as errors).
The following options are mutually exclusive with each other. The last option that's specified from this group is the one applied:

The /external compiler options let you specify compiler diagnostic behavior for certain header files. "External" headers are the natural complement of "Just my code": Header files such as system files or third-party library files that you can't or don't intend to change. Since you aren't going to change these files, you may decide it isn't useful to see diagnostic messages from the compiler about them. The /external compiler options give you control over these warnings.
The /external compiler options are available starting in Visual Studio 2017 version 15.6. In versions of Visual Studio before Visual Studio 2019 version 16.10, the /external options require you also set the /experimental:external compiler option.
/w1nnnn
/w2nnnn
/w3nnnn
/w4nnnn 	Sets the warning level for the warning number specified by nnnn. These options let you change the compiler behavior for that warning when a specific warning level is set. You can use these options in combination with other warning options to enforce your own coding standards for warnings, rather than the default ones provided by Visual Studio.
For example, /w34326 causes C4326 to be generated as a level 3 warning instead of level 1. If you compile by using both the /w34326 option and the /W2 option, warning C4326 isn't generated.
/wdnnnn 	Suppresses the compiler warning that is specified by nnnn.
For example, /wd4326 suppresses compiler warning C4326.
/wennnn 	Treats the compiler warning that is specified by nnnn as an error.
For example, /we4326 causes warning number C4326 to be treated as an error by the compiler.
/wonnnn 	Reports the compiler warning that is specified by nnnn only once.
For example, /wo4326 causes warning C4326 to be reported only once, the first time it's encountered by the compiler.
/experimental:external
Enables the external headers options. This option isn't required in Visual Studio 2019 version 16.10 and later.
/external:anglebrackets
Treats all headers included by #include <header>, where the header file is enclosed in angle brackets (< >), as external headers.
/external:I path
Defines a root directory that contains external headers. All recursive subdirectories of path are considered external, but only the path value is added to the list of directories the compiler searches for include files. The space between /external:I and path is optional. Directories that include spaces must be enclosed in double quotes. A directory may be an absolute path or a relative path.
/external:env:var
Specifies the name of an environment variable var that holds a semicolon-separated list of external header directories. It's useful for build systems that rely on environment variables such as INCLUDE, which you use to specify the list of external include files. Or, CAExcludePath, for files that shouldn't be analyzed by /analyze. For example, you can specify /external:env:INCLUDE to make every directory in INCLUDE an external header directory at once. It's the same as using /external:I to specify the individual directories, but much less verbose. There should be no space between var and /external:env:.
/external:Wn
This option sets the default warning level to n (a value from 0 to 4) for external headers. For example, /external:W0 effectively turns off warnings for external headers. If this option isn't specified, the compiler issues command line warning D9007 for other /external options. Those options are ignored, because they would have no effect.
The /external:Wn option has an effect similar to wrapping an included header in a #pragma warning directive:
C++
#pragma warning (push, 0)
// the global warning level is now 0 here
#include <external_header>
#pragma warning (pop)
/external:templates-
Allows warnings from external headers when they occur in a template that's instantiated in your code.

/FA[scu] configure assembly listing
/Fa[file] name assembly listing file
The /FA compiler option generates an assembler listing file for each translation unit in the compilation, which generally corresponds to a C or C++ source file. By default, only assembler is included in the listing file, which is encoded as ANSI. The optional c, s, and u arguments to /FA control whether machine code or source code are output together with the assembler listing, and whether the listing is encoded as UTF-8.
By default, each listing file gets the same base name as the source file, and has an .asm extension. When machine code is included by using the c option, the listing file has a .cod extension. You can change the name and extension of the listing file and the directory where it's created by using the /Fa option.
/FA arguments
none
Only assembler language is included in the listing.
c
Optional. Includes machine code in the listing.
s
Optional. Includes source code in the listing.
u
Optional. Encodes the listing file in UTF-8 format, and includes a byte order marker. By default, the file is encoded as ANSI. Use u to create a listing file that displays correctly on any system, or if you're using Unicode source code files as input to the compiler.
If both s and u are specified, and if a source code file uses a Unicode encoding other than UTF-8, then the code lines in the .asm file may not display correctly.
/Fa argument
none
One source.asm file is created for each source code file in the compilation.
filename
The compiler places a listing file named filename.asm in the current directory. This argument form is only valid when compiling a single source code file.
filename.extension
The compiler places a listing file named filename.extension in the current directory. This argument form is only valid when compiling a single source code file.
directory\
The compiler creates one source_file.asm file for each source code file in the compilation. It's placed in the specified directory. The trailing backslash is required. Only paths on the current disk are allowed.
directory\filename
A listing file named filename.asm is placed in the specified directory. This argument form is only valid when compiling a single source code file.
directory\filename.extension
A listing file named filename.extension is placed in the specified directory. This argument form is only valid when compiling a single source code file.

--------------------------------------------LINKER-ARGS-DETAILS--------------------------------------------

/SUBSYSTEM:{BOOT_APPLICATION|CONSOLE|EFI_APPLICATION|
                  EFI_BOOT_SERVICE_DRIVER|EFI_ROM|EFI_RUNTIME_DRIVER|
                  NATIVE|POSIX|WINDOWS|WINDOWSCE}[,#[.##]]
BOOT_APPLICATION
An application that runs in the Windows boot environment. For more information about boot applications, see About BCD.
CONSOLE
Win32 character-mode application. The operating system provides a console for console applications. If main or wmain is defined for native code, int main(array<String ^> ^) is defined for managed code, or you build the application completely by using /clr:safe, CONSOLE is the default.
EFI_APPLICATION
EFI_BOOT_SERVICE_DRIVER
EFI_ROM
EFI_RUNTIME_DRIVER
The Extensible Firmware Interface subsystems. For more information, see the UEFI specification. For examples, see the Intel UEFI Driver and Application Tool Resources. The minimum version and default version is 1.0.
NATIVE
Kernel mode drivers for Windows NT. This option is normally reserved for Windows system components. If /DRIVER:WDM is specified, NATIVE is the default.
POSIX
Application that runs with the POSIX subsystem in Windows NT.
WINDOWS
The application doesn't require a console, probably because it creates its own windows for interaction with the user. If WinMain or wWinMain is defined for native code, or WinMain(HINSTANCE *, HINSTANCE *, char *, int) or wWinMain(HINSTANCE *, HINSTANCE *, wchar_t *, int) is defined for managed code, WINDOWS is the default.
major and minor
(Optional) Specify the minimum required version of the subsystem. The arguments are decimal numbers in the range 0 through 65,535. There are no upper bounds for version numbers.

/OPT:REF
Eliminates functions and data that are never referenced; /OPT:NOREF keeps functions and data that are never referenced. When /OPT:REF is enabled, LINK removes unreferenced packaged functions and data, known as COMDATs. This optimization is known as transitive COMDAT elimination. The /OPT:REF option also disables incremental linking. Inlined functions and member functions defined inside a class declaration are always COMDATs. All of the functions in an object file are made into COMDATs if it is compiled by using the /Gy option. To place const data in COMDATs, you must declare it by using __declspec(selectany). For information about how to specify data for removal or folding, see selectany. By default, /OPT:REF is enabled by the linker unless /OPT:NOREF or /DEBUG is specified. To override this default and keep unreferenced COMDATs in the program, specify /OPT:NOREF. You can use the /INCLUDE option to override the removal of a specific symbol.

/OPT:ICF
Use ICF[=iterations] to perform identical COMDAT folding. Redundant COMDATs can be removed from the linker output. The optional iterations parameter specifies the number of times to traverse the symbols for duplicates. The default number of iterations is 1. Additional iterations may locate more duplicates that are uncovered through folding in the previous iteration. By default, /OPT:ICF is enabled by the linker unless /OPT:NOICF or /DEBUG is specified. To override this default and prevent COMDATs from being folded in the program, specify /OPT:NOICF.

/LTCG[:{INCREMENTAL|NOSTATUS|OFF|STATUS|}]
Use /LTCG to perform whole-program optimization, or to create profile-guided optimization (PGO) instrumentation, perform training, and create profile-guided optimized builds.
INCREMENTAL
(Optional) Specifies that the linker only applies whole program optimization or link-time code generation (LTCG) to files affected by an edit, instead of the entire project. By default, this flag isn't set when /LTCG is specified, and the entire project is linked by using whole program optimization.
/LTCG is implied with /GL.
The linker invokes link-time code generation if it's passed a module that was compiled by using /GL or an MSIL module (see .netmodule Files as Linker Input). If you don't explicitly specify /LTCG when you pass /GL or MSIL modules to the linker, the linker eventually detects this situation and restarts the link by using /LTCG. Explicitly specify /LTCG when you pass /GL and MSIL modules to the linker for the fastest possible build performance.For even faster performance, use /LTCG:INCREMENTAL. This option tells the linker to reoptimize only the files affected by a source file change, instead of the entire project. This option can significantly reduce the link time required. This option isn't the same option as incremental linking.
/LTCG isn't valid for use with /INCREMENTAL.
When /LTCG is used to link modules compiled by using /Og, /O1, /O2, or /Ox, the following optimizations are performed:
Cross-module inlining
Interprocedural register allocation (64-bit operating systems only)
Custom calling convention (x86 only)
Small TLS displacement (x86 only)
Stack double alignment (x86 only)
Improved memory disambiguation (better interference information for global variables and input parameters)
The linker determines which optimizations were used to compile each function and applies the same optimizations at link time.
Using /LTCG and /O2 causes double-alignment optimization.
If /LTCG and /O1 are specified, double alignment isn't performed. If most of the functions in an application are compiled for speed, with a few functions compiled for size (for example, by using the optimize pragma), the compiler double-aligns the functions that are optimized for size if they call functions that require double alignment. If the compiler can identify all of the call sites of a function, the compiler ignores explicit calling-convention modifiers on a function and tries to optimize the function's calling convention:
pass parameters in registers
reorder parameters for alignment
remove unused parameters
If a function is called through a function pointer, or if a function is called from outside a module that is compiled by using /GL, the compiler doesn't attempt to optimize the function's calling convention.

/TLBID:id
id: A user-specified value for a linker-created type library. It overrides the default resource ID of 1.
Remarks
When compiling a program that uses attributes, the linker will create a type library. The linker will assign a resource ID of 1 to the type library. If this resource ID conflicts with one of your existing resources, you can specify another ID with /TLBID. The range of values that you can pass to id is 1 to 65535.

/DYNAMICBASE[:NO]
Specifies whether to generate an executable image that can be randomly rebased at load time by using the address space layout randomization (ASLR) feature of Windows that was first available in Windows Vista. The /DYNAMICBASE option modifies the header of an executable image, a .dll or .exe file, to indicate whether the application should be randomly rebased at load time, and enables virtual address allocation randomization, which affects the virtual memory location of heaps, stacks, and other operating system allocations. The /DYNAMICBASE option applies to both 32-bit and 64-bit images. ASLR is supported on Windows Vista and later operating systems. The option is ignored by earlier operating systems. By default, /DYNAMICBASE is enabled. To disable this option, use /DYNAMICBASE:NO. The /DYNAMICBASE option is required for the /HIGHENTROPYVA option to have an effect. Because ASLR can't be disabled on ARM, ARM64, or ARM64EC architectures, /DYNAMICBASE:NO isn't supported for these targets.

/NXCOMPAT[:NO]
Indicates that an executable was tested to be compatible with the Windows Data Execution Prevention feature.By default, /NXCOMPAT is on. /NXCOMPAT:NO can be used to explicitly specify an executable as incompatible with Data Execution Prevention. Data Execution Prevention (DEP) is a system-level memory protection feature that is built into the operating system starting with Windows XP and Windows Server 2003. DEP enables the system to mark one or more pages of memory as non-executable. Marking memory regions as non-executable means that code cannot be run from that region of memory, which makes it harder for the exploitation of buffer overruns. DEP prevents code from being run from data pages such as the default heap, stacks, and memory pools. If an application attempts to run code from a data page that is protected, a memory access violation exception occurs, and if the exception is not handled, the calling process is terminated. DEP is not intended to be a comprehensive defense against all exploits; it is intended to be another tool that you can use to secure your application.

/MACHINE:{ARM|ARM64|ARM64EC|EBC|X64|X86}
The /MACHINE option specifies the target platform for the program. Usually, you don't have to specify the /MACHINE option. LINK infers the machine type from the .obj files. However, in some circumstances, LINK cannot determine the machine type and issues a linker tools error LNK1113. If such an error occurs, specify /MACHINE.

/ENTRY:symbol
function
A function that specifies a user-defined starting address for an .exe file or DLL. The /ENTRY option specifies an entry point function as the starting address for an .exe file or DLL. The function must be defined to use the __stdcall calling convention. The parameters and return value depend on if the program is a console application, a windows application or a DLL. It is recommended that you let the linker set the entry point so that the C run-time library is initialized correctly, and C++ constructors for static objects are executed.
mainCRTStartup (or wmainCRTStartup)			An application that uses /SUBSYSTEM:CONSOLE; calls main (or wmain)
WinMainCRTStartup (or wWinMainCRTStartup)	An application that uses /SUBSYSTEM:WINDOWS; calls WinMain (or wWinMain), which must be defined to use __stdcall
_DllMainCRTStartup							A DLL; calls DllMain if it exists, which must be defined to use __stdcall


 
usage: cl [ option... ] filename... [ /link linkoption... ]
						C/C++ COMPILER OPTIONS


                              -OPTIMIZATION-

/O1 maximum optimizations (favor space) /O2 maximum optimizations (favor speed)
/Ob<n> inline expansion (default n=0)   /Od disable optimizations (default)
/Og enable global optimization          /Oi[-] enable intrinsic functions
/Os favor code space                    /Ot favor code speed
/Ox optimizations (favor speed)
/favor:<blend|AMD64|INTEL64|ATOM> select processor to optimize for, one of:
    blend - a combination of optimizations for several different x64 processors
    AMD64 - 64-bit AMD processors
    INTEL64 - Intel(R)64 architecture processors
    ATOM - Intel(R) Atom(TM) processors

                             -CODE GENERATION-

/Gu[-] ensure distinct functions have distinct addresses
/Gw[-] separate global variables for linker
/GF enable read-only string pooling     /Gm[-] enable minimal rebuild
/Gy[-] separate functions for linker    /GS[-] enable security checks
/GR[-] enable C++ RTTI                  /GX[-] enable C++ EH (same as /EHsc)
/guard:cf[-] enable CFG (control flow guard)
/guard:ehcont[-] enable EH continuation metadata (CET)
/EHs enable C++ EH (no SEH exceptions)  /EHa enable C++ EH (w/ SEH exceptions)
/EHc extern "C" defaults to nothrow
/EHr always generate noexcept runtime termination checks
/fp:<contract|except[-]|fast|precise|strict> choose floating-point model:
    contract - consider floating-point contractions when generating code
    except[-] - consider floating-point exceptions when generating code
    fast - "fast" floating-point model; results are less predictable
    precise - "precise" floating-point model; results are predictable
    strict - "strict" floating-point model (implies /fp:except)
/Qfast_transcendentals generate inline FP intrinsics even with /fp:except
/Qspectre[-] enable mitigations for CVE 2017-5753
/Qpar[-] enable parallel code generation
/Qpar-report:1 auto-parallelizer diagnostic; indicate parallelized loops
/Qpar-report:2 auto-parallelizer diagnostic; indicate loops not parallelized
/Qvec-report:1 auto-vectorizer diagnostic; indicate vectorized loops
/Qvec-report:2 auto-vectorizer diagnostic; indicate loops not vectorized
/GL[-] enable link-time code generation
/volatile:<iso|ms> choose volatile model:
    iso - Acquire/release semantics not guaranteed on volatile accesses
    ms  - Acquire/release semantics guaranteed on volatile accesses
/GA optimize for Windows Application    /Ge force stack checking for all funcs
/Gs[num] control stack checking calls   /Gh enable _penter function call
/GH enable _pexit function call         /GT generate fiber-safe TLS accesses
/RTC1 Enable fast checks (/RTCsu)       /RTCc Convert to smaller type checks
/RTCs Stack Frame runtime checking      /RTCu Uninitialized local usage checks
/clr[:option] compile for common language runtime, where option is:
    pure : produce IL-only output file (no native executable code)
    safe : produce IL-only verifiable output file
    netcore : produce assemblies targeting .NET Core runtime
    noAssembly : do not produce an assembly
    nostdlib : ignore the system .NET framework directory when searching for assemblies
    nostdimport : do not import any required assemblies implicitly
    initialAppDomain : enable initial AppDomain behavior of Visual C++ 2002
    implicitKeepAlive- : turn off implicit emission of System::GC::KeepAlive(this)
/fsanitize=address Enable address sanitizer codegen
/homeparams Force parameters passed in registers to be written to the stack
/GZ Enable stack checks (/RTCs)         /Gv __vectorcall calling convention
/arch:<AVX|AVX2|AVX512> minimum CPU architecture requirements, one of:
   AVX - enable use of instructions available with AVX-enabled CPUs
   AVX2 - enable use of instructions available with AVX2-enabled CPUs
   AVX512 - enable use of instructions available with AVX-512-enabled CPUs
/QIntel-jcc-erratum enable mitigations for Intel JCC erratum
/Qspectre-load Enable spectre mitigations for all instructions which load memory
/Qspectre-load-cf Enable spectre mitigations for all control-flow instructions which load memory
/fpcvt:<IA|BC> FP to unsigned integer conversion compatibility
   IA - results compatible with VCVTTSD2USI instruction
   BC - results compatible with VS2017 and earlier compiler

                              -OUTPUT FILES-

/Fa[file] name assembly listing file    /FA[scu] configure assembly listing
/Fd[file] name .PDB file                /Fe<file> name executable file
/Fm[file] name map file                 /Fo<file> name object file
/Fp<file> name precompiled header file  /Fr[file] name source browser file
/FR[file] name extended .SBR file       /Fi[file] name preprocessed file
/Fd: <file> name .PDB file              /Fe: <file> name executable file
/Fm: <file> name map file               /Fo: <file> name object file
/Fp: <file> name .PCH file              /FR: <file> name extended .SBR file
/Fi: <file> name preprocessed file
/Ft<dir> location of the header files generated for #import
/doc[file] process XML documentation comments and optionally name the .xdc file

                              -PREPROCESSOR-

/AI<dir> add to assembly search path    /FU<file> forced using assembly/module
/C don't strip comments                 /D<name>{=|#}<text> define macro
/E preprocess to stdout                 /EP preprocess to stdout, no #line
/P preprocess to file                   /Fx merge injected code to file
/FI<file> name forced include file      /U<name> remove predefined macro
/u remove all predefined macros         /I<dir> add to include search path
/X ignore "standard places"
/PH generate #pragma file_hash when preprocessing
/PD print all macro definitions

                                -LANGUAGE-

/std:<c++14|c++17|c++20|c++latest> C++ standard version
    c++14 - ISO/IEC 14882:2014 (default)
    c++17 - ISO/IEC 14882:2017
    c++20 - ISO/IEC 14882:2020
    c++latest - latest draft standard (feature set subject to change)
/permissive[-] enable some nonconforming code to compile (feature set subject to change) (on by default)
/Ze enable extensions (default)         /Za disable extensions
/ZW enable WinRT language extensions    /Zs syntax check only
/Zc:arg1[,arg2] C++ language conformance, where arguments can be:
  forScope[-]           enforce Standard C++ for scoping rules
  wchar_t[-]            wchar_t is the native type, not a typedef
  auto[-]               enforce the new Standard C++ meaning for auto
  trigraphs[-]          enable trigraphs (off by default)
  rvalueCast[-]         enforce Standard C++ explicit type conversion rules
  strictStrings[-]      disable string-literal to [char|wchar_t]*
                        conversion (off by default)
  implicitNoexcept[-]   enable implicit noexcept on required functions
  threadSafeInit[-]     enable thread-safe local static initialization
  inline[-]             remove unreferenced function or data if it is
                        COMDAT or has internal linkage only (off by default)
  sizedDealloc[-]       enable C++14 global sized deallocation
                        functions (on by default)
  throwingNew[-]        assume operator new throws on failure (off by default)
  referenceBinding[-]   a temporary will not bind to an non-const
                        lvalue reference (off by default)
  twoPhase-             disable two-phase name lookup
  ternary[-]            enforce C++11 rules for conditional operator (off by default)
  noexceptTypes[-]      enforce C++17 noexcept rules (on by default in C++17 or later)
  alignedNew[-]         enable C++17 alignment of dynamically allocated objects (on by default)
  hiddenFriend[-]       enforce Standard C++ hidden friend rules (implied by /permissive-)
  externC[-]            enforce Standard C++ rules for 'extern "C"' functions (implied by /permissive-)
  lambda[-]             better lambda support by using the newer lambda processor (off by default)
  tlsGuards[-]          generate runtime checks for TLS variable initialization (on by default)
  zeroSizeArrayNew[-]   call member new/delete for 0-size arrays of objects (on by default)
  static_assert[-]      strict handling of 'static_assert' (implied by /permissive-)
  __STDC__              define __STDC__ to 1 in C
/await enable resumable functions extension
/await:strict enable standard C++20 coroutine support with earlier language versions
/constexpr:depth<N>     recursion depth limit for constexpr evaluation (default: 512)
/constexpr:backtrace<N> show N constexpr evaluations in diagnostics (default: 10)
/constexpr:steps<N>     terminate constexpr evaluation after N steps (default: 100000)
/Zi enable debugging information        /Z7 enable old-style debug info
/Zo[-] generate richer debugging information for optimized code (on by default)
/ZH:[MD5|SHA1|SHA_256] hash algorithm for calculation of file checksum in debug info (default: SHA_256)
/Zp[n] pack structs on n-byte boundary  /Zl omit default library name in .OBJ
/vd{0|1|2} disable/enable vtordisp      /vm<x> type of pointers to members
/std:<c11|c17> C standard version
    c11 - ISO/IEC 9899:2011
    c17 - ISO/IEC 9899:2018
/ZI enable Edit and Continue debug info
/openmp enable OpenMP 2.0 language extensions
/openmp:experimental enable OpenMP 2.0 language extensions plus select OpenMP 3.0+ language extensions
/openmp:llvm OpenMP language extensions using LLVM runtime

                              -MISCELLANEOUS-

@<file> options response file           /?, /help print this help message
/bigobj generate extended object format /c compile only, no link
/errorReport:option deprecated. Report internal compiler errors to Microsoft
    none - do not send report
    prompt - prompt to immediately send report
    queue - at next admin logon, prompt to send report (default)
    send - send report automatically
/FC use full pathnames in diagnostics   /H<num> max external name length
/J default char type is unsigned
/MP[n] use up to 'n' processes for compilation
/nologo suppress copyright message      /showIncludes show include file names
/Tc<source file> compile file as .c     /Tp<source file> compile file as .cpp
/TC compile all files as .c             /TP compile all files as .cpp
/V<string> set version string           /Yc[file] create .PCH file
/Yd put debug info in every .OBJ        /Yl[sym] inject .PCH ref for debug lib
/Yu[file] use .PCH file                 /Y- disable all PCH options
/Zm<n> max memory alloc (% of default)  /FS force to use MSPDBSRV.EXE
/source-charset:<iana-name>|.nnnn set source character set
/execution-charset:<iana-name>|.nnnn set execution character set
/utf-8 set source and execution character set to UTF-8
/validate-charset[-] validate UTF-8 files for only legal characters
/fastfail[-] enable fast-fail mode      /JMC[-] enable native just my code
/presetPadding[-] zero initialize padding for stack based class types
/volatileMetadata[-] generate metadata on volatile memory accesses
/sourcelink [file] file containing source link information

                                -LINKING-

/LD Create .DLL                         /LDd Create .DLL debug library
/LN Create a .netmodule                 /F<num> set stack size
/link [linker options and libraries]    /MD link with MSVCRT.LIB
/MT link with LIBCMT.LIB                /MDd link with MSVCRTD.LIB debug lib
/MTd link with LIBCMTD.LIB debug lib

                              -CODE ANALYSIS-

/analyze[-] Enable native analysis      /analyze:quiet[-] No warning to console
/analyze:log<name> Warnings to file     /analyze:autolog Log to *.pftlog
/analyze:autolog:ext<ext> Log to *.<ext>/analyze:autolog- No log file
/analyze:WX- Warnings not fatal         /analyze:stacksize<num> Max stack frame
/analyze:max_paths<num> Max paths       /analyze:only Analyze, no code gen

                              -DIAGNOSTICS-

/diagnostics:<args,...> controls the format of diagnostic messages:
             classic   - retains prior format
             column[-] - prints column information
             caret[-]  - prints column and the indicated line of source
/Wall enable all warnings               /w   disable all warnings
/W<n> set warning level (default n=1)
/Wv:xx[.yy[.zzzzz]] disable warnings introduced after version xx.yy.zzzzz
/WX treat warnings as errors            /WL enable one line diagnostics
/wd<n> disable warning n                /we<n> treat warning n as an error
/wo<n> issue warning n once             /w<l><n> set warning level 1-4 for n
/external:I <path>      - location of external headers
/external:env:<var>     - environment variable with locations of external headers
/external:anglebrackets - treat all headers included via <> as external
/external:W<n>          - warning level for external headers
/external:templates[-]  - evaluate warning level across template instantiation chain
/sdl enable additional security features and warnings
/options:strict unrecognized compiler options are an error


                                             LINKER


Incremental Linker Version 14.32.31329.0

 usage: LINK [options] [files] [@commandfile]

   options:

      /ALIGN:#
      /ALLOWBIND[:NO]
      /ALLOWISOLATION[:NO]
      /APPCONTAINER[:NO]
      /ASSEMBLYDEBUG[:DISABLE]
      /ASSEMBLYLINKRESOURCE:filename
      /ASSEMBLYMODULE:filename
      /ASSEMBLYRESOURCE:filename[,[name][,PRIVATE]]
      /BASE:{address[,size]|@filename,key}
      /CLRIMAGETYPE:{IJW|PURE|SAFE|SAFE32BITPREFERRED}
      /CLRLOADEROPTIMIZATION:{MD|MDH|NONE|SD}
      /CLRSUPPORTLASTERROR[:{NO|SYSTEMDLL}]
      /CLRTHREADATTRIBUTE:{MTA|NONE|STA}
      /CLRUNMANAGEDCODECHECK[:NO]
      /DEBUG[:{FASTLINK|FULL|NONE}]
      /DEF:filename
      /DEFAULTLIB:library
      /DELAY:{NOBIND|UNLOAD}
      /DELAYLOAD:dll
      /DELAYSIGN[:NO]
      /DEPENDENTLOADFLAG:flag
      /DLL
      /DRIVER[:{UPONLY|WDM}]
      /DYNAMICBASE[:NO]
      /EMITVOLATILEMETADATA[:NO]
(press <return> to continue)
      /ENTRY:symbol
      /ERRORREPORT:{NONE|PROMPT|QUEUE|SEND}
      /EXPORT:symbol
      /EXPORTPADMIN[:size]
      /FASTFAIL[:NO]
      /FASTGENPROFILE[:{COUNTER32|COUNTER64|EXACT|MEMMAX=#|MEMMIN=#|NOEXACT|
                        NOPATH|NOTRACKEH|PATH|PGD=filename|TRACKEH}]
      /FILEALIGN:#
      /FIXED[:NO]
      /FORCE[:{MULTIPLE|UNRESOLVED}]
      /FUNCTIONPADMIN[:size]
      /GUARD:{CF|NO}
      /GENPROFILE[:{COUNTER32|COUNTER64|EXACT|MEMMAX=#|MEMMIN=#|NOEXACT|
                    NOPATH|NOTRACKEH|PATH|PGD=filename|TRACKEH}]
      /HEAP:reserve[,commit]
      /HIGHENTROPYVA[:NO]
      /IDLOUT:filename
      /IGNORE:#
      /IGNOREIDL
      /ILK:filename
      /IMPLIB:filename
      /INCLUDE:symbol
      /INCREMENTAL[:NO]
      /INTEGRITYCHECK
      /KERNEL
      /KEYCONTAINER:name
      /KEYFILE:filename
      /LARGEADDRESSAWARE[:NO]
      /LIBPATH:dir
(press <return> to continue)
      /LINKREPRO:dir
      /LINKREPROTARGET:filename
      /LTCG[:{INCREMENTAL|NOSTATUS|OFF|STATUS|}]
      /LTCGOUT:filename
      /MACHINE:{ARM|ARM64|ARM64EC|ARM64X|EBC|X64|X86}
      /MANIFEST[:{EMBED[,ID=#]|NO}]
      /MANIFESTDEPENDENCY:manifest dependency
      /MANIFESTFILE:filename
      /MANIFESTINPUT:filename
      /MANIFESTUAC[:{NO|UAC fragment}]
      /MAP[:filename]
      /MAPINFO:{EXPORTS|PDATA}
      /MERGE:from=to
      /MIDL:@commandfile
      /NATVIS:filename
      /NOASSEMBLY
      /NODEFAULTLIB[:library]
      /NOENTRY
      /NOEXP
      /NOIMPLIB
      /NOLOGO
      /NXCOMPAT[:NO]
      /OPT:{ICF[=iterations]|LBR|NOICF|NOLBR|NOREF|REF}
      /ORDER:@filename
      /OUT:filename
      /PDB:filename
      /PDBSTRIPPED[:filename]
      /PROFILE
      /RELEASE
(press <return> to continue)
      /SAFESEH[:NO]
      /SECTION:name,[[!]{DEKPRSW}][,ALIGN=#]
      /SOURCELINK:filename
      /STACK:reserve[,commit]
      /STUB:filename
      /SUBSYSTEM:{BOOT_APPLICATION|CONSOLE|EFI_APPLICATION|
                  EFI_BOOT_SERVICE_DRIVER|EFI_ROM|EFI_RUNTIME_DRIVER|
                  NATIVE|POSIX|WINDOWS|WINDOWSCE}[,#[.##]]
      /SWAPRUN:{CD|NET}
      /TLBID:#
      /TLBOUT:filename
      /TIME
      /TSAWARE[:NO]
      /USEPROFILE[:{AGGRESSIVE|PGD=filename}]
      /VERBOSE[:{CLR|ICF|INCR|LIB|REF|SAFESEH|UNUSEDDELAYLOAD|UNUSEDLIBS}]
      /VERSION:#[.#]
      /WINMD[:{NO|ONLY}]
      /WINMDDELAYSIGN[:NO]
      /WINMDFILE:filename
      /WINMDKEYCONTAINER:name
      /WINMDKEYFILE:filename
      /WHOLEARCHIVE[:library]
      /WX[:NO]


C runtime .lib files

The C runtime Library (CRT) is the part of the C++ Standard Library that incorporates the ISO C standard library. The Visual C++ libraries that implement the CRT support native code development, and both mixed native and managed code. All versions of the CRT support multi-threaded development. Most of the libraries support both static linking, to link the library directly into your code, or dynamic linking to let your code use common DLL files.

Starting in Visual Studio 2015, the CRT has been refactored into new binaries. The Universal CRT (UCRT) contains the functions and globals exported by the standard C99 CRT library. The UCRT is now a Windows component, and ships as part of Windows 10 and later versions. The static library, DLL import library, and header files for the UCRT are now found in the Windows SDK. When you install Visual C++, Visual Studio setup installs the subset of the Windows SDK required to use the UCRT. You can use the UCRT on any version of Windows supported by Visual Studio 2015 and later versions. You can redistribute it using vcredist for supported versions of Windows other than Windows 10 or later. For more information, see Redistributing Visual C++ Files.

The following table lists the libraries that implement the UCRT.
┌───────────────┬─────────────────┬────────────────────────────────────────────────────────────┬────────┐
│   Library     │ Associated DLL  │                      Characteristics                       │ Option │
├───────────────┼─────────────────┼────────────────────────────────────────────────────────────┼────────┤
│ libucrt.lib   │ None            │ Statically links the UCRT into your code.                  │ /MT    │
│ libucrtd.lib  │ None            │ Debug version of the UCRT for static linking. ND.          │ /MTd   │
│ ucrt.lib      │ ucrtbase.dll    │ DLL import library for the UCRT.                           │ /MD    │
│ ucrtd.lib     │ ucrtbased.dll   │ DLL import library for the Debug version of the UCRT. ND.  │ /MDd   │
└───────────────┴─────────────────┴────────────────────────────────────────────────────────────┴────────┘
┌───────────────┬─────────────────────────┐
│   Library     │ Preprocessor directives │
├───────────────┼─────────────────────────┤
│ libucrt.lib   │ _MT                     │
│ libucrtd.lib  │ _DEBUG, _MT             │
│ ucrt.lib      │ _MT, _DLL               │
│ ucrtd.lib     │ _DEBUG, _MT, _DLL       │
└───────────────┴─────────────────────────┘

The vcruntime library contains Visual C++ CRT implementation-specific code, such as exception handling and debugging support, runtime checks and type information, implementation details and certain extended library functions. The vcruntime library version needs to match the version of the compiler you're using.

This table lists the libraries that implement the vcruntime library.
┌────────────────────┬──────────────────────────┬──────────────────────────────────────────────────┬────────┐
│      Library       │     Associated DLL       │                 Characteristics                  │ Option │
├────────────────────┼──────────────────────────┼──────────────────────────────────────────────────┼────────┤
│ libvcruntime.lib   │ None                     │ Statically linked into your code.                │ /MT    │
│ libvcruntimed.lib  │ None                     │ Debug version for static linking. NR.            │ /MTd   │
│ vcruntime.lib      │ vcruntime<version>.dll   │ DLL import library for the vcruntime.            │ /MD    │
│ vcruntimed.lib     │ vcruntime<version>d.dll  │ DLL import library for the Debug vcruntime. NR.  │ /MDd   │
└────────────────────┴──────────────────────────┴──────────────────────────────────────────────────┴────────┘
┌────────────────────┬─────────────────────────┐
│      Library       │ Preprocessor directives │
├────────────────────┼─────────────────────────┤
│ libvcruntime.lib   │ _MT                     │
│ libvcruntimed.lib  │ _MT, _DEBUG             │
│ vcruntime.lib      │ _MT, _DLL               │
│ vcruntimed.lib     │ _DEBUG, _MT, _DLL       │
└────────────────────┴─────────────────────────┘

Note

When the UCRT was refactored, the Concurrency Runtime functions were moved into concrt140.dll, which was added to the C++ redistributable package. This DLL is required for C++ parallel containers and algorithms such as concurrency::parallel_for. In addition, the C++ Standard Library requires this DLL on Windows XP to support synchronization primitives, because Windows XP doesn't have condition variables.

The code that initializes the CRT is in one of several libraries, based on whether the CRT library is statically or dynamically linked, or native, managed, or mixed code. This code handles CRT startup, internal per-thread data initialization, and termination. It's specific to the version of the compiler used. This library is always statically linked, even when using a dynamically linked UCRT.

This table lists the libraries that implement CRT initialization and termination.
┌────────────────┬───────────┬─────────────────────────┐
│    Library     │  Option   │ Preprocessor directives │
├────────────────┼───────────┼─────────────────────────┤
│ libcmt.lib     │ /MT       │ _MT                     │
│ libcmtd.lib    │ /MTd      │ _DEBUG, _MT             │
│ msvcrt.lib     │ /MD       │ _MT, _DLL               │
│ msvcrtd.lib    │ /MDd      │ _DEBUG, _MT, _DLL       │
│ msvcmrt.lib    │ /clr      │                         │
│ msvcmrtd.lib   │ /clr      │                         │
│ msvcurt.lib    │ /clr:pure │                         │
│ msvcurtd.lib   │ /clr:pure │                         │
└────────────────┴───────────┴─────────────────────────┘
│ libcmt.lib    │ Statically links the native CRT startup into your code.
│ libcmtd.lib   │ Statically links the Debug version of the native CRT startup. Not redistributable.
│ msvcrt.lib    │ Static library for the native CRT startup for use with DLL UCRT and vcruntime.
│ msvcrtd.lib   │ Static library for the Debug version of the native CRT startup for use with DLL UCRT and vcruntime
│ msvcmrt.lib   │ Static library for the mixed native and managed CRT startup for use with DLL UCRT and vcruntime.
│ msvcmrtd.lib  │ Static library for the Debug version of the mixed native and managed CRT startup for use with DLL
│ UCRT and vcruntime. Not redistributable.
│ msvcurt.lib   │ Deprecated Static library for the pure managed CRT.
│ msvcurtd.lib  │ Deprecated Static library for the Debug version of the pure managed CRT. Not redistributable.

If you link your program from the command line without a compiler option that specifies a C runtime library, the linker will use the statically linked CRT libraries: libcmt.lib, libvcruntime.lib, and libucrt.lib.

Using the statically linked CRT implies that any state information saved by the C runtime library will be local to that instance of the CRT. For example, if you use strtok when using a statically linked CRT, the position of the strtok parser is unrelated to the strtok state used in code in the same process (but in a different DLL or EXE) that is linked to another instance of the static CRT. In contrast, the dynamically linked CRT shares state for all code within a process that is dynamically linked to the CRT. This concern doesn't apply if you use the new more secure versions of these functions; for example, strtok_s doesn't have this problem.

Because a DLL built by linking to a static CRT has its own CRT state, it isn't recommended to link statically to the CRT in a DLL unless the consequences of this are specifically desired and understood. For example, if you call _set_se_translator in an executable that loads the DLL linked to its own static CRT, any hardware exceptions generated by the code in the DLL will not be caught by the translator, but hardware exceptions generated by code in the main executable will be caught.

If you're using the /clr compiler switch, your code will be linked with a static library, msvcmrt.lib. The static library provides a proxy between your managed code and the native CRT. You cannot use the statically linked CRT ( /MT or /MTd options) with /clr. Use the dynamically-linked libraries (/MD or /MDd) instead. The pure managed CRT libraries are deprecated in Visual Studio 2015 and unsupported in Visual Studio 2017.

For more information on using the CRT with /clr, see Mixed (Native and Managed) Assemblies.

To build a debug version of your application, the _DEBUG flag must be defined and the application must be linked with a debug version of one of these libraries. For more information about using the debug versions of the library files, see CRT Debugging Techniques.

This version of the CRT isn't fully conformant with the C99 standard. In versions before Visual Studio 2019 version 16.8, the <tgmath.h> header isn't supported. In all versions, the CX_LIMITED_RANGE and FP_CONTRACT pragma macros aren't supported. Certain elements such as the meaning of parameter specifiers in standard IO functions use legacy interpretations by default. You can use /Zc compiler conformance options and specify linker options to control some aspects of library conformance.
C++ Standard Library .lib files
┌───────────────────────┬──────────────────────────────┬─────────┬─────────────────────────┐
│ C++ Standard Library  │       Characteristics        │ Option  │ Preprocessor directives │
├───────────────────────┼──────────────────────────────┼─────────┼─────────────────────────┤
│ libcpmt.lib           │ Multithreaded, static link   │ /MT     │ _MT                     │
│ msvcprt.lib           │ Multithreaded, dynamic link  │ /MD     │ _MT, _DLL               │
│ libcpmtd.lib          │ Multithreaded, static link   │ /MTd    │ _DEBUG, _MT             │
│ msvcprtd.lib          │ Multithreaded, dynamic link  │ /MDd    │ _DEBUG, _MT, _DLL       │
└───────────────────────┴──────────────────────────────┴─────────┴─────────────────────────┘

/MD ----> (import library for msvcp<version>.dll)
/MDd ---> (import library for msvcp<version>d.dll)

When you build a release version of your project, one of the basic C runtime libraries (libcmt.lib, msvcmrt.lib, msvcrt.lib) is linked by default, depending on the compiler option you choose (multithreaded, DLL, /clr). If you include one of the C++ Standard Library header files in your code, a C++ Standard Library will be linked automatically by Visual C++ at compile time. For example:
C++

#include <ios>

For binary compatibility, more than one DLL file may be specified by a single import library. Version updates may introduce dot libraries, separate DLLs that introduce new library functionality. For example, Visual Studio 2017 version 15.6 introduced msvcp140_1.dll to support additional standard library functionality without breaking the Application Binary Interface (ABI) supported by msvcp140.dll. The msvcprt.lib import library included in the toolset for Visual Studio 2017 version 15.6 supports both DLLs, and the vcredist for this version installs both DLLs. Once shipped, a dot library has a fixed ABI, and will never have a dependency on a later dot library.