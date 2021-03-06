#if !defined(AFX_NODE_H__46C89526_B8A3_4D21_AF6E_5593482FDAA0__INCLUDED_)
#define AFX_NODE_H__46C89526_B8A3_4D21_AF6E_5593482FDAA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CPicture;

/////////////////////////////////////////////////////////////////////////////
// CNode wrapper class

class CNode : public COleDispatchDriver
{
public:
	CNode() {}		// Calls COleDispatchDriver default constructor
	CNode(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CNode(const CNode& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	CNode GetChild();
	void SetRefChild(LPDISPATCH newValue);
	short GetChildren();
	void SetChildren(short nNewValue);
	BOOL GetExpanded();
	void SetExpanded(BOOL bNewValue);
	VARIANT GetExpandedImage();
	void SetExpandedImage(const VARIANT& newValue);
	CNode GetFirstSibling();
	void SetRefFirstSibling(LPDISPATCH newValue);
	CString GetFullPath();
	void SetFullPath(LPCTSTR lpszNewValue);
	VARIANT GetImage();
	void SetImage(const VARIANT& newValue);
	short GetIndex();
	void SetIndex(short nNewValue);
	CString GetKey();
	void SetKey(LPCTSTR lpszNewValue);
	CNode GetLastSibling();
	void SetRefLastSibling(LPDISPATCH newValue);
	CNode GetNext();
	void SetRefNext(LPDISPATCH newValue);
	CNode GetParent();
	void SetRefParent(LPDISPATCH newValue);
	CNode GetPrevious();
	void SetRefPrevious(LPDISPATCH newValue);
	CNode GetRoot();
	void SetRefRoot(LPDISPATCH newValue);
	BOOL GetSelected();
	void SetSelected(BOOL bNewValue);
	VARIANT GetSelectedImage();
	void SetSelectedImage(const VARIANT& newValue);
	BOOL GetSorted();
	void SetSorted(BOOL bNewValue);
	VARIANT GetTag();
	void SetTag(const VARIANT& newValue);
	CString GetText();
	void SetText(LPCTSTR lpszNewValue);
	BOOL GetVisible();
	void SetVisible(BOOL bNewValue);
	CPicture CreateDragImage();
	BOOL EnsureVisible();
	unsigned long GetBackColor();
	void SetBackColor(unsigned long newValue);
	BOOL GetBold();
	void SetBold(BOOL bNewValue);
	BOOL GetChecked();
	void SetChecked(BOOL bNewValue);
	unsigned long GetForeColor();
	void SetForeColor(unsigned long newValue);
	void SetRefTag(const VARIANT& newValue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODE_H__46C89526_B8A3_4D21_AF6E_5593482FDAA0__INCLUDED_)
