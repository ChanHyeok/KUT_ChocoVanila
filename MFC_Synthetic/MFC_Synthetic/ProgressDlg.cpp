// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"
#include "MFC_SyntheticDlg.h"

const int PROGRESS_TIMER = 1;

// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialogEx)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProgressDlg::IDD, pParent)
{
	m_pParentWnd = pParent; // ������ �κп� �θ��ڵ��� ����
}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressCtrl);
	DDX_Control(pDX, IDC_MESSAGE, m_StaticMessage);
	DDX_Control(pDX, IDMYOK, m_ButtonOK);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDMYOK, &CProgressDlg::OnBnClickedMyok)
	ON_BN_CLICKED(IDMYCANCEL, &CProgressDlg::OnBnClickedMycancel)
END_MESSAGE_MAP()


// CProgressDlg message handlers

BOOL CProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ButtonOK.EnableWindow(false);
	isBackgroundSaved = false;
	frame = Mat(ROWS, COLS, CV_8UC3); // Mat(height, width, channel)
	bg = Mat(ROWS, COLS, CV_8UC3);
	bg_gray = Mat(ROWS, COLS, CV_8UC1);
	count = 0;

	vc_Source.open(videoFilePath);
	totalFrame = (int)vc_Source.get(CV_CAP_PROP_FRAME_COUNT);
	vc_Source.set(CV_CAP_PROP_POS_MSEC, 0);	// ���� ���������� �ʱ�ȭ

	if (totalFrame < FRAMES_FOR_MAKE_BACKGROUND){
		FRAMES_FOR_MAKE_BACKGROUND = totalFrame;
	}
	m_ProgressCtrl.SetRange(0, FRAMES_FOR_MAKE_BACKGROUND - 1);

	//ù ������
	vc_Source.read(bg);
	cvtColor(bg, bg_gray, CV_RGB2GRAY);

	SetTimer(PROGRESS_TIMER, 0, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CProgressDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	CDialogEx::OnCancel();
	vc_Source = NULL;
	vc_Source.release();
	frame = NULL;
	bg = NULL;
	frame.release();
	bg.release();

	//���������� ����� ������� �ʾ��� ��� ���� ����
	if (!isBackgroundSaved){
		((CMFC_SyntheticDlg *)GetParent())->OnCancel();
	}
}


void CProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CDialogEx::OnTimer(nIDEvent);
	string text;
	switch (nIDEvent) {
	case PROGRESS_TIMER:
		text = "("; text.append(to_string(count)).append("/").append(to_string(FRAMES_FOR_MAKE_BACKGROUND - 1)).append(")...��� ���� ��");
		m_StaticMessage.SetWindowTextA(text.c_str());
		if (count < FRAMES_FOR_MAKE_BACKGROUND - 1){
			vc_Source.read(frame); //get single frame
			cvtColor(frame, frame, CV_RGB2GRAY);
			temporalMedianBG(frame, bg_gray);
			m_ProgressCtrl.OffsetPos(1);
			count++;
		}
		else{
			KillTimer(PROGRESS_TIMER);
			if (imwrite(getBackgroundFilePath(fileNameNoExtension), bg_gray)){
				m_StaticMessage.SetWindowTextA(_T("��� ���� ����!"));
				m_ButtonOK.EnableWindow(true);
				printf("Background Init Completed\n");
				isBackgroundSaved = true;
			}
			else{
				m_StaticMessage.SetWindowTextA(_T("��� ���� ����"));
				m_ButtonOK.EnableWindow(true);
				printf("Background Init Failed!!\n");
			}
		}
		break;
	default:
		break;
	}
}


void CProgressDlg::OnBnClickedMyok()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


void CProgressDlg::OnBnClickedMycancel()
{
	// TODO: Add your control notification handler code here
}


BOOL CProgressDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	//���� �� esc Ű ����
	if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}
