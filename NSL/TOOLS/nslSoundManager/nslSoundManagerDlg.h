// nslSoundManagerDlg.h : header file
//
//{{AFX_INCLUDES()
#include "treeview.h"
#include "SoundFileManager.h"
#include "slider.h"
#include "richtext.h"
//}}AFX_INCLUDES

#if !defined(AFX_NSLSOUNDMANAGERDLG_H__DEB41041_457E_439D_ABF8_E8BA0AAB175B__INCLUDED_)
#define AFX_NSLSOUNDMANAGERDLG_H__DEB41041_457E_439D_ABF8_E8BA0AAB175B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CNslSoundManagerDlg dialog

class CNslSoundManagerDlg : public CDialog
{
// Construction
public:
	CString m_output_spu_dir;
	CString m_output_level_snd_dir;
	CString m_nslSoundToolPath;
	int m_sortInfo;
	void SetLoop(bool loop);
	CString m_root_dir;
	void SetLocation(CString location);
	void SetType(CString type);
	SoundFileManager m_soundfilemanager;
	HTREEITEM m_tree_levels;
	CNslSoundManagerDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	//{{AFX_DATA(CNslSoundManagerDlg)
	enum { IDD = IDD_NSLSOUNDMANAGER_DIALOG };
	CTreeCtrl	m_tree1;
	CListCtrl	m_list1;
//	CRichText	m_volume;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNslSoundManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNslSoundManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadioSfx();
	afx_msg void OnButtonSfx();
	afx_msg void OnButtonAmbient();
	afx_msg void OnButtonMusic();
	afx_msg void OnButtonVoice();
	afx_msg void OnButtonSpu();
	afx_msg void OnButtonCd();
	afx_msg void OnButtonEnable();
	afx_msg void OnButtonDisable();
	afx_msg void OnButtonDefault();
	afx_msg void OnButtonLoop();
	afx_msg void OnButtonNoloop();
	afx_msg void OnEndlabeleditList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonAddsound();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonSave();
	afx_msg void OnButtonAddlevel();
	afx_msg void OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonProcess();
	afx_msg void OnButtonProcessLevel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NSLSOUNDMANAGERDLG_H__DEB41041_457E_439D_ABF8_E8BA0AAB175B__INCLUDED_)
