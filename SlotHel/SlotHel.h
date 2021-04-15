// SlotHel.h : main header file for the SlotHel DLL
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSlotHelApp
// See SlotHel.cpp for the implementation of this class
//

class CSlotHelApp : public CWinApp
{
public:
	CSlotHelApp();

	// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
