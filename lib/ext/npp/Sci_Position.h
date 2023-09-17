// Scintilla source code edit control
/** @file Sci_Position.h
 ** Define the Sci_Position type used in Scintilla's external interfaces.
 ** These need to be available to clients written in C so are not in a C++ namespace.
 **/
// Copyright 2015 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCI_POSITION_H
#define SCI_POSITION_H

#include <stddef.h>

// Basic signed type used throughout interface
typedef ptrdiff_t Sci_Position;

// Unsigned variant used for ILexer::Lex and ILexer::Fold
typedef size_t Sci_PositionU;

// Deprecated by Notepad++ 2GB+ support via new scintilla interfaces from 5.2.3 (see https://www.scintilla.org/ScintillaHistory.html) 
// Please use Sci_Position, SCI_GETTEXTRANGEFULL, SCI_FINDTEXTFULL, and SCI_FORMATRANGEFULL and corresponding defines/structs
// typedef long Sci_PositionCR;

#ifdef _WIN32
	#define SCI_METHOD __stdcall
#else
	#define SCI_METHOD
#endif

#endif
