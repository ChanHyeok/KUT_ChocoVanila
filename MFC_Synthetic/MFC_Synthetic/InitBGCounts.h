/*
	������ �ε��ؼ� ����� ���� ������ ����, ����� ������ų ������ ������ �Է¹޴� ��ȭ���ڸ� ������ �����Դϴ�.
	��ȭ���ڸ� ���� OK, Cancel ������, �� ���� EditText�� �ֽ��ϴ�.
	����� ���� ���� �������� ������ �޴� ���� ����Ʈ�� 800�� �����Ǿ� �ֽ��ϴ�.
	����� ������Ʈ�� �ֱ⸦ ���ϴ� �������� ������ ����Ʈ�� 1000�� �����Ǿ� �ֽ��ϴ�.
*/

#pragma once
#include "afxwin.h"
// CInitBGCounts dialog

class CInitBGCounts : public CDialogEx
{
	DECLARE_DYNAMIC(CInitBGCounts)

public:
	CInitBGCounts(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInitBGCounts();

// Dialog Data
	enum { IDD = IDD_MFC_BG_COUNT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	int BGMAKINGCOUNTS;
	int BGUPDATECOUNTS;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();

	int getBGMAKINGCOUNTS();
	int getBGUPDATECOUNTS();
	CEdit CEditBGMakeCounts;
	CEdit CEditBGUpdateCounts;


};
