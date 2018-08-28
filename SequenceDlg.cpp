// SequenceDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DongAutoClicker.h"
#include "SequenceDlg.h"
#include "afxdialogex.h"


// CSequenceDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSequenceDlg, CDialogEx)

CSequenceDlg::CSequenceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSequenceDlg::IDD, pParent)
	, m_dwPositionX(0)
	, m_dwPositionY(0)
	, m_dwDelay(10)
	, m_dwRepeat(1)
	, m_bDelayOnly(FALSE)
	, m_nButtonPosition(0)
{

}

CSequenceDlg::~CSequenceDlg()
{
}

void CSequenceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_POS_X, m_dwPositionX);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_POS_Y, m_dwPositionY);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_DELAY, m_dwDelay);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_REPEAT, m_dwRepeat);
	DDX_Control(pDX, IDC_CHECK_SEQUENCE_DELAY_ONLY, m_ctlDelayOnly);
	DDX_Radio(pDX, IDC_RADIO_SEQUENCE_BUTTON_POS, m_nButtonPosition);
	DDX_Check(pDX, IDC_CHECK_SEQUENCE_DELAY_ONLY, m_bDelayOnly);
}


BEGIN_MESSAGE_MAP(CSequenceDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSequenceDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSequenceDlg 메시지 처리기입니다.


void CSequenceDlg::OnBnClickedOk()
{
	UpdateData(TRUE);	
	CDialogEx::OnOK();
}