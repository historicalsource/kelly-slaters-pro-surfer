// nslSoundManager.h : main header file for the NSLSOUNDMANAGER application
//

#if !defined(AFX_NSLSOUNDMANAGER_H__7CE7F01C_0413_4F09_A569_A81E30D4039F__INCLUDED_)
#define AFX_NSLSOUNDMANAGER_H__7CE7F01C_0413_4F09_A569_A81E30D4039F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols



/////////////////////////////////////////////////////////////////////////////
// CNslSoundManagerApp:
// See nslSoundManager.cpp for the implementation of this class
//

class CNslSoundManagerApp : public CWinApp
{
public:
	CNslSoundManagerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNslSoundManagerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNslSoundManagerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NSLSOUNDMANAGER_H__7CE7F01C_0413_4F09_A569_A81E30D4039F__INCLUDED_)
