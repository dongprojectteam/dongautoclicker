// SetupDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "DongAutoClicker.h"
#include "SetupDlg.h"
#include "afxdialogex.h"


// CSetupDlg ��ȭ �����Դϴ�.

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


// CSetupDlg �޽��� ó�����Դϴ�.
