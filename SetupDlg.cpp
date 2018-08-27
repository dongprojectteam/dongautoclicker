// SetupDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DongAutoClicker.h"
#include "SetupDlg.h"
#include "afxdialogex.h"


// CSetupDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSetupDlg, CDialogEx)

CSetupDlg::CSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetupDlg::IDD, pParent)
{

}

CSetupDlg::~CSetupDlg()
{
}

void CSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetupDlg, CDialogEx)
END_MESSAGE_MAP()


// CSetupDlg 메시지 처리기입니다.
