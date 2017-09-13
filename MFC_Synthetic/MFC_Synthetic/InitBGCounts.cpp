// InitBGCounts.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "InitBGCounts.h"
#include "afxdialogex.h"


// CInitBGCounts dialog

IMPLEMENT_DYNAMIC(CInitBGCounts, CDialogEx)

CInitBGCounts::CInitBGCounts(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInitBGCounts::IDD, pParent)
{
	m_pParentWnd = pParent; // ������ �κп� �θ��ڵ��� ����
}

CInitBGCounts::~CInitBGCounts()
{
}

void CInitBGCounts::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, CEditBGMakeCounts);
	DDX_Control(pDX, IDC_EDIT2, CEditBGUpdateCounts);
}


BEGIN_MESSAGE_MAP(CInitBGCounts, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInitBGCounts::OnBnClickedOk)
END_MESSAGE_MAP()


// CInitBGCounts message handlers
BOOL CInitBGCounts::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CInitBGCounts::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	//���� �� esc Ű ����
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CInitBGCounts::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnOK();
}


void CInitBGCounts::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class


	CDialogEx::OnCancel();
}



void CInitBGCounts::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString temp1, temp2;
	CEditBGMakeCounts.GetWindowTextA(temp1);
	CEditBGUpdateCounts.GetWindowTextA(temp2);

	// ��� ����Ʈ �� �����κ�
	BGMAKINGCOUNTS = atoi(temp1);
	BGUPDATECOUNTS = atoi(temp2);
	CDialogEx::OnOK();
}
