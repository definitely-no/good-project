/**
 * Windows 11 Win32 Subsystem Kernel Driver (win32k.sys) - Header
 * Target: extracted_usermode/win32k.sys (SHA-256 verified)
 */

#ifndef _WIN32K_H_
#define _WIN32K_H_

#include "ntoskrnl.h"

typedef struct _RECTL {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECTL, *PRECTL;

typedef struct _CLIPOBJ {
    ULONG iUniq;
    RECTL rclBounds;
    uint8_t iDComplexity;
    uint8_t iFComplexity;
    uint8_t iMode;
    uint8_t fjOptions;
} CLIPOBJ, *PCLIPOBJ;

typedef struct _ENUMRECTS {
    ULONG c;
    RECTL arcl[1];
} ENUMRECTS, *PENUMRECTS;

typedef struct _DXGK_WIN32K_INTERFACE {
    ULONG Version;
    PVOID DxgkProcessCallout;
    PVOID DxgkThreadCallout;
} DXGK_WIN32K_INTERFACE, *PDXGK_WIN32K_INTERFACE;

/* Exported Functions from win32k.sys */
PVOID DxDdGetDxgkWin32kInterface(ULONG Version);
NTSTATUS DxDdProcessCallout(PEPROCESS Process, BOOLEAN Initialize);
NTSTATUS DxDdThreadCallout(PETHREAD Thread);
HANDLE CddEngCreateRectRgn(LONG Left, LONG Top, LONG Right, LONG Bottom);
LONG CddEngCombineRgn(HANDLE DestRgn, HANDLE SrcRgn1, HANDLE SrcRgn2, LONG CombineMode);
ULONG CLIPOBJ_cEnumStart(PCLIPOBJ ClipObj, BOOLEAN Dci, ULONG ObjType, ULONG Direction, ULONG MaxRects);
BOOLEAN CLIPOBJ_bEnum(PCLIPOBJ ClipObj, ULONG BufferSize, PVOID Buffer);

#endif /* _WIN32K_H_ */
