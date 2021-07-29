// nslSoundManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nslSoundManager.h"
#include "nslSoundManagerDlg.h"
#include "mstask.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CString SOURCE_SOUNDS_PATH("dev\\data\\sounds");
CString DEST_SOUNDS_PATH("data");
CString PS2SOUND("PS2SOUND");
CString LEVELS(".\\");



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNslSoundManagerDlg dialog

CNslSoundManagerDlg::CNslSoundManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNslSoundManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNslSoundManagerDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNslSoundManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNslSoundManagerDlg)
	DDX_Control(pDX, IDC_TREE1, m_tree1);
	DDX_Control(pDX, IDC_LIST1, m_list1);
//	DDX_Control(pDX, IDC_RICHTEXTCTRL_VOLUME, m_volume);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNslSoundManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CNslSoundManagerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_BUTTON_SFX, OnButtonSfx)
	ON_BN_CLICKED(IDC_BUTTON_AMBIENT, OnButtonAmbient)
	ON_BN_CLICKED(IDC_BUTTON_MUSIC, OnButtonMusic)
	ON_BN_CLICKED(IDC_BUTTON_VOICE, OnButtonVoice)
	ON_BN_CLICKED(IDC_BUTTON_SPU, OnButtonSpu)
	ON_BN_CLICKED(IDC_BUTTON_CD, OnButtonCd)
	ON_BN_CLICKED(IDC_BUTTON_ENABLE, OnButtonEnable)
	ON_BN_CLICKED(IDC_BUTTON_DISABLE, OnButtonDisable)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnButtonDefault)
	ON_BN_CLICKED(IDC_BUTTON_LOOP, OnButtonLoop)
	ON_BN_CLICKED(IDC_BUTTON_NOLOOP, OnButtonNoloop)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnEndlabeleditList1)
	ON_BN_CLICKED(IDC_BUTTON_ADDSOUND, OnButtonAddsound)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_ADDLEVEL, OnButtonAddlevel)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE1, OnEndlabeleditTree1)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, OnColumnclickList1)
	ON_BN_CLICKED(IDC_BUTTON_PROCESS, OnButtonProcess)
	ON_BN_CLICKED(IDC_BUTTON_PROCESS_LEVEL, OnButtonProcessLevel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNslSoundManagerDlg message handlers

BOOL CNslSoundManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_root_dir = "C:\\MR\\";
	m_nslSoundToolPath = "C:\\MR\\dev\\data\\sounds\\nslSoundTool.exe";

	CStdioFile inifile("nslSoundManager.ini", CFile::modeRead | CFile::typeText );
	CString line;
	while(inifile.ReadString(line)) {
		line += " ";
		line.TrimLeft();
		if(line=="" || line.GetAt(0)==';' || line.GetAt(0)=='#') continue;
		CString param = line.Left(line.FindOneOf(" \t"));
		CString option = line.Mid(line.FindOneOf(" \t")+1);
		param.TrimLeft();
		param.TrimRight();
		option.TrimLeft();
		option.TrimRight();
		if(param.CompareNoCase("ROOT_DIR")==0) {
			m_root_dir = option;
		} else if(param.CompareNoCase("SOURCE_SOUNDS_PATH")==0) {
			SOURCE_SOUNDS_PATH = option;
		} else if(param.CompareNoCase("LEVELS")==0) {
			LEVELS = option;
		} else if(param.CompareNoCase("DEST_SOUNDS_PATH")==0) {
			DEST_SOUNDS_PATH = option;
		} else if(param.CompareNoCase("PS2SOUND")==0) {
			PS2SOUND = option;
		} else if(param.CompareNoCase("NSLSOUNDTOOL_PATH")==0) {
			m_nslSoundToolPath = option;
		} else if(param.CompareNoCase("output_spu_dir")==0) {
			m_output_spu_dir = option;
		} else if(param.CompareNoCase("output_level_snd_dir")==0) {
			m_output_level_snd_dir = option;
		}
	}
	//[-I input_dir] [-S SND_file_dir] [-O output_dir] [-L output_level_snd_dir] [-P output_spu_dir] <SND files>

	inifile.Close();

	HTREEITEM common = m_tree1.InsertItem("COMMON");
	m_tree_levels = m_tree1.InsertItem("LEVELS");

	WIN32_FIND_DATA FindFileData;  // data buffer
	HANDLE hfind = FindFirstFile(m_root_dir + SOURCE_SOUNDS_PATH + LEVELS +"\\*.SND", &FindFileData);

	if(hfind == INVALID_HANDLE_VALUE) {
		MessageBox("Couldn't find any .SND files in "+m_root_dir + SOURCE_SOUNDS_PATH + LEVELS, "Error", MB_ICONSTOP);
	} else {
		int morefiles = 1;
		while (morefiles) {
			CString fname = FindFileData.cFileName;
			fname = fname.Left(fname.ReverseFind('.')); // remove extension

			m_soundfilemanager.GetLevel("COMMON") = m_soundfilemanager.AddLevel(fname).ReadSndFile(m_root_dir + SOURCE_SOUNDS_PATH + LEVELS);
/*
			if( fname.CompareNoCase("COMMON")==0 ) {
				m_soundfilemanager.GetLevel("COMMON").ReadSndFile(m_root_dir + SOURCE_SOUNDS_PATH + LEVELS);
			}
			else {
				m_soundfilemanager.AddLevel(fname).ReadSndFile(m_root_dir + SOURCE_SOUNDS_PATH + LEVELS);
			}
*/
			morefiles = FindNextFile(hfind, &FindFileData);
		}
	}

	FindClose(hfind);

	int i;

	// set the tree to have the levels
	for(i=0;i<m_soundfilemanager.GetNumLevels();i++) {
		SoundFile& level = m_soundfilemanager.GetLevel(i);
		m_tree1.InsertItem(level.GetName(), m_tree_levels);
	}

	m_tree1.Expand(m_tree_levels, TVE_EXPAND);

	m_list1.InsertColumn(0, "Name", LVCFMT_LEFT, 220);
	m_list1.InsertColumn(1, "",LVCFMT_LEFT, 20);	// enabled, disabled
	m_list1.InsertColumn(2, "Type", LVCFMT_LEFT, 50); // sfx, ambient, music and voice
	m_list1.InsertColumn(3, "Location", LVCFMT_LEFT, 55); // CD, SPU
//	m_list1.InsertColumn(4, "Vol", LVCFMT_LEFT, 35);
	m_list1.InsertColumn(4, "Loop", LVCFMT_LEFT, 40);
/*
	for(i=0;i<50;i++) {
		char buf[4];
		itoa(i,buf,10);
		m_list1.InsertItem(i,CString("item")+buf);
	}
*/

//	m_soundfilemanager.SetCurrentLevel("COMMON");
	m_tree1.Select(common,TVGN_CARET);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNslSoundManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNslSoundManagerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNslSoundManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CNslSoundManagerDlg::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	m_soundfilemanager.SetCurrentLevel(m_tree1.GetItemText(m_tree1.GetSelectedItem()));
	SoundFile& soundfile = m_soundfilemanager.GetCurrentLevel();

	m_list1.DeleteAllItems();

	// print out all the sound files to the display
	for(int i=0;i<soundfile.GetNumSounds();i++) {
		Sound& sound = soundfile.GetSound(i);
		int n = m_list1.InsertItem(0, sound.GetName());
		m_list1.SetItem(n, 1, LVIF_TEXT, (sound.IsEnabled()?"":"X"), NULL, NULL, NULL, NULL);
		m_list1.SetItem(n, 2, LVIF_TEXT, sound.GetType(), NULL, NULL, NULL, NULL);
		m_list1.SetItem(n, 3, LVIF_TEXT, sound.GetLocation(), NULL, NULL, NULL, NULL);
//		m_list1.SetItem(n, 4, LVIF_TEXT, sound.GetVol(), NULL, NULL, NULL, NULL);
		m_list1.SetItem(n, 4, LVIF_TEXT, (sound.IsLooped()?"loop":""), NULL, NULL, NULL, NULL);
	}
	*pResult = 0;
}


void CNslSoundManagerDlg::SetType(CString type)
{
	POSITION pos = m_list1.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   return;

	SoundFile& sf = m_soundfilemanager.GetCurrentLevel();
	int i;

	while (pos)
	{
		i = m_list1.GetNextSelectedItem(pos);
		CString name = m_list1.GetItemText(i,0);
		sf.GetSound(name).SetType(type);
		m_list1.SetItemText(i,2,type);
	}
}

void CNslSoundManagerDlg::SetLocation(CString location)
{
	POSITION pos = m_list1.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   return;

	SoundFile& sf = m_soundfilemanager.GetCurrentLevel();
	int i;

	while (pos)
	{
		i = m_list1.GetNextSelectedItem(pos);
		CString name = m_list1.GetItemText(i,0);
		sf.GetSound(name).SetLocation(location);
		m_list1.SetItemText(i,3,location);
	}
}

/*
void CNslSoundManagerDlg::OnButtonSetvolume() 
{
	POSITION pos = m_list1.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   return;

	SoundFile& sf = m_soundfilemanager.GetCurrentLevel();
	int i;

	while (pos)
	{
		i = m_list1.GetNextSelectedItem(pos);
		CString name = m_list1.GetItemText(i,0);
		sf.GetSound(name).SetVolume(m_volume.GetText());
		m_list1.SetItemText(i,4,m_volume.GetText());
	}
}
*/

void CNslSoundManagerDlg::OnButtonDisable() 
{
	POSITION pos = m_list1.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   return;

	SoundFile& sf = m_soundfilemanager.GetCurrentLevel();
	int i;

	while (pos)
	{
		i = m_list1.GetNextSelectedItem(pos);
		CString name = m_list1.GetItemText(i,0);
		sf.GetSound(name).Disable();
		m_list1.SetItemText(i,1,"X");
	}
}

void CNslSoundManagerDlg::OnButtonEnable() 
{
	POSITION pos = m_list1.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   return;

	SoundFile& sf = m_soundfilemanager.GetCurrentLevel();
	int i;

	while (pos)
	{
		i = m_list1.GetNextSelectedItem(pos);
		CString name = m_list1.GetItemText(i,0);
		sf.GetSound(name).Enable();
		m_list1.SetItemText(i,1,"");
	}
}

void CNslSoundManagerDlg::SetLoop(bool loop)
{
	POSITION pos = m_list1.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   return;

	SoundFile& sf = m_soundfilemanager.GetCurrentLevel();
	int i;

	while (pos)
	{
		i = m_list1.GetNextSelectedItem(pos);
		CString name = m_list1.GetItemText(i,0);
		if(loop) {
			sf.GetSound(name).SetLoop();
			m_list1.SetItemText(i,4,"loop");
		} else {
			sf.GetSound(name).ClearLoop();
			m_list1.SetItemText(i,4,"");
		}
	}
}

void CNslSoundManagerDlg::OnButtonSfx() 
{
	SetType("sfx");	
}

void CNslSoundManagerDlg::OnButtonAmbient() 
{
	SetType("ambient");	
}

void CNslSoundManagerDlg::OnButtonMusic() 
{
	SetType("music");	
}

void CNslSoundManagerDlg::OnButtonVoice() 
{
	SetType("voice");	
}


void CNslSoundManagerDlg::OnButtonSpu() 
{
	SetLocation("spu");
}

void CNslSoundManagerDlg::OnButtonCd() 
{
	SetLocation("cd");	
}


void CNslSoundManagerDlg::OnButtonDefault() 
{
	SetLocation("");	
}

void CNslSoundManagerDlg::OnButtonLoop() 
{
	SetLoop(true);
}

void CNslSoundManagerDlg::OnButtonNoloop() 
{
	SetLoop(false);	
}


void CNslSoundManagerDlg::OnEndlabeleditList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if(pDispInfo->item.pszText != NULL) { // make sure there's something to change it to
		int i = m_list1.GetSelectionMark();
		CString name = m_list1.GetItemText(i,0);
		m_list1.SetItem(i, 0, LVIF_TEXT, pDispInfo->item.pszText, NULL, NULL, NULL, NULL);
		m_soundfilemanager.GetCurrentLevel().GetSound(name).SetName(pDispInfo->item.pszText);
	}
	
	*pResult = 0;
}

void CNslSoundManagerDlg::OnButtonAddsound() 
{
	if(m_soundfilemanager.GetCurrentLevel().GetName().CompareNoCase("LEVELS")==0) {
		MessageBox("Can't add sounds here");
		return;
	}
	CString curdir = m_root_dir+SOURCE_SOUNDS_PATH;
	curdir.MakeUpper();
	curdir.TrimRight("\\");

	CFileDialog savebox(true, NULL, curdir+"\\*.WAV;*.AIF;*.AIFF", OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY, "AIFF/WAV files (*.wav;*.aif;*.aiff)|*.wav;*.aif;*.aiff|All files (*.*)|*.*||",NULL);
	if( savebox.DoModal() == IDOK ) {
		// add the files to the level and also to the list
		POSITION pos = savebox.GetStartPosition();
		while(pos) {
			CString name = savebox.GetNextPathName(pos);
			name.MakeUpper();
			// strip off m_root_dir+SOURCE_SOUNDS_PATH, if it's there
			if(name.Replace(curdir+"\\", "") != 1) {
				MessageBox("Sounds can only be added from\n"+curdir+"\n(or a subdirectory)","Error");
				return;
			}
			// remove .WAV, .AIF or .AIF extension
			name.Replace(".WAV","");
			name.Replace(".AIFF","");
			name.Replace(".AIF","");

			name.MakeLower();
			CString type("sfx");	// default type
			Sound& snd = m_soundfilemanager.GetCurrentLevel().AddSound(name);
			snd.SetType(type);
			int i = m_list1.InsertItem(m_soundfilemanager.GetCurrentLevel().GetNumSounds(), name);
			m_list1.SetItem(i, 2, LVIF_TEXT, type, NULL, NULL, NULL, NULL);
		}
	}
}

void CNslSoundManagerDlg::OnButtonRemove() 
{
	POSITION pos = m_list1.GetFirstSelectedItemPosition();
	if (pos == NULL)
	   return;

	if(MessageBox("Remove sound(s) from level?", "Confirm", MB_OKCANCEL) == IDCANCEL) {
		return;
	}

	SoundFile& sf = m_soundfilemanager.GetCurrentLevel();
	int i;

	while (pos)
	{
		pos = m_list1.GetFirstSelectedItemPosition();
		i = m_list1.GetNextSelectedItem(pos);
		m_soundfilemanager.GetCurrentLevel().RemoveSound(m_list1.GetItemText(i,0));
		m_list1.DeleteItem(i);
	}

}

void CNslSoundManagerDlg::OnButtonSave() 
{
	for(int i=0; i<m_soundfilemanager.GetNumLevels(); i++) {
		SoundFile& level = m_soundfilemanager.GetLevel(i);
		FILE* file = fopen(m_root_dir+"\\"+SOURCE_SOUNDS_PATH+ LEVELS + "\\"+level.GetName()+".SND", "w");
		if(file == NULL) {
			MessageBox("Couldn't open "+m_root_dir+"\\"+SOURCE_SOUNDS_PATH+ LEVELS + "\\"+level.GetName()+".SND"+ " for writing", "Error", MB_OK);
			return;
		}
		level.WriteSndFile(file);
		m_soundfilemanager.GetLevel("COMMON").WriteSndFile(file);
		fclose(file);
	}
}

void CNslSoundManagerDlg::OnButtonAddlevel() 
{
	CString newlevel("New Level");
	m_soundfilemanager.AddLevel(newlevel);
	m_tree1.InsertItem(newlevel, m_tree_levels);
}

void CNslSoundManagerDlg::OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here

	if( pTVDispInfo->item.pszText != NULL ) {
		HTREEITEM hti = m_tree1.GetSelectedItem();
		CString newlevelname = pTVDispInfo->item.pszText;
		newlevelname.TrimLeft();
		newlevelname.TrimRight();

		if( !(newlevelname.IsEmpty()) ) { // make sure there's something to change it to
			CString oldlevelname = m_soundfilemanager.GetCurrentLevel().GetName();
			m_soundfilemanager.GetCurrentLevel().SetName(pTVDispInfo->item.pszText);
			m_tree1.SetItem(hti, LVIF_TEXT, m_soundfilemanager.GetCurrentLevel().GetName(), NULL, NULL, NULL, NULL, NULL);
			// rename the .SND file to reflect the changes
			if(IDOK == MessageBox("Confirm rename of "+oldlevelname+".SND\nto "+pTVDispInfo->item.pszText+".SND","Confirm File Rename",MB_OKCANCEL))
				rename(m_root_dir+"\\"+SOURCE_SOUNDS_PATH + LEVELS + "\\"+oldlevelname+".SND",
					m_root_dir+"\\"+SOURCE_SOUNDS_PATH + LEVELS + "\\"+pTVDispInfo->item.pszText+".SND");
		}
	}

	*pResult = 0;
}

/*
// Sort the item in reverse alphabetical order.
static int CALLBACK
MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  // lParamSort contains a pointer to the list view control.
  // The lParam of an item is just its index.
  CNslSoundManagerDlg* dlg = (CNslSoundManagerDlg*) lParamSort;
  bool sign = (dlg->m_sortInfo < 0);
  int column = abs(dlg->m_sortInfo);
  CString    strItem1 = dlg->m_list1.GetItemText(lParam1, column);
  CString    strItem2 = dlg->m_list1.GetItemText(lParam2, column);

  if(sign)
	  return strcmp(strItem2, strItem1);
  else
	  return strcmp(strItem1, strItem2);
}
*/

extern CNslSoundManagerApp theApp;
void CNslSoundManagerDlg::OnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

/*	// this doesn't work
	int i = pNMListView->iSubItem;
	char a[4];
	itoa(i,a,10);
	MessageBox(a);
	m_list1.SortItems(MyCompareProc,(LPARAM) &(theApp.m_pMainWnd));
*/
	*pResult = 0;
}

void CNslSoundManagerDlg::OnButtonProcess() 
{
/*
(old) Usage: 
	nslSoundTool	[-D]
					[-I input_dir]
					[-S SND_file_dir]
					[-O output_dir]
					[-L output_level_snd_dir]
					[-P output_spu_dir]
					<SND files>

	
(new) Usage: NSLSoundTool.exe [OPTION]... [SND]...
Usage: NSLSoundTool.exe [OPTION]... [SND]...
 -D          Dry run, do not write output
 -Q          Be quiet
 -U          Ultra verbose mode (print out all files as processed)
 -T <lang>   Build sounds for the given language (see nsl.cpp for list)
 -F <file>   Global SND file for sound info defaults
 -I <dir>    Input WAV file directory
 -S <dir>    Input SND file directory
 -L <dir>    Output SND file directory name (e.g., "levels") [NOT SUPPORTED]
 -O <dir>    Output PS2 audio directory
 -P <dir>    Output PS2 SPU directory name (e.g., "spu")
 -X <dir>    Output Xbox audio directory
 -G <dir>    Output Gamecube audio directory
*/

	OnButtonSave(); // make sure our .SND files are up to date

	CString levels;
	for(int i=0;i<m_soundfilemanager.GetNumLevels();i++)
		levels += " " + m_soundfilemanager.GetLevel(i).GetName();

	CString optD(" -D");
	CString optQ("");
	CString optT("");
	CString optU(" -U");
	CString optF("");
	CString optI(" -I "+m_root_dir+SOURCE_SOUNDS_PATH);
	CString optS(" -S "+m_root_dir+SOURCE_SOUNDS_PATH+ LEVELS);
//	CString optL(" -L "+m_output_level_snd_dir);
	CString optO(" -O "+m_root_dir+DEST_SOUNDS_PATH+"\\"+PS2SOUND);
	CString optP(" -P "+m_output_spu_dir);
	CString optX("");
	CString optG("");

	// process only this level
	CString nslcmd
	(
		m_nslSoundToolPath +
//		optD +
		optQ +
		optT +
		optU +
		optF +
		optI +
		optS +
//		optL +
		optO +
		optP +
		optX +
		optG +
		levels +
		" & pause"
	);

	
//	MessageBox(nslcmd);
	system(nslcmd);

}

void CNslSoundManagerDlg::OnButtonProcessLevel() 
{
	if(m_soundfilemanager.GetCurrentLevel().GetName().CompareNoCase("LEVELS")==0
		|| m_soundfilemanager.GetCurrentLevel().GetName().CompareNoCase("COMMON")==0) {
		MessageBox(m_soundfilemanager.GetCurrentLevel().GetName() + " is not a valid level", "Error processing level", MB_OK);
		return;
	}

	// save only this level
	SoundFile& level = m_soundfilemanager.GetCurrentLevel();
	CString levelname = level.GetName();

	FILE* file = fopen(m_root_dir+"\\"+SOURCE_SOUNDS_PATH+ LEVELS + "\\"+levelname+".SND", "w");
	if(file == NULL) {
		MessageBox("Couldn't open "+m_root_dir+"\\"+SOURCE_SOUNDS_PATH+ LEVELS + "\\"+levelname+".SND"+ " for writing", "Error", MB_OK);
		return;
	}
	level.WriteSndFile(file);
	m_soundfilemanager.GetLevel("COMMON").WriteSndFile(file);
	fclose(file);

/*
Usage: NSLSoundTool.exe [OPTION]... [SND]...
 -D          Dry run, do not write output
 -Q          Be quiet
 -U          Ultra verbose mode (print out all files as processed)
 -T <lang>   Build sounds for the given language (see nsl.cpp for list)
 -F <file>   Global SND file for sound info defaults
 -I <dir>    Input WAV file directory
 -S <dir>    Input SND file directory
 -L <dir>    Output SND file directory name (e.g., "levels") [NOT SUPPORTED]
 -O <dir>    Output PS2 audio directory
 -P <dir>    Output PS2 SPU directory name (e.g., "spu")
 -X <dir>    Output Xbox audio directory
 -G <dir>    Output Gamecube audio directory
 
*/
	CString levels = " "+m_soundfilemanager.GetCurrentLevel().GetName();

	CString optD(" -D");
	CString optQ("");
	CString optT("");
	CString optU(" -U");
	CString optF("");
	CString optI(" -I "+m_root_dir+SOURCE_SOUNDS_PATH);
	CString optS(" -S "+m_root_dir+SOURCE_SOUNDS_PATH+ LEVELS);
	CString optL(" -L "+m_output_level_snd_dir);
	CString optO(" -O "+m_root_dir+DEST_SOUNDS_PATH+"\\"+PS2SOUND);
	CString optP(" -P "+m_output_spu_dir);
	CString optX("");
	CString optG("");


	// process only this level
//	CString levels = " "+m_soundfilemanager.GetCurrentLevel().GetName();
	CString nslcmd
	(
		m_nslSoundToolPath +
//		optD +
		optQ +
		optT +
		optU +
		optF +
		optI +
		optS +
//		optL +
		optO +
		optP +
		optX +
		optG +
		levels +
		" & pause"
	);

	
//	MessageBox(nslcmd);
	system(nslcmd);
}
