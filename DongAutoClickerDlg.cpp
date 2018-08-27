
// DongAutoClickerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "DongAutoClicker.h"
#include "DongAutoClickerDlg.h"
#include "afxdialogex.h"
#include "SetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "User32.lib")

static BOOL g_bStarted = FALSE;
static CSetupDlg g_dlgSetup;
static HHOOK g_hHook;
static CDongAutoClickerDlg* g_pDlg;
static UINT g_unRecordingIndex = 0;

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

		case WM_LBUTTONDBLCLK:
			TRACE("WM_LBUTTONDBLCLK\n");
			break;
		case WM_NCMOUSEMOVE:
			TRACE("WM_NCMOUSEMOVE\n");
			break;
		

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
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
			if( g_pDlg->m_vRecording.size() <= g_unRecordingIndex ) {
				if(g_pDlg->m_ctlRecordingSetup.GetCurSel() == 0) {
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
					g_unRecordingIndex = 0;
					TRACE("Repeat again\n");
					continue;
				} else {
					g_pDlg->OnBnClickedButtonStop();
					break;
				}
			}

			if( g_pDlg->m_vRecording[ g_unRecordingIndex ].m_dwDelayMilliSec <= GetTickCount() - g_pDlg->m_dwRecordingDelay ) {
				TRACE("Press Mouse - TimeClock = %d\n",g_pDlg->m_vRecording[ g_unRecordingIndex ].m_dwDelayMilliSec);

		
				INPUT input;
				input.type=INPUT_MOUSE;
				input.mi.mouseData=0;
				input.mi.dwExtraInfo=NULL;
				input.mi.time=0;

				input.mi.dwFlags=(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE);
				input.mi.dwFlags|= (
					g_pDlg->m_vRecording[ g_unRecordingIndex ].m_unMsg == WM_LBUTTONUP ? MOUSEEVENTF_LEFTUP :
					g_pDlg->m_vRecording[ g_unRecordingIndex ].m_unMsg == WM_LBUTTONDOWN ? MOUSEEVENTF_LEFTDOWN :
					g_pDlg->m_vRecording[ g_unRecordingIndex ].m_unMsg == WM_RBUTTONUP ? MOUSEEVENTF_RIGHTUP :
					MOUSEEVENTF_RIGHTDOWN
					);


				input.mi.dx=(LONG)((FLOAT)g_pDlg->m_vRecording[ g_unRecordingIndex ].m_stPoint.x * (65536.0f / GetSystemMetrics(SM_CXSCREEN)));
				input.mi.dy=(LONG)((FLOAT)g_pDlg->m_vRecording[ g_unRecordingIndex ].m_stPoint.y * (65536.0f / GetSystemMetrics(SM_CYSCREEN)));

				SendInput(1,&input,sizeof(INPUT));
				g_unRecordingIndex++;
			}

			Sleep(10);
		}
	} else {
		g_pDlg->OnBnClickedButtonStop();
	}


	TRACE("Recording Thread End\n");
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
				input.mi.dx=(LONG)((FLOAT)p.x * (65536.0f / GetSystemMetrics(SM_CXSCREEN)));
				input.mi.dy=(LONG)((FLOAT)p.y * (65536.0f / GetSystemMetrics(SM_CYSCREEN)));
			} else if(g_pDlg->m_nMousePosSetup == 1)
			{
				// Fixed Position
				TRACE("x : %u, y : %u\n",g_pDlg->m_dwFixPosX, g_pDlg->m_dwFixPosY);
				input.mi.dx=(LONG)((FLOAT)g_pDlg->m_dwFixPosX * (65536.0f / GetSystemMetrics(SM_CXSCREEN)));
				input.mi.dy=(LONG)((FLOAT)g_pDlg->m_dwFixPosY * (65536.0f / GetSystemMetrics(SM_CYSCREEN)));
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
	, m_unButtonOption(0)
	, m_unClickOption(0)
	, m_nMousePosSetup(0)
	, m_dwRepeatInterval(20)
	, m_bRecording(FALSE)
	, m_strRecording(_T(""))
	, m_strFilePath(_T(""))

{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_pDlg = this;
}

void CDongAutoClickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_INTERVAL_PRESET, m_ctlPresetInterval);
	DDX_Control(pDX, IDC_COMBO_RECORDING, m_ctlRecordingSetup);	
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
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_RECORDING, &CDongAutoClickerDlg::OnBnClickedButtonBrowseRecording)

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

	
	for(int i = 1 ; i <= 12 ; i++) {
		CString temp;
		temp.Format("F%d",i);
		m_ctlStartHotKey.AddString(temp);
		m_ctlStopHotKey.AddString(temp);
	}	

	for( UINT i = 0 ; i < m_vPresets.size() ; i++) {
		m_ctlPresetInterval.AddString( m_vPresets[i].first );
	}

	m_ctlRecordingSetup.AddString("반복 실행");
	m_ctlRecordingSetup.AddString("한번 실행");
	m_ctlRecordingSetup.SetCurSel(0);

	m_dwIntervalSec = 1;
	m_dwIntervalMilliSec = 0;
	

	HBITMAP hBmpTarget;
	hBmpTarget=::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_TARGET));
	m_btnTarget.SetBitmap(hBmpTarget);
	m_btnRecording.SetBitmap(hBmpTarget);

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

	if(m_fileRecording.m_hFile != CFile::hFileNull) {
		m_fileRecording.Close();
	}
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

			m_dwFixPosX = p.x;
			m_dwFixPosY = p.y;

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

		g_unRecordingIndex = 0;
		m_dwRecordingDelay = GetTickCount();

		if( m_nMousePosSetup == 2)
		{
			m_pWinThread = AfxBeginThread(RecordingThread,this);
		} else 
		{
			m_pWinThread = AfxBeginThread(WorkerThread,this);
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

void CDongAutoClickerDlg::OpenRecordingFile()
{
	if(m_fileRecording.m_hFile != CFile::hFileNull) {
		m_fileRecording.Close();
	}

	m_fileRecording.Open( m_strFilePath, CFile::modeReadWrite | CFile::modeNoTruncate | CFile::modeCreate, 0);
	LoadRecordingFromFile();
}

void CDongAutoClickerDlg::LoadRecordingFromFile()
{
	// Load recording data from file
	char szUnique = 'a';
	if(m_fileRecording.GetLength() > 0) {
		m_fileRecording.Read(&szUnique, 1);

		if(szUnique == 'z') {
			TRACE("This is dac file\n");
			m_vRecording.clear();

			UINT unSize;
			m_fileRecording.Read(&unSize, sizeof(UINT));

			TRACE("Recording Number %d\n", unSize);

			for(UINT unCnt = 0 ; unCnt < unSize ; unCnt++) 
			{
				CMouseInformation mi;
				m_fileRecording.Read(&mi, sizeof(mi));
				m_vRecording.push_back(mi);
			}
			m_strRecording.Format("%d 개",m_vRecording.size());

		} else {
			TRACE("This is NOT dac file\n");
		}
	}
}

void CDongAutoClickerDlg::SaveRecordingToFile()
{
	

	if(m_fileRecording.m_hFile != CFile::hFileNull)
	{
		m_fileRecording.Close();
	}

	m_fileRecording.Open( m_strFilePath, CFile::modeWrite | CFile::modeCreate, 0);
	char szUnique = 'z';
	UINT unRecordingNum = m_vRecording.size();
	m_fileRecording.Write( (const void*)&szUnique, sizeof(char) );
	m_fileRecording.Write( (const void*)&unRecordingNum, sizeof(UINT) );
	for(UINT i = 0 ; i < unRecordingNum ; i++) {
		//TRACE("Msg %#x Point %d %d Delay %d\n", m_vRecording[i].m_unMsg, m_vRecording[i].m_stPoint.x, m_vRecording[i].m_stPoint.y, m_vRecording[i].m_dwDelayMilliSec);
		m_fileRecording.Write( (const void*)&m_vRecording[i], sizeof(m_vRecording[i]) );
	}
	m_fileRecording.Close();
	
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
}


void CDongAutoClickerDlg::OnBnClickedRadioMousePosition2()
{
	UpdateData(TRUE);
}


void CDongAutoClickerDlg::OnBnClickedRadioMousePosition3()
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
			SaveRecordingToFile();

			LoadRecordingFromFile();
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
	UpdateData(TRUE);
	m_nMousePosSetup = 2;	
	
	if(m_bRecording == FALSE)
	{
		if(m_strFilePath.GetLength() <= 0 || m_fileRecording == 0) {
			OnBnClickedButtonBrowseRecording();
		}

		if(m_fileRecording.GetLength() > 0) {
			AfxMessageBox("파일 안 기존 내용은 삭제됩니다.");
		}

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

void CDongAutoClickerDlg::OnBnClickedButtonBrowseRecording()
{
	UpdateData(TRUE);
	m_nMousePosSetup = 2;

	CFileDialog dlgFile(TRUE, "dac", 0, 0,"DAC (*.dac)|*.dac|All (*.*)|*.*||");
	if(dlgFile.DoModal() == IDOK) {
		m_strFilePath = dlgFile.GetPathName();
		TRACE("File Path: %s\n",m_strFilePath);

		OpenRecordingFile();
	}

	UpdateData(FALSE);
}