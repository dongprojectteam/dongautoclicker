#pragma once
#include "afxwin.h"


// CSequenceDlg 대화 상자입니다.

class CSequenceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSequenceDlg)

public:
	CSequenceDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSequenceDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_SEQUENCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

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
