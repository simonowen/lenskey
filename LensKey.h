// LensKey - a Lenslok Emulator
//
//	(c) 2002-2005	Simon Owen <simon.owen@simcoupe.org>

#ifndef LENSKEY_H
#define LENSKEY_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#pragma comment(linker,"/merge:.text=.data")
#pragma comment(linker,"/merge:.reloc=.data")

#if _MSC_VER >= 1000
#pragma comment(linker,"/filealign:0x200")
#endif

#endif
