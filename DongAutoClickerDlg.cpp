
// DongAutoClickerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "DongAutoClicker.h"
#include "DongAutoClickerDlg.h"
#include "afxdialogex.h"
#include "SetupDlg.h"
#include "SequenceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "User32.lib")

static BOOL g_bStarted = FALSE;
static CSetupDlg g_dlgSetup;
static HHOOK g_hHook;
static CDongAutoClickerDlg* g_pDlg;
static UINT g_unPlayIndex = 0;

static LRESULT CALLBACK MouseMsgProc(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
	{ 
		CallNextHookEx(g_hHook, nCode, wParam, lParam);
		return 0;
	}

	MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT *)lParam;
	if (nCode == 0)	{ // we have information in wParam/lParam ? If yes, let's check it:

		CMouseInformation mi;
		mi.m_unMsg = wParam;
		switch (wParam){
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			break;
		

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		
			mi.m_stPoint = pMouseStruct->pt;

			if(g_pDlg->m_vRecording.size() == 0) {
				g_pDlg->m_dwRecordingDelay = GetTickCount();
				mi.m_dwDelayMilliSec = 0;
			} else {
				mi.m_dwDelayMilliSec = GetTickCount() - g_pDlg->m_dwRecordingDelay;
			}
			g_pDlg->m_vRecording.push_back(mi);

			
			break;
		

		}

	}

	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
} 

static UINT RecordingThread(LPVOID pParam)
{
	TRACE("Recording Thread Started\n");
	TRACE("Recording Size = %d\n", g_pDlg->m_vRecording.size());
	if(g_pDlg->m_vRecording.size() > 0) 
	{
		while(g_bStarted) 
		{
			if( g_pDlg->m_vRecording.size() <= g_unPlayIndex ) {
				if(g_pDlg->m_ctlRepeatCheck.GetCheck() == TRUE) {
					if( g_pDlg->m_dwRepeatInterval > 0 ) {
						for(UINT uCnt = 0 ; uCnt < g_pDlg->m_dwRepeatInterval ; uCnt++) {
							Sleep(100);
							if(!g_bStarted) {
								TRACE("Recording Thread End #1\n");
								return 0;
							}
						}
					}

					g_pDlg->m_dwRecordingDelay = GetTickCount();
					g_unPlayIndex = 0;
					TRACE("Repeat again\n");
					continue;
				} else {
					g_pDlg->OnBnClickedButtonStop();
					break;
				}
			}

			if( g_pDlg->m_vRecording[ g_unPlayIndex ].m_dwDelayMilliSec <= GetTickCount() - g_pDlg->m_dwRecordingDelay ) {
				TRACE("Press Mouse - TimeClock = %d\n",g_pDlg->m_vRecording[ g_unPlayIndex ].m_dwDelayMilliSec);

		
				INPUT input;
				input.type=INPUT_MOUSE;
				input.mi.mouseData=0;
				input.mi.dwExtraInfo=NULL;
				input.mi.time=0;

				input.mi.dwFlags=(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE);
				input.mi.dwFlags|= (
					g_pDlg->m_vRecording[ g_unPlayIndex ].m_unMsg == WM_LBUTTONUP ? MOUSEEVENTF_LEFTUP :
					g_pDlg->m_vRecording[ g_unPlayIndex ].m_unMsg == WM_LBUTTONDOWN ? MOUSEEVENTF_LEFTDOWN :
					g_pDlg->m_vRecording[ g_unPlayIndex ].m_unMsg == WM_RBUTTONUP ? MOUSEEVENTF_RIGHTUP :
					MOUSEEVENTF_RIGHTDOWN
					);


				input.mi.dx=(LONG)((FLOAT)g_pDlg->m_vRecording[ g_unPlayIndex ].m_stPoint.x * (65535.0f / (GetSystemMetrics(SM_CXSCREEN)-1)));
				input.mi.dy=(LONG)((FLOAT)g_pDlg->m_vRecording[ g_unPlayIndex ].m_stPoint.y * (65535.0f / (GetSystemMetrics(SM_CYSCREEN)-1)));

				SendInput(1,&input,sizeof(INPUT));
				g_unPlayIndex++;
			}

			Sleep(1);
		}
	} else {
		g_pDlg->OnBnClickedButtonStop();
	}


	TRACE("Recording Thread End\n");
	return 0;
}

static UINT SequenceThread(LPVOID pParam)
{
	if(g_pDlg->m_vSequence.size() > 0) 
	{
		while(g_bStarted) 
		{
			if( g_pDlg->m_vSequence.size() <= g_unPlayIndex ) {
				if(g_pDlg->m_ctlRepeatCheck.GetCheck() == TRUE) {
					g_pDlg->m_dwRecordingDelay = GetTickCount();
					g_unPlayIndex = 0;
					TRACE("Repeat again\n");
					continue;
				} else {
					g_pDlg->OnBnClickedButtonStop();
					break;
				}
			}

			if(g_pDlg->m_vSequence[ g_unPlayIndex ].m_bDelayOnly == FALSE) 
			{
				INPUT input;
				input.type=INPUT_MOUSE;
				input.mi.mouseData=0;
				input.mi.dwExtraInfo=NULL;
				input.mi.time=0;

				input.mi.dx=(LONG)((FLOAT)g_pDlg->m_vSequence[ g_unPlayIndex ].m_stPoint.x * (65535.0f / (GetSystemMetrics(SM_CXSCREEN)-1)));
				input.mi.dy=(LONG)((FLOAT)g_pDlg->m_vSequence[ g_unPlayIndex ].m_stPoint.y * (65535.0f / (GetSystemMetrics(SM_CYSCREEN)-1)));

				input.mi.dwFlags=(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE);
				if( g_pDlg->m_vSequence[ g_unPlayIndex ].m_unMsg == WM_LBUTTONDOWN ) 
				{
					input.mi.dwFlags|= (MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP);
				} else if( g_pDlg->m_vSequence[ g_unPlayIndex ].m_unMsg == WM_RBUTTONDOWN ) 
				{
					input.mi.dwFlags|= (MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP);
				}

				for(UINT unCnt = 0 ; unCnt < g_pDlg->m_vSequence[ g_unPlayIndex ].m_dwRepeat ; unCnt++) {
					SendInput(1,&input,sizeof(INPUT));
					if( g_pDlg->m_vSequence[ g_unPlayIndex ].m_dwDelayMilliSec > 0 ) {
						for(UINT uCnt = 0 ; uCnt < g_pDlg->m_vSequence[ g_unPlayIndex ].m_dwDelayMilliSec ; uCnt++) {
							Sleep(100);
							if(!g_bStarted) {
								TRACE("Sequence Thread End #1\n");
								return 0;
							}
						}
					}
				}
				
			} else {
				for(UINT uCnt = 0 ; uCnt < g_pDlg->m_vSequence[ g_unPlayIndex ].m_dwDelayMilliSec ; uCnt++) {
					Sleep(100);
					if(!g_bStarted) {
						TRACE("Recording Thread End #2\n");
						return 0;
					}
				}
			}
			g_unPlayIndex++;
			Sleep(1);
		}

	}

	TRACE("Sequence Thread End\n");
	return 0;
}

static UINT WorkerThread(LPVOID pParam)
{
	DWORD OneOfTenSecondCount=0;

	TRACE("Button Option %d, Click Option %d\n", g_pDlg->m_unButtonOption, g_pDlg->m_unClickOption);

	while(g_bStarted) 
	{
		if(OneOfTenSecondCount == 0) 
		{
			POINT p;
			GetCursorPos(&p);
			
			INPUT input;
			input.type=INPUT_MOUSE;
			
			input.mi.dwFlags=(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE);
			g_pDlg->m_unButtonOption == 0 ?
			input.mi.dwFlags|= (MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP)
			:
			input.mi.dwFlags|= (MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP);			
			input.mi.mouseData=0;
			input.mi.dwExtraInfo=NULL;
			input.mi.time=0;

			if(g_pDlg->m_nMousePosSetup == 0)
			{

				// Current Mouse Position
				input.mi.dx=(LONG)((FLOAT)p.x * (65535.0f / (GetSystemMetrics(SM_CXSCREEN)-1)));
				input.mi.dy=(LONG)((FLOAT)p.y * (65535.0f / (GetSystemMetrics(SM_CYSCREEN)-1)));
			} else if(g_pDlg->m_nMousePosSetup == 1)
			{
				input.mi.dx=(LONG)((FLOAT)g_pDlg->m_dwFixPosX * (65535.0f / (GetSystemMetrics(SM_CXSCREEN)-1)));
				input.mi.dy=(LONG)((FLOAT)g_pDlg->m_dwFixPosY * (65535.0f / (GetSystemMetrics(SM_CYSCREEN)-1)));
			} 

			SendInput(1,&input,sizeof(INPUT));
			if(g_pDlg->m_unClickOption == 1)
			{
				SendInput(1,&input,sizeof(INPUT));
			}
		}

		if(++OneOfTenSecondCount == (g_pDlg->m_dwInterval/100))
		{
			OneOfTenSecondCount = 0;
		}

		Sleep(100);
	}
	
	TRACE("Worker Thread End\n");
	return 0;
}


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDongAutoClickerDlg 대화 상자




CDongAutoClickerDlg::CDongAutoClickerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDongAutoClickerDlg::IDD, pParent)
	, m_dwIntervalSec(0)
	, m_dwIntervalMilliSec(0)
	, m_bCapturing(FALSE)
	, m_dwFixPosX(0)
	, m_dwFixPosY(0)
	, m_dwCapturePosX(0)
	, m_dwCapturePosY(0)
	, m_unButtonOption(0)
	, m_unClickOption(0)
	, m_nMousePosSetup(0)
	, m_nSaveType(-1)
	, m_dwRepeatInterval(20)
	, m_bRecording(FALSE)
	, m_strRecording(_T(""))
	, m_strFilePath(_T(""))
	, m_dwSequenceInterval(0)
	, m_dwSequenceRepeat(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_pDlg = this;
}

void CDongAutoClickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_INTERVAL_PRESET, m_ctlPresetInterval);
	DDX_Text(pDX, IDC_EDIT_INTERVAL_S, m_dwIntervalSec);
	DDX_Text(pDX, IDC_EDIT_INTERVAL_MS, m_dwIntervalMilliSec);
	DDX_Control(pDX, IDC_SPIN_INTERVAL_S, m_ctlSpinIntervalSec);
	DDX_Control(pDX, IDC_SPIN_INTERVAL_MS, m_ctlSpinIntervalMilliSec);
	DDX_Control(pDX, IDC_BUTTON_SET_FIX_POSITION, m_btnTarget);
	DDV_MinMaxUInt(pDX, m_dwIntervalMilliSec, 0, 9);
	DDV_MinMaxUInt(pDX, m_dwIntervalSec, 0, 4294967295);
	DDX_Text(pDX, IDC_STATIC_X, m_dwFixPosX);
	DDX_Text(pDX, IDC_STATIC_Y, m_dwFixPosY);
	DDX_Radio(pDX, IDC_RADIO_MOUSE_POSITION, m_nMousePosSetup);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnPause);
	DDX_Control(pDX, IDC_COMBO_HOTKEY, m_ctlStartHotKey);
	DDX_Control(pDX, IDC_COMBO_HOTKEY2, m_ctlStopHotKey);
	DDX_Control(pDX, IDC_COMBO_BUTTON, m_ctlButtonOption);
	DDX_Control(pDX, IDC_COMBO_CLICK, m_ctlClickOption);
	DDX_Control(pDX, IDC_BUTTON_RECORDING, m_btnRecording);	
	DDX_Text(pDX, IDC_STATIC_N_REC, m_strRecording);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, m_strFilePath);
	DDX_Text(pDX, IDC_EDIT_REPEAT_INTERVAL, m_dwRepeatInterval);
	DDV_MinMaxUInt(pDX, m_dwRepeatInterval, 0, 4294967295);
	DDX_Control(pDX, IDC_BUTTON_SEQUENCE, m_btnSequence);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_INTERVAL, m_dwSequenceInterval);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_REPEAT_COUNT, m_dwSequenceRepeat);
	DDX_Control(pDX, IDC_LIST_SEQUENCE, m_ctlSequenceListBox);
	DDX_Control(pDX, IDC_CHECK_REPEAT, m_ctlRepeatCheck);
}

BEGIN_MESSAGE_MAP(CDongAutoClickerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDongAutoClickerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDongAutoClickerDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_START, &CDongAutoClickerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CDongAutoClickerDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_SETUP, &CDongAutoClickerDlg::OnBnClickedButtonSetup)
	ON_EN_CHANGE(IDC_EDIT_INTERVAL_MS, &CDongAutoClickerDlg::OnEnChangeEditIntervalMs)
	ON_EN_CHANGE(IDC_EDIT_INTERVAL_S, &CDongAutoClickerDlg::OnEnChangeEditIntervalS)
	ON_CBN_SELCHANGE(IDC_COMBO_INTERVAL_PRESET, &CDongAutoClickerDlg::OnCbnSelchangeComboIntervalPreset)
	ON_BN_CLICKED(IDC_BUTTON_SET_FIX_POSITION, &CDongAutoClickerDlg::OnBnClickedButtonSetFixPosition)
	ON_BN_CLICKED(IDC_RADIO_MOUSE_POSITION, &CDongAutoClickerDlg::OnBnClickedRadioMousePosition)
	ON_BN_CLICKED(IDC_RADIO_MOUSE_POSITION2, &CDongAutoClickerDlg::OnBnClickedRadioMousePosition2)
	ON_BN_CLICKED(IDC_RADIO_MOUSE_POSITION3, &CDongAutoClickerDlg::OnBnClickedRadioMousePosition3)
	ON_CBN_SELCHANGE(IDC_COMBO_BUTTON, &CDongAutoClickerDlg::OnCbnSelchangeComboButton)
	ON_CBN_SELCHANGE(IDC_COMBO_CLICK, &CDongAutoClickerDlg::OnCbnSelchangeComboClick)
	ON_WM_HOTKEY()
	ON_CBN_SELCHANGE(IDC_COMBO_HOTKEY, &CDongAutoClickerDlg::OnCbnSelchangeComboHotkey)
	ON_CBN_SELCHANGE(IDC_COMBO_HOTKEY2, &CDongAutoClickerDlg::OnCbnSelchangeComboHotkey2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_INTERVAL_S, &CDongAutoClickerDlg::OnDeltaposSpinIntervalS)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_INTERVAL_MS, &CDongAutoClickerDlg::OnDeltaposSpinIntervalMs)
	ON_BN_CLICKED(IDC_BUTTON_RECORDING, &CDongAutoClickerDlg::OnBnClickedButtonRecording)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_RECORDING, &CDongAutoClickerDlg::OnBnClickedButtonBrowse)

	ON_BN_CLICKED(IDC_BUTTON_SEQUENCE, &CDongAutoClickerDlg::OnBnClickedButtonSequence)
	ON_BN_CLICKED(IDC_BUTTON_SEQUENCE_IDLE, &CDongAutoClickerDlg::OnBnClickedButtonSequenceIdle)
	ON_BN_CLICKED(IDC_BUTTON_SEQUENCE_MODIFY, &CDongAutoClickerDlg::OnBnClickedButtonSequenceModify)
	ON_BN_CLICKED(IDC_BUTTON_SEQUENCE_DELETE, &CDongAutoClickerDlg::OnBnClickedButtonSequenceDelete)
	ON_BN_CLICKED(IDC_CHECK_REPEAT, &CDongAutoClickerDlg::OnBnClickedCheckRepeat)
	ON_BN_CLICKED(IDC_BUTTON_SEQUENCE_DELETE_ALL, &CDongAutoClickerDlg::OnBnClickedButtonSequenceDeleteAll)
	ON_BN_CLICKED(IDC_BUTTON_SEQUENCE_MOVE_UP, &CDongAutoClickerDlg::OnBnClickedButtonSequenceMoveUp)
	ON_BN_CLICKED(IDC_BUTTON_SEQUENCE_MOVE_DOWN, &CDongAutoClickerDlg::OnBnClickedButtonSequenceMoveDown)
	ON_BN_CLICKED(IDC_RADIO_MOUSE_POSITION4, &CDongAutoClickerDlg::OnBnClickedRadioMousePosition4)
END_MESSAGE_MAP()


// CDongAutoClickerDlg 메시지 처리기

BOOL CDongAutoClickerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_pWinThread = 0;
	m_dwInterval = 1000;

	m_vPresets.push_back( std::make_pair( "1/10 초간", 100 ) );
	m_vPresets.push_back( std::make_pair( "1 초간", 1000 ) );
	m_vPresets.push_back( std::make_pair( "2 초간", 2000 ) );
	m_vPresets.push_back( std::make_pair( "3 초간", 3000 ) );
	m_vPresets.push_back( std::make_pair( "5 초간", 5000 ) );
	m_vPresets.push_back( std::make_pair( "1 분간", 60000 ) );
	m_vPresets.push_back( std::make_pair( "5 분간", 300000 ) );
	m_vPresets.push_back( std::make_pair( "10 분간", 600000 ) );
	m_vPresets.push_back( std::make_pair( "1 시간", 3600000 ) );
	m_vPresets.push_back( std::make_pair( "하루에 한번", 86400000 ) );

	m_ctlSequenceListBox.InsertColumn(0, "PosX");
	m_ctlSequenceListBox.SetColumnWidth(0, 40);
	m_ctlSequenceListBox.InsertColumn(1, "PosY");
	m_ctlSequenceListBox.SetColumnWidth(1, 40);
	m_ctlSequenceListBox.InsertColumn(2, "Repeat");
	m_ctlSequenceListBox.SetColumnWidth(2, 55);
	m_ctlSequenceListBox.InsertColumn(3, "Delay");
	m_ctlSequenceListBox.SetColumnWidth(3, 45);
	m_ctlSequenceListBox.InsertColumn(4, "Btn");
	m_ctlSequenceListBox.SetColumnWidth(4, 35);

	m_ctlRepeatCheck.SetCheck(TRUE);

	m_ctlSequenceListBox.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	
	for(int i = 1 ; i <= 12 ; i++) {
		CString temp;
		temp.Format("F%d",i);
		m_ctlStartHotKey.AddString(temp);
		m_ctlStopHotKey.AddString(temp);
	}	

	for( UINT i = 0 ; i < m_vPresets.size() ; i++) {
		m_ctlPresetInterval.AddString( m_vPresets[i].first );
	}

	m_dwIntervalSec = 1;
	m_dwIntervalMilliSec = 0;
	m_dwSequenceInterval = 10;
	m_dwSequenceRepeat = 1;
	

	HBITMAP hBmpTarget;
	hBmpTarget=::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_TARGET));
	m_btnTarget.SetBitmap(hBmpTarget);
	m_btnRecording.SetBitmap(hBmpTarget);
	m_btnSequence.SetBitmap(hBmpTarget);

	m_btnPause.EnableWindow(FALSE);
	m_ctlStartHotKey.SetCurSel(1);
	m_ctlStopHotKey.SetCurSel(2);
	m_ctlButtonOption.SetCurSel(m_unButtonOption);
	m_ctlClickOption.SetCurSel(m_unClickOption);

	m_strRecording = "녹화 안됨";

	SetupHotKey();

	UpdateData(FALSE);
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CDongAutoClickerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CDongAutoClickerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CDongAutoClickerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDongAutoClickerDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


void CDongAutoClickerDlg::OnBnClickedCancel()
{
	g_bStarted = FALSE;
	CheckThreadTerminate();

	if(m_bRecording == TRUE)
	{
		if(UnhookWindowsHookEx(g_hHook)) {
			g_hHook = NULL;
		}
	}
	TRACE("OnCancel\n");

	/*
	if(m_fileRecording.m_hFile != CFile::hFileNull) {
		m_fileRecording.Close();
	}
	*/
	CDialogEx::OnCancel();
}


void CDongAutoClickerDlg::OnClose()
{
	TRACE("OnClose\n");
	CDialogEx::OnClose();
}


BOOL CDongAutoClickerDlg::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN )
	{
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
			return TRUE; // Ignore Enter and Escape Keys
		}
	}
	
	if( pMsg->message == WM_LBUTTONUP )
	{
		if(m_bCapturing == TRUE)
		{
			POINT p;

			m_bCapturing = FALSE;

			GetCursorPos(&p);
			m_dwCapturePosX = p.x;
			m_dwCapturePosY = p.y;

			if( m_nMousePosSetup == 1) { // Fix Position
				m_dwFixPosX = p.x;
				m_dwFixPosY = p.y;
			} else if(m_nMousePosSetup == 3) { // Sequence Position
				AddNewSequence();
				SaveToFile(TRUE);
			}

			// 릴리즈 할 필요 없다. 취소의 개념이 있을 수 없다.
			ReleaseCapture();
			UpdateData(FALSE);

		}
	} else if( pMsg->message == WM_RBUTTONUP )
	{
		if(m_bCapturing == TRUE && m_nMousePosSetup == 3) // Sequence
		{
			POINT p;

			m_bCapturing = FALSE;

			GetCursorPos(&p);
			m_dwCapturePosX = p.x;
			m_dwCapturePosY = p.y;

			AddNewSequence(FALSE, TRUE);
			SaveToFile(TRUE);

			// 릴리즈 할 필요 없다. 취소의 개념이 있을 수 없다.
			ReleaseCapture();
			UpdateData(FALSE);
		}
	}


	return CDialogEx::PreTranslateMessage(pMsg);
}




void CDongAutoClickerDlg::OnBnClickedButtonStart()
{
	UpdateData(TRUE);
	TRACE("%d\n",m_nMousePosSetup);

	m_dwInterval = m_dwIntervalSec*1000 + m_dwIntervalMilliSec*100;
	TRACE("Interval is %u\n", m_dwInterval);



	if(!g_bStarted) {
		m_btnStart.EnableWindow(FALSE);
		m_btnPause.EnableWindow(TRUE);

		g_bStarted = TRUE;
		TRACE("Started %d\n",g_bStarted);

		g_unPlayIndex = 0;
		m_dwRecordingDelay = GetTickCount();

		if( m_nMousePosSetup < 2)
		{
			m_pWinThread = AfxBeginThread(WorkerThread,this);
		} else if( m_nMousePosSetup == 2)
		{
			m_pWinThread = AfxBeginThread(RecordingThread,this);
		} else if( m_nMousePosSetup == 3)
		{
			m_pWinThread = AfxBeginThread(SequenceThread,this);
		}

	}

	
}


void CDongAutoClickerDlg::OnBnClickedButtonStop()
{
	TRACE("Stopping");

	g_bStarted = FALSE;
	CheckThreadTerminate();

	TRACE("Stopped");
	m_btnStart.EnableWindow(TRUE);
	m_btnPause.EnableWindow(FALSE);
}

void CDongAutoClickerDlg::CheckThreadTerminate()
{
	if(g_bStarted) {
		TRACE("Wait for thread termination\n");
		::WaitForSingleObject(m_pWinThread->m_hThread,INFINITE);
		TRACE("Thread terminated\n");
	}
}

void CDongAutoClickerDlg::SetupHotKey()
{
	// 존재하고 있다면 먼저 삭제한다.
	UnregisterHotKey(this->m_hWnd, 100);	// Start Hot Key
	UnregisterHotKey(this->m_hWnd, 101);	// Stop Hot Key

	RegisterHotKey(this->m_hWnd, 100, MOD_CONTROL, VK_F1 + m_ctlStartHotKey.GetCurSel() );
	RegisterHotKey(this->m_hWnd, 101, MOD_CONTROL, VK_F1 + m_ctlStopHotKey.GetCurSel() );
	RegisterHotKey(this->m_hWnd, 1000, 0, VK_F4 );
}

void CDongAutoClickerDlg::LoadMouseInformationFromFile()
{
	/*
	if(m_fileRecording.m_hFile != CFile::hFileNull) {
		m_fileRecording.Close();
	}
	*/
	// 시퀀스와 레코딩 모두를 클리어하여 두개가 공존하게 하지 않는다.
	m_vRecording.clear();
	m_vSequence.clear();
	m_strRecording.Format("0 개");
	m_ctlSequenceListBox.DeleteAllItems();

	m_fileRecording.Open( m_strFilePath, CFile::modeReadWrite | CFile::modeNoTruncate | CFile::modeCreate, 0);

	// Load recording data from file
	char szUnique[3];
	char szType;
	if(m_fileRecording.GetLength() > 4) {
		m_fileRecording.SeekToBegin();
		m_fileRecording.Read(szUnique, 3);
		if( szUnique[0] == 'd' && szUnique[1] == 'a' && szUnique[2] == 'c' ) {
			m_fileRecording.Read(&szType, 1);			

			UINT unSize;
			m_fileRecording.Read(&unSize, sizeof(UINT));
			TRACE("항목 개수는 %d\n",unSize);
			

			if(szType == 'r') {
				TRACE("This is recording file\n");
				m_nSaveType = 2;
				m_nMousePosSetup = 2;

				for(UINT unCnt = 0 ; unCnt < unSize ; unCnt++) 
				{
					CMouseInformation mi;
					m_fileRecording.Read(&mi, sizeof(mi));
					m_vRecording.push_back(mi);
				}

				m_strRecording.Format("%d 개",m_vRecording.size());
			} else if(szType == 's') {
				TRACE("This is sequence file\n");
				m_nSaveType = 3;
				m_nMousePosSetup = 3;
				m_ctlSequenceListBox.DeleteAllItems();

				for(UINT unCnt = 0 ; unCnt < unSize ; unCnt++) 
				{
					CMouseInformation mi;
					m_fileRecording.Read(&mi, sizeof(mi));
					m_vSequence.push_back(mi);

					AddSequenceOneLine(unCnt, mi);
					
				}
				TRACE("%d개 항목 로드 완료\n", m_vSequence.size());


			} else {
				TRACE("I don't know the file format %c\n", szUnique);
			}


		} else {
			TRACE("This is NOT dac file %c %c %c\n",szUnique[0], szUnique[1], szUnique[2]);

		}
	} else {
		TRACE("This is NOT dac file, size = %d\n",m_fileRecording.GetLength());
	}

	m_fileRecording.Close();
	UpdateData(FALSE);
}

void CDongAutoClickerDlg::SaveToFile(BOOL bSequence)
{
	/*
	if(m_fileRecording.m_hFile != CFile::hFileNull)
	{
		m_fileRecording.Close();
	}
	*/

	std::vector< CMouseInformation >* m_vPtr = &m_vRecording;
	m_fileRecording.Open( m_strFilePath, CFile::modeWrite | CFile::modeCreate, 0);
	char szUnique[3] = { 'd', 'a', 'c' };
	char szType = 'r';
	m_nSaveType = 2;
	if(bSequence == TRUE) {
		szType = 's';
		m_vPtr = &m_vSequence;
		m_nSaveType = 3;
	} 
	UINT unItemNum = m_vPtr->size();
	m_fileRecording.Write( (const void*)szUnique, sizeof(szUnique) );
	m_fileRecording.Write( (const void*)&szType, sizeof(char) );
	m_fileRecording.Write( (const void*)&unItemNum, sizeof(UINT) );
	for(UINT i = 0 ; i < unItemNum ; i++) {
		TRACE("Msg %#x Point %d %d Delay %d\n", (*m_vPtr)[i].m_unMsg, (*m_vPtr)[i].m_stPoint.x, (*m_vPtr)[i].m_stPoint.y, (*m_vPtr)[i].m_dwDelayMilliSec);
		m_fileRecording.Write( (const void*)&(*m_vPtr)[i], sizeof((*m_vPtr)[i]) );
	}
	m_fileRecording.Close();
}

BOOL CDongAutoClickerDlg::CheckBeforePressButton()
{
	if(m_bRecording == TRUE || g_bStarted == TRUE || m_bCapturing == TRUE) {
		AfxMessageBox("레코딩 중이거나, 작동 중에는 해당 기능이 작동하지 않습니다.");
		return FALSE;
	}
	return TRUE;
}

void CDongAutoClickerDlg::OnBnClickedButtonSetup()
{
	UpdateData(TRUE);
	g_dlgSetup.DoModal();
}


void CDongAutoClickerDlg::OnEnChangeEditIntervalMs()
{

	UpdateData(TRUE);
	
	if(m_dwIntervalMilliSec > 9) {
		m_dwIntervalMilliSec = 9;
		UpdateData(FALSE);
	} else if(m_dwIntervalMilliSec < 0) {
		m_dwIntervalMilliSec = 0;
		UpdateData(FALSE);
	} else if(m_dwIntervalMilliSec == 0 && m_dwIntervalSec == 0) {
		m_dwIntervalMilliSec = 1;
		UpdateData(FALSE);
	}
	
}


void CDongAutoClickerDlg::OnEnChangeEditIntervalS()
{

	UpdateData(TRUE);

	if(m_dwIntervalSec < 0) {
		m_dwIntervalSec = 0;
		UpdateData(FALSE);
	} else if(m_dwIntervalMilliSec == 0 && m_dwIntervalSec == 0) {
		m_dwIntervalSec = 1;
		UpdateData(FALSE);
	}
	
}


void CDongAutoClickerDlg::OnCbnSelchangeComboIntervalPreset()
{
	DWORD dwTempValue = m_vPresets[ m_ctlPresetInterval.GetCurSel() ].second;
	TRACE("Preset %u\n", dwTempValue);

	m_dwIntervalSec = dwTempValue / 1000;
	m_dwIntervalMilliSec = (dwTempValue % 1000)/100;

	UpdateData(FALSE);
}


void CDongAutoClickerDlg::OnBnClickedButtonSetFixPosition()
{
	if(CheckBeforePressButton() == FALSE) return;

	// 마우스 캡쳐 시작.
	UpdateData(TRUE);
	m_nMousePosSetup = 1;
	UpdateData(FALSE);

	m_bCapturing = TRUE;
	SetCapture();
}


void CDongAutoClickerDlg::OnBnClickedRadioMousePosition()
{
	UpdateData(TRUE);
	m_ctlRepeatCheck.SetCheck(TRUE);
}


void CDongAutoClickerDlg::OnBnClickedRadioMousePosition2()
{
	UpdateData(TRUE);
	m_ctlRepeatCheck.SetCheck(TRUE);
}


void CDongAutoClickerDlg::OnBnClickedRadioMousePosition3()
{
	
	UpdateData(TRUE);
}

void CDongAutoClickerDlg::OnBnClickedRadioMousePosition4()
{
	
	UpdateData(TRUE);
}


void CDongAutoClickerDlg::OnCbnSelchangeComboButton()
{
	m_unButtonOption = m_ctlButtonOption.GetCurSel();
}


void CDongAutoClickerDlg::OnCbnSelchangeComboClick()
{
	m_unClickOption = m_ctlClickOption.GetCurSel();
}


void CDongAutoClickerDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if(nHotKeyId == 100) {
		OnBnClickedButtonStart();
	} else if(nHotKeyId == 101) {
		OnBnClickedButtonStop();
	} else if(nHotKeyId == 1000) {
		if(m_bRecording == TRUE) {
			m_bRecording = FALSE;
			//UnregisterHotKey(this->m_hWnd, 1000);
			
			if(UnhookWindowsHookEx(g_hHook)) {
				g_hHook = NULL;
			}
			AfxMessageBox("Recording Stopped\n");

			for(UINT i = 0 ; i < m_vRecording.size() ; i++) {
				TRACE("Msg %#x Point %d %d Delay %d\n", m_vRecording[i].m_unMsg, m_vRecording[i].m_stPoint.x, m_vRecording[i].m_stPoint.y, m_vRecording[i].m_dwDelayMilliSec);
			}
			SaveToFile();

			LoadMouseInformationFromFile();
			UpdateData(FALSE);
		}
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CDongAutoClickerDlg::OnCbnSelchangeComboHotkey()
{
	if(m_ctlStartHotKey.GetCurSel() == m_ctlStopHotKey.GetCurSel()) {
		AfxMessageBox("시작키와 멈춤키가 동일할 수 없습니다.");
		if(m_ctlStopHotKey.GetCurSel() != 1) {
			m_ctlStartHotKey.SetCurSel(1);
		} else {
			m_ctlStartHotKey.SetCurSel(0);
		}
	}
	SetupHotKey();

}


void CDongAutoClickerDlg::OnCbnSelchangeComboHotkey2()
{
	if(m_ctlStartHotKey.GetCurSel() == m_ctlStopHotKey.GetCurSel()) {
		AfxMessageBox("시작키와 멈춤키가 동일할 수 없습니다.");
		if(m_ctlStartHotKey.GetCurSel() != 2) {
			m_ctlStopHotKey.SetCurSel(2);
		} else {
			m_ctlStopHotKey.SetCurSel(3);
		}
	}
	SetupHotKey();
}


void CDongAutoClickerDlg::OnDeltaposSpinIntervalS(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if(pNMUpDown->iDelta > 0 && m_dwIntervalSec == 0) {
		
	} else {

		m_dwIntervalSec -= pNMUpDown->iDelta;

		if(m_dwIntervalSec < 0 ) {
			m_dwIntervalSec = 0;
		}

		UpdateData(FALSE);
	}

	*pResult = 0;
}


void CDongAutoClickerDlg::OnDeltaposSpinIntervalMs(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if(pNMUpDown->iDelta > 0 && m_dwIntervalMilliSec == 0) {
		
	} else if(pNMUpDown->iDelta < 0 && m_dwIntervalMilliSec == 9) {
		
	} else {

		m_dwIntervalMilliSec -= pNMUpDown->iDelta;

		if(m_dwIntervalMilliSec < 0 ) {
			m_dwIntervalMilliSec = 0;
		}

		UpdateData(FALSE);
	}

	*pResult = 0;
}

void CDongAutoClickerDlg::OnBnClickedButtonRecording()
{
	if(CheckBeforePressButton() == FALSE) return;

	UpdateData(TRUE);
	m_nMousePosSetup = 2;
	UpdateData(FALSE);
	
	if(m_bRecording == FALSE)
	{
		if( m_nSaveType == 3) {
			if( MessageBox("시퀀스 파일이 레코딩 파일로 바뀝니다. 계속 하시겠습니까?\n새로운 파일을 이용하는 것을 권장합니다.","경고",MB_YESNO) == IDNO )
			{
				return;
			} else 
			{
				m_ctlSequenceListBox.DeleteAllItems();
				m_vSequence.clear();
			}
		}

		if(m_strFilePath.GetLength() <= 0 /*|| m_fileRecording == CFile::hFileNull*/) {
			OnBnClickedButtonBrowse();
		}

		/*
		if(m_fileRecording.GetLength() > 0) {
			AfxMessageBox("파일 안 기존 내용은 삭제됩니다.");
		}
		*/

		m_vRecording.clear();
		g_hHook = SetWindowsHookEx( WH_MOUSE_LL, (HOOKPROC)MouseMsgProc, 0, 0 );
		if(g_hHook == NULL) {
		
			INT nError = GetLastError();
			CString strErr;
			strErr.Format("Failed to hook system: Error No %#x",nError);
			AfxMessageBox(strErr);			
			m_bRecording = FALSE;
			m_strRecording = "녹화 안됨";
		} else {
			m_bRecording = TRUE;
			m_strRecording = "레코딩 중...";

		}

	}

	UpdateData(FALSE);
}

void CDongAutoClickerDlg::OnBnClickedButtonBrowse()
{
	if(CheckBeforePressButton() == FALSE) return;

	UpdateData(TRUE);

	CFileDialog dlgFile(TRUE, "dac", 0, 0,"DAC (*.dac)|*.dac|All (*.*)|*.*||");
	if(dlgFile.DoModal() == IDOK) {
		m_strFilePath = dlgFile.GetPathName();
		TRACE("File Path: %s\n",m_strFilePath);

		LoadMouseInformationFromFile();
	}

	UpdateData(FALSE);
}

void CDongAutoClickerDlg::OnBnClickedButtonSequence()
{
	if(CheckBeforePressButton() == FALSE) return;

	UpdateData(TRUE);
	m_nMousePosSetup = 3;
	UpdateData(FALSE);

	if( m_nSaveType == 2) {
		if( MessageBox("레코딩 파일이 시퀀스 파일로 바뀝니다. 계속 하시겠습니까?\n새로운 파일을 이용하는 것을 권장합니다.","경고",MB_YESNO) == IDNO )
		{
			return;
		} else 
		{
			m_strRecording.Format("0 개");
			m_vRecording.clear();
			UpdateData(FALSE);
		}
	}

	if(m_strFilePath.GetLength() <= 0 /*|| m_fileRecording == 0*/) {
		OnBnClickedButtonBrowse();
		return;
	}

	m_bCapturing = TRUE;
	SetCapture();
}


void CDongAutoClickerDlg::OnBnClickedButtonSequenceIdle()
{
	if(CheckBeforePressButton() == FALSE) return;

	if( m_nSaveType == 2) {
		if( MessageBox("레코딩 파일이 시퀀스 파일로 바뀝니다. 계속 하시겠습니까?\n새로운 파일을 이용하는 것을 권장합니다.","경고",MB_YESNO) == IDNO )
		{
			return;
		} else 
		{
			m_strRecording.Format("0 개");
			m_vRecording.clear();
			UpdateData(FALSE);
		}
	}

	AddNewSequence(TRUE);
	SaveToFile(TRUE);
}


void CDongAutoClickerDlg::OnBnClickedButtonSequenceModify()
{
	INT nIndex = m_ctlSequenceListBox.GetSelectionMark();
	if(CheckBeforePressButton() == FALSE) return;
	if(nIndex < 0) return;

	

	CSequenceDlg dlg;
	dlg.m_bDelayOnly = m_vSequence[ nIndex ].m_bDelayOnly;
	if(m_vSequence[ nIndex ].m_bDelayOnly == FALSE) {
		dlg.m_dwDelay = m_vSequence[ nIndex ].m_dwDelayMilliSec;
		dlg.m_dwPositionX = m_vSequence[ nIndex ].m_stPoint.x;
		dlg.m_dwPositionY = m_vSequence[ nIndex ].m_stPoint.y;
		dlg.m_dwRepeat = m_vSequence[ nIndex ].m_dwRepeat;
		dlg.m_nButtonPosition = m_vSequence[ nIndex ].m_unMsg == WM_LBUTTONDOWN ? 0 : 1;
	}

	if( dlg.DoModal() == IDOK ) 
	{
		m_vSequence[ nIndex ].m_bDelayOnly = dlg.m_bDelayOnly;
		if( !m_vSequence[ nIndex ].m_bDelayOnly )
		{
			m_vSequence[ nIndex ].m_dwDelayMilliSec = dlg.m_dwDelay;
			m_vSequence[ nIndex ].m_dwRepeat = dlg.m_dwRepeat;
			m_vSequence[ nIndex ].m_unMsg = (dlg.m_nButtonPosition == 0 ? WM_LBUTTONDOWN : WM_RBUTTONDOWN);
			m_vSequence[ nIndex ].m_stPoint.x = dlg.m_dwPositionX;
			m_vSequence[ nIndex ].m_stPoint.y = dlg.m_dwPositionY;

			AddSequenceAllWithVector();
			SaveToFile(TRUE);
		}
	}
}


void CDongAutoClickerDlg::OnBnClickedButtonSequenceDelete()
{
	if(CheckBeforePressButton() == FALSE) return;

	if( m_ctlSequenceListBox.GetSelectionMark() >= 0) {
		TRACE("Selected Item is %d=n", m_ctlSequenceListBox.GetSelectionMark());

		m_vSequence.erase( m_vSequence.begin() + m_ctlSequenceListBox.GetSelectionMark() );
		m_ctlSequenceListBox.DeleteItem(m_ctlSequenceListBox.GetSelectionMark());

		SaveToFile(TRUE);
	}
}

void CDongAutoClickerDlg::AddNewSequence(BOOL bDelayOnly, BOOL bRightClick)
{
	// x, y, rep, del, btn, style
	UpdateData(TRUE);

	UINT unIndex = m_vSequence.size();

	CMouseInformation mi;
	mi.m_dwDelayMilliSec = m_dwSequenceInterval;
	mi.m_dwRepeat = m_dwSequenceRepeat;
	mi.m_bDelayOnly = bDelayOnly;
	mi.m_stPoint.x = m_dwCapturePosX;
	mi.m_stPoint.y = m_dwCapturePosY;



	if( bRightClick == FALSE )
	{
		mi.m_unMsg = WM_LBUTTONDOWN;
	} else 
	{
		mi.m_unMsg = WM_RBUTTONDOWN;
	}

	m_vSequence.push_back(mi);
	AddSequenceOneLine(unIndex, mi);
}

void CDongAutoClickerDlg::AddSequenceColumn(UINT unIndex, UINT unSubItem, char* szText)
{
	LVITEM lv;
	lv.iItem = unIndex;
	lv.iSubItem = unSubItem;
	lv.pszText = szText;
	lv.mask = LVIF_TEXT;
	if(unSubItem == 0) {
		m_ctlSequenceListBox.InsertItem(&lv);
	} else {
		m_ctlSequenceListBox.SetItem(&lv);
	}
}

void CDongAutoClickerDlg::AddSequenceOneLine(UINT unIndex, CMouseInformation mi)
{
	CString strTemp;
	AddSequenceColumn(unIndex, 0, mi.m_bDelayOnly ? "-" : (strTemp.Format("%d", mi.m_stPoint.x), strTemp.GetBuffer()));
	AddSequenceColumn(unIndex, 1, mi.m_bDelayOnly ? "-" : (strTemp.Format("%d", mi.m_stPoint.y), strTemp.GetBuffer()));
	AddSequenceColumn(unIndex, 2, mi.m_bDelayOnly ? "-" : (strTemp.Format("%d", mi.m_dwRepeat), strTemp.GetBuffer()));
	AddSequenceColumn(unIndex, 3, (strTemp.Format("%d", mi.m_dwDelayMilliSec), strTemp.GetBuffer()));
	AddSequenceColumn(unIndex, 4, mi.m_bDelayOnly ? "-" : ( mi.m_unMsg == WM_LBUTTONDOWN ? "L" : "R" ));
}

void CDongAutoClickerDlg::AddSequenceAllWithVector()
{
	m_ctlSequenceListBox.DeleteAllItems();
	for(UINT unCnt = 0 ; unCnt < m_vSequence.size() ; unCnt++)
	{
		CMouseInformation mi;
		mi.m_bDelayOnly = m_vSequence[unCnt].m_bDelayOnly;
		mi.m_dwDelayMilliSec = m_vSequence[unCnt].m_dwDelayMilliSec;
		mi.m_dwRepeat = m_vSequence[unCnt].m_dwRepeat;
		mi.m_stPoint = m_vSequence[unCnt].m_stPoint;
		mi.m_unMsg = m_vSequence[unCnt].m_unMsg;

		AddSequenceOneLine( unCnt, mi );
	}

}

void CDongAutoClickerDlg::OnBnClickedCheckRepeat()
{
	UpdateData(TRUE);
	if(m_nMousePosSetup <= 1) {
		m_ctlRepeatCheck.SetCheck(TRUE);
	}
}


void CDongAutoClickerDlg::OnBnClickedButtonSequenceDeleteAll()
{
	if(CheckBeforePressButton() == FALSE) return;

	if(m_vSequence.size() > 0) {

		if(MessageBox("파일의 모든 내용이 삭제 됩니다.","주의1!", MB_YESNO) == IDYES) {
			m_ctlSequenceListBox.DeleteAllItems();
			m_vSequence.clear();
			SaveToFile(TRUE);
		}
	} else {
		AfxMessageBox("삭제할 내용이 없습니다.");
	}
}


void CDongAutoClickerDlg::OnBnClickedButtonSequenceMoveUp()
{
	INT nIndex = m_ctlSequenceListBox.GetSelectionMark();


	if(nIndex <= 0) {
		return;
	} else {
		CMouseInformation mi = m_vSequence[nIndex - 1];
		m_vSequence.erase( m_vSequence.begin() + nIndex - 1 );
		m_vSequence.insert( m_vSequence.begin() + nIndex, mi );

		AddSequenceAllWithVector();
		SaveToFile(TRUE);

		m_ctlSequenceListBox.SetItemState( nIndex - 1, LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED );
		m_ctlSequenceListBox.SetFocus();
	}
}


void CDongAutoClickerDlg::OnBnClickedButtonSequenceMoveDown()
{
	INT nIndex = m_ctlSequenceListBox.GetSelectionMark();
	if(nIndex == m_vSequence.size() - 1 || nIndex < 0) {
		return;
	} else {

		CMouseInformation mi = m_vSequence[nIndex];
		m_vSequence.erase( m_vSequence.begin() + nIndex );
		m_vSequence.insert( m_vSequence.begin() + nIndex + 1, mi );

		AddSequenceAllWithVector();
		SaveToFile(TRUE);

		m_ctlSequenceListBox.SetItemState( nIndex + 1, LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED );
		m_ctlSequenceListBox.SetFocus();
	}
}
