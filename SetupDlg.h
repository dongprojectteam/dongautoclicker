#pragma once


// CSetupDlg ��ȭ �����Դϴ�.

class CSetupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetupDlg)

public:
	CSetupDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CSetupDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};
