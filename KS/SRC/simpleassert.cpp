#include "global.h"

//****************************************************************************
//**
//**    cAssert.cpp
//**    Source - Replacement assert functions.
//**
//****************************************************************************
#ifndef _XBOX
#include <windows.h>
#endif /* _XBOX */

#include <stdio.h>
#include "SimpleAssert.h"

#include "resource.h"
//=============================================================
#ifdef _DEBUG

extern HINSTANCE g_hInst;

static const char* _s_expStr  = NULL;
static const char* _s_msg     = NULL;
static const char* _s_file    = NULL;
static char _s_lineNum[12];

enum
{
  AD_DEBUG        = 0x01,
  AD_IGNORE       = 0x02,
  AD_IGNOREALWAYS = 0x03,
  AD_EXIT         = 0x04
};

//=============================================================
// AssertDlgProc
#ifdef _XBOX
INT_PTR CALLBACK AssertDlgProc(HWND hDlg, UINT msg,
                               WPARAM wParam, LPARAM lParam)
{
  STUB( "AssertDlgProc" );

  return TRUE;
}


//=============================================================
// _sAssertDlg_
bool _sAssertDlg_(const char* expStr, const char* msg,
                  const char* file, int line, bool* ignoreAlways)
{
  STUB( "_sAssertDlg_" );

  return true;
}

#else
INT_PTR CALLBACK AssertDlgProc(HWND hDlg, UINT msg,
                               WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
  case WM_INITDIALOG:
    {
      ::SetWindowText(::GetDlgItem(hDlg, IDC_FILENAME),
                      _s_file);

      ::SetWindowText(::GetDlgItem(hDlg, IDC_LINENUMBER),
                      _s_lineNum);

      ::SetWindowText(::GetDlgItem(hDlg, IDC_EXPRESSION),
                      _s_expStr);

      ::SetWindowText(::GetDlgItem(hDlg, IDC_MESSAGE),
                      _s_msg);

      char modulePath[MAX_PATH];
      GetModuleFileName(NULL, modulePath, MAX_PATH);

      const char* moduleName = strrchr(modulePath, '\\');
      moduleName = moduleName ? moduleName+1 : modulePath;

      char title[MAX_PATH + 20];
      sprintf(title, "Assert Failed: %s", moduleName);
      SetWindowText(hDlg, title);

      // Paste it to clipboard:
      if (OpenClipboard(NULL))
      {
        HGLOBAL hMem;
        char buf[1024];
        char *pMem;

        sprintf(buf, "Application: %s\r\nFilename: %s (%s)\r\nExpression: %s\r\nMessage: %s\r\n",
                moduleName, _s_file, _s_lineNum, _s_expStr,
                _s_msg ? _s_msg : "");

        hMem = GlobalAlloc(GHND|GMEM_DDESHARE, strlen(buf)+1);
        if (hMem)
        {
          pMem = (char*)GlobalLock(hMem);
          strcpy(pMem, buf);
          GlobalUnlock(hMem);

          EmptyClipboard();
          SetClipboardData(CF_TEXT, hMem);
        }

        CloseClipboard();
        GlobalFree(hMem);
      }

      break;
    }
  case WM_COMMAND:
    {
      switch(LOWORD(wParam))
      {
      case IDDEBUG:
        EndDialog(hDlg, AD_DEBUG);
        return TRUE;

      case IDIGNORE:
        EndDialog(hDlg, AD_IGNORE);
        return TRUE;

      case IDIGNOREALWAYS:
        EndDialog(hDlg, AD_IGNOREALWAYS);
        return TRUE;

      case IDEXIT:
        EndDialog(hDlg, AD_EXIT);
        return TRUE;

      }
    }
    break;
  }

  return FALSE;
}

//=============================================================
// _sAssertDlg_
bool _sAssertDlg_(const char* expStr, const char* msg,
                  const char* file, int line, bool* ignoreAlways)
{
  DWORD lastErr = GetLastError();

  // EnterCriticalSection(...)

  _s_expStr = expStr;
  _s_msg = msg;

  // log it
  char logMsg[1024];
  sprintf(logMsg, "%s(%i) : [Assert] (%s) %s\n", file, line, expStr,
          msg ? msg : "");
  OutputDebugString(logMsg);

  // calc last "\" pos
  _s_file = strrchr(file, '\\');
  _s_file = _s_file ? _s_file+1 : file;

  // convert line to a string
  itoa(line, _s_lineNum, 10);

  // show dialog
  INT_PTR res;
  res = DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SIMPLE_ASSERT), NULL,
                  (DLGPROC) AssertDlgProc);

  switch (res)
  {
  case AD_IGNOREALWAYS:
    *ignoreAlways = true;

  case AD_IGNORE:
    return false;

  case AD_DEBUG:
    return true;

  case AD_EXIT:
    exit(0);
    break;
  }

  // LeaveCriticalSection(...)

  SetLastError(lastErr);
  return true;
}
#endif /* _XBOX JIV DEBUG */

#endif // _DEBUG

