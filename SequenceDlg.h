#pragma once
#include "afxwin.h"


// CSequenceDlg ��ȭ �����Դϴ�.

class CSequenceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSequenceDlg)

public:
	CSequenceDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CSequenceDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_SEQUENCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	DWORD m_dwPositionX;
	DWORD m_dwPositionY;
	DWORD m_dwDelay;
	DWORD m_dwRepeat;
	CButton m_ctlDelayOnly;
	BOOL m_bDelayOnly;
	INT m_nButtonPosition;

	afx_msg void OnBnClickedOk();
};
