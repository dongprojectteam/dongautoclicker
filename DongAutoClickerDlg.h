
// DongAutoClickerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "afxext.h"

#include "MouseInformation.h"
#include <vector>


// CDongAutoClickerDlg 대화 상자
class CDongAutoClickerDlg : public CDialogEx
{
// 생성입니다.
public:
	CDongAutoClickerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DONGAUTOCLICKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	
	std::vector< std::pair<CString, DWORD> > m_vPresets;
	CWinThread* m_pWinThread;
	BOOL m_bCapturing;
	BOOL m_bRecording;
	

	void CheckThreadTerminate();
	void SetupHotKey();
	void LoadMouseInformationFromFile();
	void SaveToFile(BOOL bSequence=FALSE);
	BOOL CheckBeforePressButton();
	void AddNewSequence(BOOL bDelayOnly=FALSE, BOOL bRightClick=FALSE);
	void AddSequenceColumn(UINT unIndex, UINT unSubItem, char* szText);
	void AddSequenceOneLine(UINT unIndex, CMouseInformation mi);
	void AddSequenceAllWithVector();

public:
	std::vector< CMouseInformation > m_vRecording;
	std::vector< CMouseInformation > m_vSequence;

	DWORD m_dwInterval;
	DWORD m_dwIntervalSec;
	DWORD m_dwIntervalMilliSec;
	DWORD m_dwFixPosX;
	DWORD m_dwFixPosY;
	DWORD m_dwCapturePosX;
	DWORD m_dwCapturePosY;

	DWORD m_dwRecordingDelay;
	DWORD m_dwSequenceInterval;
	DWORD m_dwSequenceRepeat;

	INT m_nMousePosSetup;
	INT m_nSaveType;
	
	UINT m_unButtonOption;
	UINT m_unClickOption;
	UINT m_dwRepeatInterval;

	CString m_strRecording;
	CString m_strFilePath;

	CFile m_fileRecording;

	
	CComboBox m_ctlPresetInterval;
	CSpinButtonCtrl m_ctlSpinIntervalSec;
	CSpinButtonCtrl m_ctlSpinIntervalMilliSec;
	CButton m_btnTarget;
	CButton m_btnRecording;
	CButton m_btnStart;
	CButton m_btnPause;
	CButton m_btnSequence;
	CButton m_ctlRepeatCheck;
	CComboBox m_ctlStartHotKey;
	CComboBox m_ctlStopHotKey;
	CComboBox m_ctlButtonOption;
	CComboBox m_ctlClickOption;
	CListCtrl m_ctlSequenceListBox;
	

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();	
	afx_msg void OnBnClickedButtonSetup();
	afx_msg void OnEnChangeEditIntervalMs();
	afx_msg void OnEnChangeEditIntervalS();
	afx_msg void OnCbnSelchangeComboIntervalPreset();
	afx_msg void OnBnClickedButtonSetFixPosition();	
	afx_msg void OnBnClickedRadioMousePosition();
	afx_msg void OnBnClickedRadioMousePosition2();
	afx_msg void OnBnClickedRadioMousePosition3();
	afx_msg void OnBnClickedRadioMousePosition4();
	afx_msg void OnCbnSelchangeComboButton();
	afx_msg void OnCbnSelchangeComboClick();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnCbnSelchangeComboHotkey();
	afx_msg void OnCbnSelchangeComboHotkey2();
	afx_msg void OnDeltaposSpinIntervalS(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinIntervalMs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonRecording();
	afx_msg void OnBnClickedButtonBrowse();	
	afx_msg void OnBnClickedButtonSequence();
	afx_msg void OnBnClickedButtonSequenceIdle();
	afx_msg void OnBnClickedButtonSequenceModify();
	afx_msg void OnBnClickedButtonSequenceDelete();
	afx_msg void OnBnClickedCheckRepeat();
	afx_msg void OnBnClickedButtonSequenceDeleteAll();
	afx_msg void OnBnClickedButtonSequenceMoveUp();
	afx_msg void OnBnClickedButtonSequenceMoveDown();
	
};
