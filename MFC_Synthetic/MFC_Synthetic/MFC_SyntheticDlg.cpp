// MFC_SyntheticDlg.cpp : implementation file
#include <crtdbg.h>

#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include "SplashScreenEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// �޸� ������ �����ϴ� Ű���� (http://codes.xenotech.net/38)
// �����ϱ� ���� ����� ���� ���� ��, ����� �α׸� ���� ��
// #include <crtdbg.h> ���� ���� _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 

/*** ���� ����� ***/
#define LOGO_TIMER 0
#define VIDEO_TIMER 1
#define SYN_RESULT_TIMER 2
#define BIN_VIDEO_TIMER 3
#define MAX_STR_BUFFER_SIZE  128 // ���ڿ� ��¿� �� ���� ����
const int FRAMECOUNT_FOR_MAKE_BACKGROUND = 500; // ����� ����� ���� �ʿ��� ������ī��Ʈ

/***  ��������  ***/
segment *m_segmentArray;
Queue segment_queue; // C++ STL�� queue Ű����� ��ġ�� ������ ������ ������
int videoStartMsec, segmentCount, fps; // ���� millisecond, ���׸�Ʈ ī���ú���, �ʴ� �����Ӽ�
int radioChoice, preRadioChoice;	//���� ��ư ���� ��� ���� ����. 0 - ��������, 1 - �ռ�����, 2 - ��������
boolean isPlayBtnClicked, isPauseBtnClicked;
Mat background, background_gray; // ��� �����Ӱ� ���� ������

unsigned int COLS, ROWS;

// background �������� ����
// synthesis �߿� ������ �������� ������� ���װ� �߻��Ͽ� �����ϸ鼭 m_background�� ������

// File ����
FILE *fp; // frameInfo�� �ۼ��� File Pointer
std::string video_filename(""); // �Է¹��� �������� �̸�
std::string background_filename, txt_filename; // ��� ���� �̸��� txt ���� �̸�

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFC_SyntheticDlg dialog



CMFC_SyntheticDlg::CMFC_SyntheticDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CMFC_SyntheticDlg::IDD, pParent)
, mRadioPlay(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CMFC_SyntheticDlg::~CMFC_SyntheticDlg()
{
	background.release();
	background_gray.release();
	// MFC���� �Ҹ��ڿ����� �޸� ������ �ǹ̰� ����
}


void CMFC_SyntheticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SYN_SLIDER_START_TIME, m_sliderSearchStartTime);
	DDX_Control(pDX, IDC_SYN_SLIDER_END_TIME, m_sliderSearchEndTime);
	DDX_Control(pDX, IDC_SYN_SLIDER_FPS, m_sliderFps);
	DDX_Radio(pDX, IDC_RADIO_PLAY1, mRadioPlay);
	DDX_Control(pDX, IDC_SEG_SLIDER_WMIN, m_SliderWMIN);
	DDX_Control(pDX, IDC_SEG_SLIDER_WMAX, m_SliderWMAX);
	DDX_Control(pDX, IDC_SEG_SLIDER_HMIN, m_SliderHMIN);
	DDX_Control(pDX, IDC_SEG_SLIDER_HMAX, m_SliderHMAX);
}

BEGIN_MESSAGE_MAP(CMFC_SyntheticDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_SEGMENTATION, &CMFC_SyntheticDlg::OnBnClickedBtnSegmentation)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CMFC_SyntheticDlg::OnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_MENU_LOAD, &CMFC_SyntheticDlg::OnBnClickedBtnMenuLoad)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_PLAY1, IDC_RADIO_PLAY3, &CMFC_SyntheticDlg::SetRadioStatus)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CMFC_SyntheticDlg::OnBnClickedBtnPause)
END_MESSAGE_MAP()


// CMFC_SyntheticDlg message handlers

BOOL CMFC_SyntheticDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ShowWindow(SW_SHOWMAXIMIZED);	//��üȭ��
	this->GetWindowRect(m_rectCurHist);	//���̾�α� ũ�⸦ ����

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//*********************************************Layout ����********************************************************
	//(http://gandus.tistory.com/530)

	CWnd *pResultImage = GetDlgItem(IDC_RESULT_IMAGE);

	//Dialog
	//���� dialog ũ�� ����
	int dialogWidth = m_rectCurHist.right;
	int dialogHeight = m_rectCurHist.bottom - 50;//�۾�ǥ���� ũ�� ����
	int padding = 10;
	SetWindowPos(&wndTop, 0, 0, dialogWidth, dialogHeight, SWP_NOMOVE);//���̾�α� ũ�� ����

	//group box - MENU
	CWnd *pGroupMenu = GetDlgItem(IDC_GROUP_MENU);
	CWnd *pStringFileName = GetDlgItem(IDC_MENU_STRING_FILE_NAME);
	CButton *pButtonLoad = (CButton *)GetDlgItem(IDC_BTN_MENU_LOAD);
	CWnd *pRadioBtn1 = GetDlgItem(IDC_RADIO_PLAY1);
	CWnd *pRadioBtn2 = GetDlgItem(IDC_RADIO_PLAY2);
	CWnd *pRadioBtn3 = GetDlgItem(IDC_RADIO_PLAY3);
	int box_MenuX = padding;
	int box_MenuY = padding;
	int box_MenuWidth = (dialogWidth - 3 * padding)*0.2;
	int box_MenuHeight = ((dialogHeight - 3 * padding)*0.7 - padding)*0.3;

	pGroupMenu->MoveWindow(box_MenuX, box_MenuY, box_MenuWidth, box_MenuHeight, TRUE);
	pStringFileName->MoveWindow(box_MenuX + padding, box_MenuY + 2 * padding, 230, 20, TRUE);
	pButtonLoad->MoveWindow(box_MenuX + box_MenuWidth - padding - 100, box_MenuY + 3 * padding + 20, 100, 20, TRUE);
	pRadioBtn1->MoveWindow(box_MenuX + padding, box_MenuY + 4 * padding + 40, 100, 20, TRUE);
	pRadioBtn3->MoveWindow(box_MenuX + padding + 150, box_MenuY + 4 * padding + 40, 100, 20, TRUE);
	pRadioBtn2->MoveWindow(box_MenuX + padding, box_MenuY + 5 * padding + 60, 100, 20, TRUE);

	//Picture Control
	CButton *pButtonPlay = (CButton *)GetDlgItem(IDC_BTN_PLAY);
	CButton *pButtonPause = (CButton *)GetDlgItem(IDC_BTN_PAUSE);
	int pictureContorlX = 2 * padding + box_MenuWidth;
	int pictureContorlY = padding;
	int pictureContorlWidth = (dialogWidth - 3 * padding) - box_MenuWidth - 15;
	int pictureContorlHeight = (dialogHeight - 3 * padding)*0.7 - 40;
	pResultImage->MoveWindow(pictureContorlX, pictureContorlY, pictureContorlWidth, pictureContorlHeight, TRUE);
	pButtonPlay->MoveWindow(pictureContorlX + pictureContorlWidth*0.5 - padding - 100, pictureContorlY + pictureContorlHeight + 10, 100, 20, TRUE);
	pButtonPause->MoveWindow(pictureContorlX + pictureContorlWidth*0.5 + padding, pictureContorlY + pictureContorlHeight + 10, 100, 20, TRUE);

	//group box - segmetation
	CWnd *pGroupSegmentation = GetDlgItem(IDC_GROUP_SEG);
	CWnd *pStringStartTime = GetDlgItem(IDC_SEG_STRING_VIDEO_START_TIME);
	CWnd *pStringColon = GetDlgItem(IDC_SEG_STRING_COLON);
	m_pEditBoxStartHour = (CEdit *)GetDlgItem(IDC_SEG_EDITBOX_START_HOUR);
	m_pEditBoxStartMinute = (CEdit *)GetDlgItem(IDC_SEG_EDITBOX_START_MINUTE);
	CWnd *pGroupSegWidth = GetDlgItem(IDC_GROUP_SEG_WIDTH);
	CWnd *pStringWMIN = GetDlgItem(IDC_SEG_STRING_MIN_W);
	CWnd *pStringWMAX = GetDlgItem(IDC_SEG_STRING_MAX_W);
	CWnd *pStringValWMIN = GetDlgItem(IDC_SEG_STRING_VAL_MIN_W);
	CWnd *pStringValWMAX = GetDlgItem(IDC_SEG_STRING_VAL_MAX_W);
	//CWnd *pSegSliderWMIN = GetDlgItem(IDC_SEG_SLIDER_WMIN);
	CWnd *pSegSliderWMAX = GetDlgItem(IDC_SEG_SLIDER_WMAX);
	CWnd *pGroupSegHeight = GetDlgItem(IDC_GROUP_SEG_HEIGHT);
	CWnd *pStringHMIN = GetDlgItem(IDC_SEG_STRING_MIN_H);
	CWnd *pStringHMAX = GetDlgItem(IDC_SEG_STRING_MAX_H);
	CWnd *pStringValHMIN = GetDlgItem(IDC_SEG_STRING_VAL_MIN_H);
	CWnd *pStringValHMAX = GetDlgItem(IDC_SEG_STRING_VAL_MAX_H);
	CWnd *pSegSliderHMIN = GetDlgItem(IDC_SEG_SLIDER_HMIN);
	CWnd *pSegSliderHMAX = GetDlgItem(IDC_SEG_SLIDER_HMAX);
	CButton *pButtonSegmentation = (CButton *)GetDlgItem(IDC_BTN_SEG_SEGMENTATION);
	int box_segmentationX = padding;
	int box_segmentationY = 2 * padding + box_MenuHeight;
	int box_segmentationWidth = box_MenuWidth;
	int box_segmentationHeight = ((dialogHeight - 3 * padding)*0.7 - padding) - box_MenuHeight;
	pGroupSegmentation->MoveWindow(box_segmentationX, box_segmentationY, box_segmentationWidth, box_segmentationHeight, TRUE);
	pStringStartTime->MoveWindow(box_segmentationX + padding, box_segmentationY + 2 * padding, 230, 20, TRUE);
	m_pEditBoxStartHour->MoveWindow(box_segmentationX + padding + box_segmentationWidth*0.5, box_segmentationY + 3 * padding + 20, 20, 20, TRUE);
	pStringColon->MoveWindow(box_segmentationX + padding + 25 + box_segmentationWidth*0.5, box_segmentationY + 3 * padding + 20, 20, 20, TRUE);
	m_pEditBoxStartMinute->MoveWindow(box_segmentationX + padding + 35 + box_segmentationWidth*0.5, box_segmentationY + 3 * padding + 20, 20, 20, TRUE);
	pGroupSegWidth->MoveWindow(box_segmentationX + padding, box_segmentationY + 4 * padding + 40, box_segmentationWidth - 2 * padding, 80, TRUE);
	pStringWMIN->MoveWindow(box_segmentationX + 2 * padding, box_segmentationY + 6 * padding + 40, 40, 20, TRUE);
	m_SliderWMIN.MoveWindow(box_segmentationX + 3 * padding + 40, box_segmentationY + 6 * padding + 40, box_segmentationWidth - 6 * padding - 60, 20, TRUE);
	pStringValWMIN->MoveWindow(box_segmentationX + box_segmentationWidth - 2 * padding - 20, box_segmentationY + 6 * padding + 40, 20, 20, TRUE);
	pStringWMAX->MoveWindow(box_segmentationX + 2 * padding, box_segmentationY + 7 * padding + 60, 40, 20, TRUE);
	m_SliderWMAX.MoveWindow(box_segmentationX + 3 * padding + 40, box_segmentationY + 7 * padding + 60, box_segmentationWidth - 6 * padding - 60, 20, TRUE);
	pStringValWMAX->MoveWindow(box_segmentationX + box_segmentationWidth - 2 * padding - 20, box_segmentationY + 7 * padding + 60, 20, 20, TRUE);
	pGroupSegHeight->MoveWindow(box_segmentationX + padding, box_segmentationY + 5 * padding + 120, box_segmentationWidth - 2 * padding, 80, TRUE);
	pStringHMIN->MoveWindow(box_segmentationX + 2 * padding, box_segmentationY + 7 * padding + 120, 40, 20, TRUE);
	m_SliderHMIN.MoveWindow(box_segmentationX + 3 * padding + 40, box_segmentationY + 7 * padding + 120, box_segmentationWidth - 6 * padding - 60, 20, TRUE);
	pStringValHMIN->MoveWindow(box_segmentationX + box_segmentationWidth - 2 * padding - 20, box_segmentationY + 7 * padding + 120, 20, 20, TRUE);
	pStringHMAX->MoveWindow(box_segmentationX + 2 * padding, box_segmentationY + 8 * padding + 140, 40, 20, TRUE);
	m_SliderHMAX.MoveWindow(box_segmentationX + 3 * padding + 40, box_segmentationY + 8 * padding + 140, box_segmentationWidth - 6 * padding - 60, 20, TRUE);
	pStringValHMAX->MoveWindow(box_segmentationX + box_segmentationWidth - 2 * padding - 20, box_segmentationY + 8 * padding + 140, 20, 20, TRUE);
	pButtonSegmentation->MoveWindow(box_segmentationX + box_segmentationWidth - padding - 100, box_segmentationY + box_segmentationHeight - 30, 100, 20, TRUE);


	//group box - Play Settings
	CWnd *pGroupSynthetic = GetDlgItem(IDC_GROUP_PLAY_SETTINGS);
	CWnd *pStringSearchStartTime = GetDlgItem(IDC_STRING_SEARCH_START_TIME);
	CWnd *pStringSearchEndTime = GetDlgItem(IDC_STRING_SEARCH_END_TIME);
	CWnd *pStringFps = GetDlgItem(IDC_STRING_FPS);
	CWnd *pStringSearchStartTimeSlider = GetDlgItem(IDC_STRING_SEARCH_START_TIME_SLIDER);
	CWnd *pStringSearchEndTimeSlider = GetDlgItem(IDC_STRING_SEARCH_END_TIME_SLIDER);
	CWnd *pStringFpsSlider = GetDlgItem(IDC_STRING_FPS_SLIDER);

	int box_syntheticX = padding;
	int box_syntheticY = box_segmentationY + box_segmentationHeight + padding;
	int box_syntheticWidth = dialogWidth - 3 * padding;
	int box_syntheticHeight = (dialogHeight - 3 * padding)*0.3 - 40;
	pGroupSynthetic->MoveWindow(box_syntheticX, box_syntheticY, box_syntheticWidth, box_syntheticHeight, TRUE);
	pStringSearchStartTime->MoveWindow(box_syntheticX + padding, box_syntheticY + box_syntheticHeight*0.3, 100, 20, TRUE);
	m_sliderSearchStartTime.MoveWindow(box_syntheticX + padding, box_syntheticY + box_syntheticHeight*0.3 + 20 + padding, 140, 20, TRUE);
	pStringSearchStartTimeSlider->MoveWindow(box_syntheticX + padding + 40, box_syntheticY + box_syntheticHeight*0.3 + 40 + padding * 2, 140, 20, TRUE);

	pStringSearchEndTime->MoveWindow(box_syntheticX + padding + 150, box_syntheticY + box_syntheticHeight*0.3, 100, 20, TRUE);
	m_sliderSearchEndTime.MoveWindow(box_syntheticX + padding + 150, box_syntheticY + box_syntheticHeight*0.3 + 20 + padding, 140, 20, TRUE);
	pStringSearchEndTimeSlider->MoveWindow(box_syntheticX + padding + 40 + 150, box_syntheticY + box_syntheticHeight*0.3 + 40 + padding * 2, 140, 20, TRUE);

	pStringFps->MoveWindow(box_syntheticX + padding + 300, box_syntheticY + box_syntheticHeight*0.3, 100, 20, TRUE);
	m_sliderFps.MoveWindow(box_syntheticX + padding + 300, box_syntheticY + box_syntheticHeight*0.3 + 20 + padding, 140, 20, TRUE);
	pStringFpsSlider->MoveWindow(box_syntheticX + padding + 60 + 300, box_syntheticY + box_syntheticHeight*0.3 + 40 + padding * 2, 30, 20, TRUE);


	/*
	slider m_sliderSearchStartTime, m_sliderSearchEndTime, m_sliderFps, segment Ʈ���� ����
	*/
	m_sliderSearchStartTime.SetRange(0, 500);
	m_sliderSearchEndTime.SetRange(0, 500);
	m_sliderFps.SetRange(0, 100);

	// ������ ũ�⿡ �°� slider�� ��Ÿ�� ���� ������ ������ �ʿ䰡 ����
	// Default Range Value At Slider
	m_SliderWMIN.SetRange(0, 1000);
	m_SliderWMAX.SetRange(0, 1000);
	m_SliderHMIN.SetRange(0, 1000);
	m_SliderHMAX.SetRange(0, 1000);

	//����� ���� ���� �ҷ���
	loadFile();

	isPlayBtnClicked = false;
	isPauseBtnClicked = true;
	CheckRadioButton(IDC_RADIO_PLAY1, IDC_RADIO_PLAY3, IDC_RADIO_PLAY1);//���� ��ư �ʱ�ȭ
	radioChoice = 0; preRadioChoice = 0;	//���� ��ư�� default�� �� ó�� ��ư��
	//edit box default
	m_pEditBoxStartHour->SetWindowTextA("0");
	m_pEditBoxStartMinute->SetWindowTextA("0");
	//slider default
	SetDlgItemText(IDC_STRING_SEARCH_START_TIME_SLIDER, _T("00 : 00 : 00"));
	SetDlgItemText(IDC_STRING_SEARCH_END_TIME_SLIDER, _T("00 : 00 : 00"));
	SetDlgItemText(IDC_STRING_FPS_SLIDER, to_string(fps).c_str());
	m_sliderSearchStartTime.SetPos(0);
	m_sliderSearchEndTime.SetPos(0);
	m_sliderFps.SetPos(fps);
	SetDlgItemText(IDC_SEG_STRING_VAL_MIN_W, _T("0"));
	SetDlgItemText(IDC_SEG_STRING_VAL_MAX_W, _T("0"));
	SetDlgItemText(IDC_SEG_STRING_VAL_MIN_H, _T("0"));
	SetDlgItemText(IDC_SEG_STRING_VAL_MAX_H, _T("0"));
	m_SliderWMIN.SetPos(0);
	m_SliderWMAX.SetPos(0);
	m_SliderHMIN.SetPos(0);
	m_SliderHMAX.SetPos(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFC_SyntheticDlg::loadFile(){	
	//���� ���̾�α� ȣ���ؼ� segmentation �� ���� ����	
	char szFilter[] = "Video (*.avi, *.MP4) | *.avi;*.mp4; | All Files(*.*)|*.*||";	//�˻� �ɼ�
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, AfxGetMainWnd());	//���� ���̾�α� ����
	dlg.DoModal();	//���̾�α� ���

	//���̾�α� ���� �� �ε� â ���
	//�ε��� hide�ϴ� ���� �޸�������. �׶� �׶� ����� ��
	CSplashScreenEx *pSplash; //�ε�â
	pSplash = new CSplashScreenEx();
	pSplash->Create(this, "Loading", 0, CSS_FADE | CSS_CENTERAPP | CSS_SHADOW);
	pSplash->SetBitmap(IDB_LOADING, 0, 0, 0);
	pSplash->SetTextFont("Arial", 140, CSS_TEXT_BOLD);
	pSplash->SetTextRect(CRect(148, 38, 228, 70));
	pSplash->SetTextColor(RGB(0, 0, 0));
	pSplash->Show();
	pSplash->SetText("Loading");

	// Path�� �޾ƿͼ� filename���� ��� ������(������� �̸��� ������ �� ���)
	CString cstrImgPath = dlg.GetPathName();
	String temp = "File Name : ";
	video_filename = "";
	video_filename = getFileName(cstrImgPath, '\\');
	txt_filename = getTextFileName(video_filename); // frameinfo txt���� �缳��

	CWnd *pStringFileName = GetDlgItem(IDC_MENU_STRING_FILE_NAME);
	
	char *cstr = new char[temp.length() + 1];
	strcpy(cstr, temp.c_str());
	strcat(cstr, video_filename.c_str());
	pStringFileName->SetWindowTextA(cstr);
	capture.open((string)cstrImgPath);
	capture_for_background.open((string)cstrImgPath);

	if (!capture.isOpened()) { //����ó��. �ش��̸��� ������ ���� ���
		perror("No Such File!\n");
		::SendMessage(GetSafeHwnd(), WM_CLOSE, NULL, NULL);	//���̾� �α� ����
	}
	
	// delete[]cstr;

	// To Do :: cstr �޸� ����
	// ������ ��� ������ ��

	// �޾ƿ� ������ ������ :: ����, ���α��� �޾ƿ���
	COLS = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);	//���� ����
	ROWS = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);	//���� ����
	fps = capture.get(CV_CAP_PROP_FPS);

	isPlayBtnClicked = false;
	isPauseBtnClicked = true;
	CheckRadioButton(IDC_RADIO_PLAY1, IDC_RADIO_PLAY3, IDC_RADIO_PLAY1);//���� ��ư �ʱ�ȭ
	radioChoice = 0; preRadioChoice = 0;	//���� ��ư�� default�� �� ó�� ��ư��

	// edit box�� slider �⺻ �� �ҷ�����
	loadValueOfSlider(COLS, ROWS, 0, 0); // �ϴ� startTime�� endTime�� 0���� �س���

	Mat frame(ROWS, COLS, CV_8UC3); // Mat(height, width, channel)
	background = Mat(ROWS, COLS, CV_8UC3);
	background_gray = Mat(ROWS, COLS, CV_8UC1);

	// To Do :: ���� ������ load �� ��, ����� ���� ���ϋ�
	// �������̶�� ����ڿ��� �˷��ִ� Dialog�� �������ֱ�
	
	// �������κ�
	for (int i = 0; i < FRAMECOUNT_FOR_MAKE_BACKGROUND; i++){
		capture_for_background.read(frame); //get single frame
		BackgroundMaker(frame, background, ROWS * 3, COLS);
	}

	background_filename = getBackgroundFilename(video_filename);
	int background_write_check = imwrite(background_filename, background);
	printf("background Making Complete!!\n");
	//�׷��̽����� ��ȯ
	cvtColor(background, background_gray, CV_RGB2GRAY);
	
	SetTimer(LOGO_TIMER, 1, NULL);

	//������ư - �ռ����� Ȱ��ȭ / ��Ȱ��ȭ
	if (checkSegmentation()){
		GetDlgItem(IDC_RADIO_PLAY2)->EnableWindow(TRUE);
	}
	else{
		GetDlgItem(IDC_RADIO_PLAY2)->EnableWindow(FALSE);
	}


	//�ε�â ����. �޸� ���� �ڵ�
	pSplash->Hide();
}


// ������ �ҷ����鼭 ���� slider�� box�� �ʱ�ȭ�ϴ� �Լ�
void CMFC_SyntheticDlg::loadValueOfSlider(int captureCols, int captureRows, int startTime, int endTime) {
	//edit box default
	m_pEditBoxStartHour->SetWindowTextA("0");
	m_pEditBoxStartMinute->SetWindowTextA("0");

	// To Do :: startTime, endTime�� �Ű������� �޾ƿͼ� �ʱ�ȭ�����ֱ�
	// stringstream time_string;
	// time_string = timeConvertor(startTime);

	// time slider default
	SetDlgItemText(IDC_STRING_SEARCH_START_TIME_SLIDER, _T("00 : 00 : 00"));
	SetDlgItemText(IDC_STRING_SEARCH_END_TIME_SLIDER, _T("00 : 00 : 00"));
	SetDlgItemText(IDC_STRING_FPS_SLIDER, to_string(fps).c_str());
	m_sliderSearchStartTime.SetPos(0);
	m_sliderSearchEndTime.SetPos(0);
	m_sliderFps.SetPos(fps);

	SetDlgItemText(IDC_SEG_STRING_VAL_MIN_W, _T("0"));
	SetDlgItemText(IDC_SEG_STRING_VAL_MAX_W, _T("0"));
	SetDlgItemText(IDC_SEG_STRING_VAL_MIN_H, _T("0"));
	SetDlgItemText(IDC_SEG_STRING_VAL_MAX_H, _T("0"));
	m_SliderWMIN.SetPos(0);
	m_SliderWMAX.SetPos(0);
	m_SliderHMIN.SetPos(0);
	m_SliderHMAX.SetPos(0);

	COLS = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);	//���� ����
	ROWS = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);	//���� ����
	Mat frame(ROWS, COLS, CV_8UC3); // Mat(height, width, channel)
	background = Mat(ROWS, COLS, CV_8UC3);
	background_gray = Mat(ROWS, COLS, CV_8UC1);

	// �������κ�
	for (int i = 0; i < FRAMECOUNT_FOR_MAKE_BACKGROUND; i++){
		capture_for_background.read(frame); //get single frame
		BackgroundMaker(frame, background, ROWS * 3, COLS);
	}
	background_filename = RESULT_BACKGROUND_FILENAME;
	background_filename.append(video_filename).append(".jpg");
	int background_write_check = imwrite(background_filename, background);
	printf("background Making Complete!!\n");
	//�׷��̽����� ��ȯ
	cvtColor(background, background_gray, CV_RGB2GRAY);

	// detection slider range
	m_SliderWMIN.SetRange(0, captureCols);
	m_SliderWMAX.SetRange(0, captureCols);
	m_SliderHMIN.SetRange(0, captureRows);
	m_SliderHMAX.SetRange(0, captureRows);	
	
	// detection slider text
	SetDlgItemText(IDC_SEG_STRING_VAL_MIN_W, _T(to_string(captureCols / 5).c_str()));
	SetDlgItemText(IDC_SEG_STRING_VAL_MAX_W, _T(to_string(captureCols / 2).c_str()));
	SetDlgItemText(IDC_SEG_STRING_VAL_MIN_H, _T(to_string(captureRows / 5).c_str()));
	SetDlgItemText(IDC_SEG_STRING_VAL_MAX_H, _T(to_string(captureRows / 2).c_str()));
	
	// detection slider default position
	m_SliderWMIN.SetPos(captureCols / 5);
	m_SliderWMAX.SetPos(captureCols / 2);
	m_SliderHMIN.SetPos(captureRows / 5);
	m_SliderHMAX.SetPos(captureRows / 2);
}

void CMFC_SyntheticDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}

	// ���α׷��� �ߴ�(x��ư)���� ��
	else if (nID == SC_CLOSE) {
		if (MessageBox("���α׷��� �����Ͻðڽ��ϱ�??", "S/W Exit", MB_YESNO) == IDYES) {
			// ����� �̺�Ʈ
			AfxGetMainWnd()->PostMessage(WM_CLOSE);
		}
		else {
			// ��ҽ� �̺�Ʈ
		}
	}

	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// MFC���� ����(x)��ư�� ���� ��, OnClose()->OnCancel()->OnDestroy()������ ȣ��Ǿ� ����
// OnClose(), OnDestroy()�� �̿��� �ʿ䰡 ��� ������

// ���� ���� �޸� ���� �� ���Ῥ��
void CMFC_SyntheticDlg::OnCancel() {
	printf("OnCancel\n");
	// cpp���� �� ���������� �޸� ����
	background.release(); 
	background_gray.release();
	video_filename.clear(); background_filename.clear(); txt_filename.clear();

	delete[] m_segmentArray;
	
	// CMFC_SyntheticDlg Ŭ������ ��������� �޸� ����
	capture.release();
	capture_for_background.release();

//  layout���� �����Ǵ� �κ��� ������ ��� ������ ��
//	free(&m_rectCurHist);
//	free(m_pEditBoxStartHour);  free(m_pEditBoxStartMinute);

	// To Do :: �����ִ� �ؽ�Ʈ ���� ��� ����

	PostQuitMessage(0); 
}
void CMFC_SyntheticDlg::OnDestroy() {
	printf("OnDestroy\n");
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFC_SyntheticDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFC_SyntheticDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ���÷��� �Լ�
void CMFC_SyntheticDlg::DisplayImage(int IDC_PICTURE_TARGET, Mat targetMat, int TIMER_ID){
	if (targetMat.empty()) {	//����ó��. �������� ����
		perror("Empty Frame");
		KillTimer(TIMER_ID);
		return;
	}
	BITMAPINFO bitmapInfo;
	memset(&bitmapInfo, 0, sizeof(bitmapInfo));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biWidth = targetMat.cols;
	bitmapInfo.bmiHeader.biHeight = -targetMat.rows;

	// IplImage �ּҼ���
	IplImage *tempImage;
	IplImage tempImageAddress;
	tempImage = &tempImageAddress;

	if (targetMat.channels() == 1)
	{
		tempImage = cvCreateImage(targetMat.size(), IPL_DEPTH_8U, 3);
		cvCvtColor(&IplImage(targetMat), tempImage, CV_GRAY2BGR);
	}
	else if (targetMat.channels() == 3)
	{
		tempImage = cvCloneImage(&IplImage(targetMat));
	}

	bitmapInfo.bmiHeader.biBitCount = tempImage->depth * tempImage->nChannels;

	CDC* pDC;
	pDC = GetDlgItem(IDC_RESULT_IMAGE)->GetDC();
	CRect rect;
	GetDlgItem(IDC_RESULT_IMAGE)->GetClientRect(&rect);

	//http://blog.naver.com/PostView.nhn?blogId=hayandoud&logNo=220851430885&categoryNo=0&parentCategoryNo=0&viewDate=&currentPage=1&postListTopCurrentPage=1&from=postView
	pDC->SetStretchBltMode(COLORONCOLOR);

	// ���� �������� ����Ͽ� Picture control�� ���
	float fImageRatio = float(tempImage->width) / float(tempImage->height);
	float fRectRatio = float(rect.right) / float(rect.bottom);
	float fScaleFactor;
	if (fImageRatio < fRectRatio){
		fScaleFactor = float(rect.bottom) / float(tempImage->height);	//TRACE("%f",fScaleFactor);
		int rightWithRatio = tempImage->width * fScaleFactor;
		float halfOfDif = ((float)rect.right - (float)rightWithRatio) / (float)2;
		rect.left = halfOfDif;
		rect.right = rightWithRatio;
	}
	else{
		fScaleFactor = float(rect.right) / float(tempImage->width);	//TRACE("%f",fScaleFactor);
		int bottomWithRatio = tempImage->height * fScaleFactor;
		float halfOfDif = ((float)rect.bottom - (float)bottomWithRatio) / (float)2;
		rect.top = halfOfDif;
		rect.bottom = bottomWithRatio;
	}
	//�̹��� ��� (https://thebook.io/006796/ch03/04/01_01/)
	::StretchDIBits(pDC->GetSafeHdc(),
		rect.left + 5, rect.top + 5, rect.right - 10, rect.bottom - 10,
		0, 0, tempImage->width, tempImage->height,
		tempImage->imageData, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	cvReleaseImage(&tempImage);
}

//Ÿ�̸�
void CMFC_SyntheticDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
	Mat temp_frame;
	switch (nIDEvent){
	case LOGO_TIMER:	//�ΰ� ���
		if (true){
			string logoFileName = "logo.jpg";
			Mat logo = imread(logoFileName);
			DisplayImage(IDC_RESULT_IMAGE, logo, LOGO_TIMER);
			logo.release();
			KillTimer(LOGO_TIMER);
			KillTimer(VIDEO_TIMER);
			KillTimer(BIN_VIDEO_TIMER);
			KillTimer(SYN_RESULT_TIMER);
		}
		break;

		// ���� ���� ���
	case VIDEO_TIMER:
		printf("$");
		capture.read(temp_frame);
		DisplayImage(IDC_RESULT_IMAGE, temp_frame, VIDEO_TIMER);
		break;

		// ���� ���� ���
	case BIN_VIDEO_TIMER:
		if (true) {
			Mat img_labels, stats, centroids;
			capture.read(temp_frame);
			//�׷��̽����� ��ȯ
			cvtColor(temp_frame, temp_frame, CV_RGB2GRAY);
			// ���� ����
			temp_frame = ExtractFg(temp_frame, background_gray, ROWS, COLS);
			// frame_g = ExtractForegroundToMOG2(frame_g);

			// ����ȭ
			threshold(temp_frame, temp_frame, 5, 255, CV_THRESH_BINARY);

			// ������ ����
			temp_frame = morphologicalOperation(temp_frame);
			blur(temp_frame, temp_frame, Size(9, 9));
			temp_frame = morphologicalOperation(temp_frame);

			threshold(temp_frame, temp_frame, 5, 255, CV_THRESH_BINARY);

			int numOfLables = connectedComponentsWithStats(temp_frame, img_labels,
				stats, centroids, 8, CV_32S);

			cvtColor(temp_frame, temp_frame, CV_GRAY2BGR);

			//�󺧸� �� �̹����� ���� ���簢������ �ѷ��α� 
			for (int j = 1; j < numOfLables; j++) {
				int area = stats.at<int>(j, CC_STAT_AREA);
				int left = stats.at<int>(j, CC_STAT_LEFT);
				int top = stats.at<int>(j, CC_STAT_TOP);
				int width = stats.at<int>(j, CC_STAT_WIDTH);
				int height = stats.at<int>(j, CC_STAT_HEIGHT);
				if (labelSizeFiltering(width, height
					, m_SliderWMIN.GetPos(), m_SliderWMAX.GetPos(), m_SliderHMIN.GetPos(), m_SliderHMAX.GetPos())) {
					rectangle(temp_frame, Point(left, top), Point(left + width, top + height),
						Scalar(0, 0, 255), 1);
				}
			}

			DisplayImage(IDC_RESULT_IMAGE, temp_frame, BIN_VIDEO_TIMER);
		}
		break;

	case SYN_RESULT_TIMER:
		printf("#");
		background_filename = getBackgroundFilename(video_filename);
		Mat background_loadedFromFile = imread(background_filename);//�ռ� ������ ����� �� ������ �� ������. �����ռ� ���� ��ư Ŭ�� �� �ڵ����� ���Ϸκ��� �ε� ��
		
		// �ҷ��� ����� �̿��Ͽ� �ռ��� ����
		Mat syntheticResult = getSyntheticFrame(background_loadedFromFile);
		DisplayImage(IDC_RESULT_IMAGE, syntheticResult, SYN_RESULT_TIMER);
		syntheticResult.release();
		background_loadedFromFile.release();
		break;
	}
	temp_frame.release();
}

// segmentation Button�� ������ ���� ����
// �ÿ� ���� �Է¹޾� segmentation�� ������
void CMFC_SyntheticDlg::OnBnClickedBtnSegmentation()
{
	CString str_startHour, str_startMinute;
	//Edit Box�κ��� ���� �ð��� ���� �о��
	m_pEditBoxStartHour->GetWindowTextA(str_startHour);
	m_pEditBoxStartMinute->GetWindowTextA(str_startMinute);

	// Edit box�� ���� �Է�, �Ǵ� ������ �Է� �� ����ó��
	if (segmentationTimeInputException(str_startHour, str_startMinute)){
		//�ε� â ���
		CSplashScreenEx *pSplash; //�ε�â
		pSplash = new CSplashScreenEx();
		pSplash->Create(this, "Loading", 0, CSS_FADE | CSS_CENTERAPP | CSS_SHADOW);
		pSplash->SetBitmap(IDB_LOADING, 0, 0, 0);
		pSplash->SetTextFont("Arial", 110, CSS_TEXT_BOLD);
		pSplash->SetTextRect(CRect(148, 38, 228, 70));
		pSplash->SetTextColor(RGB(0, 0, 0));
		pSplash->Show();
		pSplash->SetText("Pls wait...");
		segmentationOperator(&capture, atoi(str_startHour), atoi(str_startMinute)
			, m_SliderWMIN.GetPos(), m_SliderWMAX.GetPos(), m_SliderHMIN.GetPos(), m_SliderHMAX.GetPos());	//Object Segmentation
		
		//������ư - �ռ����� Ȱ��ȭ / ��Ȱ��ȭ
		if (checkSegmentation()){
			GetDlgItem(IDC_RADIO_PLAY2)->EnableWindow(TRUE);
		}
		else{
			GetDlgItem(IDC_RADIO_PLAY2)->EnableWindow(FALSE);
		}

		pSplash->Hide();
	}
	else {	// ���� �� �Է½� ����ó��
	}
}

// segmentation ��� ����, ��ü ���� �� ���Ϸ� ����
void segmentationOperator(VideoCapture* vc_Source, int videoStartHour, int videoStartMin, int WMIN, int WMAX, int HMIN, int HMAX){
	videoStartMsec = (videoStartHour * 60 + videoStartMin) * 60 * 1000;

	unsigned int COLS = (int)vc_Source->get(CV_CAP_PROP_FRAME_WIDTH);	//���� ����
	unsigned int ROWS = (int)vc_Source->get(CV_CAP_PROP_FRAME_HEIGHT);	//���� ����

	unsigned char* result = (unsigned char*)malloc(sizeof(unsigned char)* ROWS * COLS);

	// humanDetector Vector
	vector<component> humanDetectedVector, prevHumanDetectedVector;

	/* Mat */
	Mat frame(ROWS, COLS, CV_8UC3); // Mat(height, width, channel)
	Mat frame_g(ROWS, COLS, CV_8UC1);

	//frame ī���Ϳ� ���� millisecond
	int frameCount = 0;
	unsigned int currentMsec;

	// �� ��ü �������� ������ �� �� �ؽ�Ʈ ���� ����
	fp = fopen(txt_filename.c_str(), "w");	// ������
	fprintf(fp, to_string(videoStartMsec).append("\n").c_str());	//ù�ٿ� ������۽ð� ������

	// vc_source�� ���۽ð� 0���� �ʱ�ȭ	
	vc_Source->set(CV_CAP_PROP_POS_MSEC, 0);
	
	// To Do :: ���׸����̼��� ���߿� �ߴܵǾ��� �� 
	// �� ������������ �ٽ� ���׸����̼��� �����ϰ� ���� �� vc_source�� ó��

	while (1) {
		vc_Source->read(frame); //get single frame
		if (frame.empty()) {	//����ó��. �������� ����
			perror("Empty Frame");
			break;
		}
		//printf("=====%5d ������=====\n", frameCount);
		//�׷��̽����� ��ȯ
		cvtColor(frame, frame_g, CV_RGB2GRAY);

		// To Do :: ���׸����̼� ���� ����� ������ �� �־�� ��

		// ���� ����
		frame_g = ExtractFg(frame_g, background_gray, ROWS, COLS);

		// ����ȭ
		threshold(frame_g, frame_g, 5, 255, CV_THRESH_BINARY);

		// ������ ���� �� �� ó��
		frame_g = morphologicalOperation(frame_g);
		blur(frame_g, frame_g, Size(9, 9));
		frame_g = morphologicalOperation(frame_g);

		threshold(frame_g, frame_g, 5, 255, CV_THRESH_BINARY);


		// MAT������ �󺧸�
		humanDetectedVector.clear();
		humanDetectedVector = connectedComponentsLabelling(frame_g, ROWS, COLS, WMIN, WMAX, HMIN, HMAX);

		// ���� �������� ���� �ð� ��������
		currentMsec = vc_Source->get(CV_CAP_PROP_POS_MSEC);

		// ������ ó���Ͽ� ���Ϸ� �����ϱ�
		humanDetectedVector = humanDetectedProcess(humanDetectedVector, prevHumanDetectedVector,
			frame, frameCount, videoStartMsec, currentMsec, fp);

		// ���� ������ �����͸� ���� �����Ϳ� �����ϱ�
		prevHumanDetectedVector = humanDetectedVector;

		frameCount++;	//increase frame count
	}
	//HWND hWnd = ::FindWindow(NULL, "Dude, Wait");
	//if (hWnd){ ::PostMessage(hWnd, WM_CLOSE, 0, 0); }

	// To Do :: ���׸����̼� �Ϸ� �޼��� �ڽ��� ��Ÿ���鼭 cvtColor ���� �߻�
	// MessageBox(0, "Done!!", "ding-dong", MB_OK);
	// printf("messageBox ����\n");
	// Sleep(2500);

	//�޸� ����
	free(result); 	frame.release(); frame_g.release();

	vector<component>().swap(humanDetectedVector);
	vector<component>().swap(prevHumanDetectedVector);
	printf("���׸����̼ǿ� ����ϴ� ������ �޸� ���� �Ϸ�\n");
	fclose(fp);	// �ؽ�Ʈ ���� �ݱ�
}

// ������ �̸��κ��� ����
string allocatingComponentFilename(vector<component> humanDetectedVector, int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector) {
	string name;
	return name.append(to_string(timeTag)).append("_")
		.append(to_string(currentMsec)).append("_")
		.append(to_string(frameCount)).append("_")
		.append(to_string(indexOfhumanDetectedVector));
}

// ����� ������ ������ ����� ��, true �� ��ĥ �� ����
bool IsComparePrevDetection(vector<component> curr_detected, vector<component> prev_detected, int curr_index, int prev_index) {
	return curr_detected[curr_index].left > prev_detected[prev_index].right
		|| curr_detected[curr_index].right < prev_detected[prev_index].left
		|| curr_detected[curr_index].top > prev_detected[prev_index].bottom
		|| curr_detected[curr_index].bottom < prev_detected[prev_index].top;
}

// humanDetectedVector�� �����̸��� timetag�� �ο�
// �׸��� TXT, JPG���Ͽ� �����ϴ� ���
void componentVectorHandling() {
	// TO DO :: �ڵ��� �ߺ��� ���̱� ���� ����� ��� ���� ������ �����ϱ�(�Ű����� ������)
}

vector<component> humanDetectedProcess(vector<component> humanDetectedVector, vector<component> prevHumanDetectedVector
	, Mat frame, int frameCount, int videoStartMsec, unsigned int currentMsec, FILE *fp) {

	//printf("cur msec : %d\n", currentMsec);
	int prevTimeTag;
	for (int i = 0; i < humanDetectedVector.size(); i++) {
		// TODO : ���� �����ӿ��� ���������Ӱ� ��ġ�� obj�� �ִ��� �Ǵ��Ѵ�. 
		// ���� ������Ʈ�� ����������Ʈ�� �ΰ��� ��ĥ ��� ��� ó���� ���ΰ�?
		if (!prevHumanDetectedVector.empty()) {	//���� �������� ����� ��ü�� ���� ���
			bool findFlag = false;
			for (int j = 0; j < prevHumanDetectedVector.size(); j++) {
				if (!IsComparePrevDetection(humanDetectedVector, prevHumanDetectedVector, i, j)) {	// �� ROI�� ��ĥ ���
					// ���� TimeTag�� �Ҵ�
					prevTimeTag = prevHumanDetectedVector[j].timeTag;
					//printf("%d�� ��ħ\n", prevTimeTag);
					humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, prevTimeTag, currentMsec, frameCount, i);
					humanDetectedVector[i].timeTag = prevTimeTag;
					saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec, video_filename);
					saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
					findFlag = true;
					//break;
				}
			}

			if (findFlag == false) { // �� ��ü�� ����
				humanDetectedVector[i].timeTag = currentMsec;
				humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, currentMsec, currentMsec, frameCount, i);
				saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec, video_filename);
				saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);

				//printf("���ο� ��ü : %s\n", humanDetectedVector[i].fileName);
			}
		}
		else {	// ù �����̰ų� ���� �����ӿ� ����� ��ü�� ���� ���
			// ���ο� �̸� �Ҵ�
			humanDetectedVector[i].timeTag = currentMsec;
			humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, currentMsec, currentMsec, frameCount, i);
			saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec, video_filename);
			saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
			// �� ���ٿ� ���� �ڵ��ߺ��� �� ����, ���� �߰� �� ���� ���ɼ��� ����(�˻� ��� ������ ��)
			//printf("***���������� ���� ��ü ����\n���ο� ��ü : %s\n", humanDetectedVector[i].fileName);
		}
	}
	return humanDetectedVector;
}

// �ռ��� �������� �������� ����
Mat getSyntheticFrame(Mat bgFrame) {
	int *labelMap = (int*)calloc(bgFrame.cols * bgFrame.rows, sizeof(int));	//��ħ�� �Ǵ��ϴ� �뵵

	node tempnode;	//DeQueue�� ����� ���� node
	int countOfObj = segment_queue.count;	//ť �ν��Ͻ��� ��� ����
	stringstream ss;

	//ť�� ������� Ȯ���Ѵ�. ������� �� �̻� ��� �� ���� ���� �� �̹Ƿ� ����
	if (IsEmpty(&segment_queue)){
		free(labelMap);
		return bgFrame;
	}

	vector<int> vectorPreNodeIndex; // ��ü�� �ε��� ������ �����ϱ� ���� ����

	for (int k = 0; k < countOfObj; k++){
		tempnode = Dequeue(&segment_queue);
		//if (m_segmentArray[tempnode.indexOfSegmentArray].timeTag == m_segmentArray[tempnode.indexOfSegmentArray].msec)
		vectorPreNodeIndex.push_back(tempnode.indexOfSegmentArray);	//ť�� �ִ� ��ü���� �ε��� ���� ����
		Enqueue(&segment_queue, tempnode.timeTag, tempnode.indexOfSegmentArray);
	}

	// ť�� ����ִ� ��ü ���� ��ŭ DeQueue. 
	for (int i = 0; i < countOfObj; i++) {
		//dequeue�� ��ü�� ����Ѵ�.
		tempnode = Dequeue(&segment_queue);
		BOOL isCross = false;
		int curIndex = tempnode.indexOfSegmentArray;

		//printf("\n%d : %s", i + 1, m_segmentArray[curIndex].fileName);
		//��ü�� ���� ��ü�� ��ġ���� ��
		if (i != 0 && m_segmentArray[curIndex].timeTag == m_segmentArray[curIndex].msec){	//ó���� �ƴϰ� ���� ����� ��ü�� timetag�� ù ������ �� ��
			for (int j = 0; j < i; j++){	//������ �׸� ��ü ��ο� ��ġ���� �Ǻ�
				if (IsObjectOverlapingDetector(m_segmentArray, vectorPreNodeIndex, curIndex, j)){
					isCross = false; // ��ġ�� ����
				}
				else{ //��ħ
					isCross = true;
					Enqueue(&segment_queue, tempnode.timeTag, tempnode.indexOfSegmentArray);	//������� �ʰ� �ٽ� ť�� ����
					break;
				}
			}
		}


		if (isCross == false){	//��µ� ��ü�� ���ų� ���� ��ü�� ��ġ�� �ʴ� ���
			//��濡 ��ü�� �ø��� �Լ�

			Mat temp_frame = loadJPGObjectFile(m_segmentArray[tempnode.indexOfSegmentArray], video_filename);
			bgFrame = printObjOnBG(bgFrame, temp_frame, m_segmentArray[tempnode.indexOfSegmentArray], labelMap);

			//Ÿ���±׸� string���� ��ȯ
			string timetag = "";
			int timetagInSec = (m_segmentArray[tempnode.indexOfSegmentArray].timeTag + videoStartMsec) / 1000;	//������ ���۽ð��� �����ش�.
			ss = timeConvertor(timetagInSec);
			timetag = ss.str();

			//Ŀ�õ� �̹����� Ÿ���±׸� �޾��ش�
			//params : (Mat, String to show, ����� ��ġ, ��Ʈ Ÿ��, ��Ʈ ũ��, ����, ����) 
			putText(bgFrame, timetag, Point(m_segmentArray[tempnode.indexOfSegmentArray].left + 5, m_segmentArray[tempnode.indexOfSegmentArray].top - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 150, 150), 2);

			//if (m_segmentArray[tempnode.indexOfSegmentArray + 1].timeTag == m_segmentArray[tempnode.indexOfSegmentArray].timeTag) {
			//	Enqueue(&segment_queue, tempnode.timeTag, tempnode.indexOfSegmentArray + 1);
			//}
			
			//���� �����ӿ� ���� Ÿ���±׸� ���� ��ü�� �ִ��� Ȯ���Ѵ�. ������ EnQueue
			int index = 1;
			while (1){
				//���� �����ӿ� ������ �� ���� �ƹ��͵� ���� �ʴ´�.
				if (m_segmentArray[tempnode.indexOfSegmentArray].frameCount == m_segmentArray[tempnode.indexOfSegmentArray + index].frameCount){
					index++;
				}
				else{//������ ��ȣ�� �Ѿ� ���� ���
					if ((m_segmentArray[tempnode.indexOfSegmentArray].frameCount+1) == m_segmentArray[tempnode.indexOfSegmentArray + index].frameCount){//���� ��ü�� ����� �������� ���� �����Ӱ� 1 ���̰� �� ��
						if (m_segmentArray[tempnode.indexOfSegmentArray].timeTag == m_segmentArray[tempnode.indexOfSegmentArray + index].timeTag && m_segmentArray[tempnode.indexOfSegmentArray].index == m_segmentArray[tempnode.indexOfSegmentArray + index].index) {
							Enqueue(&segment_queue, tempnode.timeTag, tempnode.indexOfSegmentArray + 1);
							break;
						}
						else
							index++;
					}
					else //���� ��ü�� �ִ� �������� ��ü�� �ִ� �����Ӱ� 1���� �̻� �� ��
						break;
				}
			}
		}

	}

	free(labelMap);
	vector<int>().swap(vectorPreNodeIndex);
	return bgFrame;
}
// ��ü�� ���� ��ħ�� �Ǻ��ϴ� �Լ�, �ϳ��� �����ϸ� ��ġ�� ����
bool IsObjectOverlapingDetector(segment *m_segment, vector<int> preNodeIndex_data, int curIndex, int countOfObj_j) {
	return m_segment[curIndex].left > m_segmentArray[preNodeIndex_data.at(countOfObj_j)].right
		|| m_segment[curIndex].right < m_segmentArray[preNodeIndex_data.at(countOfObj_j)].left
		|| m_segment[curIndex].top > m_segmentArray[preNodeIndex_data.at(countOfObj_j)].bottom
		|| m_segment[curIndex].bottom < m_segmentArray[preNodeIndex_data.at(countOfObj_j)].top;
}

//slider control�� �����̸� �߻��ϴ� �ݹ�
void CMFC_SyntheticDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_sliderSearchStartTime)
		SetDlgItemText(IDC_STRING_SEARCH_START_TIME_SLIDER, timeConvertor(m_sliderSearchStartTime.GetPos()).str().c_str());
	else if (pScrollBar == (CScrollBar*)&m_sliderSearchEndTime)
		SetDlgItemText(IDC_STRING_SEARCH_END_TIME_SLIDER, timeConvertor(m_sliderSearchEndTime.GetPos()).str().c_str());
	else if (pScrollBar == (CScrollBar*)&m_sliderFps)
		SetDlgItemText(IDC_STRING_FPS_SLIDER, to_string(m_sliderFps.GetPos()).c_str());
	else if (pScrollBar == (CScrollBar*)&m_SliderWMIN)
		SetDlgItemText(IDC_SEG_STRING_VAL_MIN_W, to_string(m_SliderWMIN.GetPos()).c_str());
	else if (pScrollBar == (CScrollBar*)&m_SliderWMAX)
		SetDlgItemText(IDC_SEG_STRING_VAL_MAX_W, to_string(m_SliderWMAX.GetPos()).c_str());
	else if (pScrollBar == (CScrollBar*)&m_SliderHMIN)
		SetDlgItemText(IDC_SEG_STRING_VAL_MIN_H, to_string(m_SliderHMIN.GetPos()).c_str());
	else if (pScrollBar == (CScrollBar*)&m_SliderHMAX)
		SetDlgItemText(IDC_SEG_STRING_VAL_MAX_H, to_string(m_SliderHMAX.GetPos()).c_str());


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

// Play Button�� ������ ��쿡 ����Ǵ� �ݹ�
void CMFC_SyntheticDlg::OnClickedBtnPlay()
{
	if (radioChoice == 0 && isPlayBtnClicked == false){//������ư�� ���������� ��� - �׳� ���
		KillTimer(BIN_VIDEO_TIMER);
		KillTimer(SYN_RESULT_TIMER);
		SetTimer(VIDEO_TIMER, 1000 / m_sliderFps.GetPos(), NULL);
		isPlayBtnClicked = true;
		isPauseBtnClicked = false;
	}
	else if (radioChoice == 2 && isPlayBtnClicked == false){//������ư�� ���������� ��� - ���� �� ��ü ���� �ٿ������ �׷��� ���� ���
		KillTimer(SYN_RESULT_TIMER);
		KillTimer(VIDEO_TIMER);
		SetTimer(BIN_VIDEO_TIMER, 1000 / m_sliderFps.GetPos(), NULL);
		isPlayBtnClicked = true;
		isPauseBtnClicked = false;

	}

	else if (radioChoice == 1 && isPlayBtnClicked == false){ //������ư�� �ռ������� ��� - ������ ���� �ռ��� ���� ���
		isPlayBtnClicked = true;
		isPauseBtnClicked = false;

		boolean isSynPlayable = checkSegmentation();

		if (isSynPlayable){
			char *txtBuffer = new char[100];	//�ؽ�Ʈ���� ���� �� ����� buffer

			string path = "./";
			path.append(getTextFileName(video_filename));

			fp = fopen(path.c_str(), "r");

			//*******************************************�ؽ�Ʈ������ �о ����****************************************************************
			m_segmentArray = new segment[BUFFER];  //(segment*)calloc(BUFFER, sizeof(segment));	//�ؽ�Ʈ ���Ͽ��� ���� segment ������ ������ �迭 �ʱ�ȭ

			segmentCount = 0;
			fseek(fp, 0, SEEK_SET);	//������ ó������ �̵�
			fgets(txtBuffer, 99, fp);
			sscanf(txtBuffer, "%d", &videoStartMsec);	//�ؽ�Ʈ ���� ù�ٿ� ��õ� ���� ���� ���� �ð� �޾ƿ�

			// frameInfo.txt ���Ͽ��� �����͸� ���� �Ͽ� segment array �ʱ�ȭ
			while (!feof(fp)) {
				fgets(txtBuffer, 99, fp);

				// txt���Ͽ� �ִ� ������ �����͵� segmentArray ���۷� ����
				sscanf(txtBuffer, "%d_%d_%d_%d %d %d %d %d %d %d",
					&m_segmentArray[segmentCount].timeTag, &m_segmentArray[segmentCount].msec,
					&m_segmentArray[segmentCount].frameCount, &m_segmentArray[segmentCount].index,
					&m_segmentArray[segmentCount].left, &m_segmentArray[segmentCount].top,
					&m_segmentArray[segmentCount].right, &m_segmentArray[segmentCount].bottom,
					&m_segmentArray[segmentCount].width, &m_segmentArray[segmentCount].height);

				// filename ����
				m_segmentArray[segmentCount].fileName
					.append(to_string(m_segmentArray[segmentCount].timeTag)).append("_")
					.append(to_string(m_segmentArray[segmentCount].msec)).append("_")
					.append(to_string(m_segmentArray[segmentCount].frameCount)).append("_")
					.append(to_string(m_segmentArray[segmentCount].index)).append(".jpg");

				// m_segmentArray�� �ε��� ����
				segmentCount++;
			}

			// ���� ���� ����Ͽ� m_segmentArray�� TimeTag������ ����
			segment *tmp_segment = new segment; // �ӽ� segment ��������, �޸� ������ �����ϰ� �ϱ�
			for (int i = 0; i < segmentCount; i++) {
				for (int j = 0; j < segmentCount - 1; j++) {
					if (m_segmentArray[j].timeTag > m_segmentArray[j + 1].timeTag) {
						// m_segmentArray[segmentCount]�� m_segmentArray[segmentCount + 1]�� ��ü
						*tmp_segment = m_segmentArray[j + 1];
						m_segmentArray[j + 1] = m_segmentArray[j];
						m_segmentArray[j] = *tmp_segment;
					}
				}
			}

			//���� Ȯ�� �ڵ�
			//{
			//for (int i = 0; i < segmentCount; i++)
			//cout << m_segmentArray[i].fileName << endl;
			//}

			// �ӽ� ���� �޸� ����
			delete tmp_segment;
			delete[] txtBuffer;

			// �ؽ�Ʈ ���� �ݱ�
			fclose(fp);
			//****************************************************************************************************************

			//ť �ʱ�ȭ
			InitQueue(&segment_queue);

			/************************************/
			//TimeTag�� Edit box�κ��� �Է¹���
			unsigned int obj1_TimeTag = m_sliderSearchStartTime.GetPos() * 1000;	//�˻��� TimeTag1
			unsigned int obj2_TimeTag = m_sliderSearchEndTime.GetPos() * 1000;	//�˻��� TimeTag2

			if (obj1_TimeTag > obj2_TimeTag){
				AfxMessageBox("Search start time can't larger than end time");
				return;
			}

			bool find1 = false;
			bool find2 = false;

			int prevTimetag = 0;
			int prevIndex = -1;

			//����� ��ü�� ť�� �����ϴ� �κ�
			for (int i = 0; i < segmentCount; i++) {
				//start timetag�� end timetag ���̸� enqueue
				if (m_segmentArray[i].timeTag >= obj1_TimeTag && m_segmentArray[i].timeTag <= obj2_TimeTag) {	//���� ã�� ���߰� ��ġ�ϴ� Ÿ���±׸� ã���� ���
					if (m_segmentArray[i].timeTag == m_segmentArray[i].msec){
						printf("%s\n", m_segmentArray[i].fileName);
						Enqueue(&segment_queue, m_segmentArray[i].timeTag, i);	//����ؾ��� ��ü�� ù �������� Ÿ���±׿� ��ġ�� ť�� ����
						prevTimetag = m_segmentArray[i].timeTag;
						prevIndex = m_segmentArray[i].index;
					}
				}
			}
			/***********/


			//�������� Ÿ�̸� ����
			KillTimer(BIN_VIDEO_TIMER);
			KillTimer(VIDEO_TIMER);
			//Ÿ�̸� ����	params = timerID, ms, callback�Լ� ��(NULL�̸� OnTimer)
			SetTimer(SYN_RESULT_TIMER, 1000 / m_sliderFps.GetPos(), NULL);
		}
		else{ //���� ���ϴ� ��� segmentation�� �����϶�� ���
			AfxMessageBox("You can't play without segmentation results");
		}
	}
	else{}


}

// segmentation�� �� ���� �Է¹޴� ���� ������ �������ִ� �Լ�
bool segmentationTimeInputException(CString str_h, CString str_m) {
	// �� :: 1~24, �� :: 1~60
	if ((atoi(str_h) > 0 && atoi(str_h) <= 24)
		&& (atoi(str_m) > 0 && atoi(str_m) <= 60)){
		return true;
	}

	else if ((str_h == "0" && atoi(str_h) == 0)
		|| (str_m == "0" && atoi(str_m) == 0))
		return true;
	// To Do :: �ϳ��� 0, �ٸ� �ϳ��� ���ڸ� �޾��� �� ����Ǵ� ��찡 ����
	else if ((str_h != "0" && atoi(str_h) == 0)
		|| (str_m != "0" && atoi(str_m) == 0))
		return false;

	else
		return false;
}


//00:00:00 �������� timetag�� ��ȯ
stringstream timeConvertor(int t) {
	int hour;
	int min;
	int sec;
	stringstream s;

	hour = t / 3600;
	min = (t % 3600) / 60;
	sec = t % 60;

	if (t / 3600 < 10)
		s << "0" << hour << " : ";
	else
		s << hour << " : ";

	if ((t % 3600) / 60 < 10)
		s << "0" << min << " : ";
	else
		s << min << " : ";

	if (t % 60 < 10)
		s << "0" << sec;
	else
		s << sec;

	return s;
}

//load ��ư�� ������ �߻��ϴ� �ݹ�
void CMFC_SyntheticDlg::OnBnClickedBtnMenuLoad(){
	loadFile();
}

void CMFC_SyntheticDlg::SetRadioStatus(UINT value) {
	UpdateData(TRUE);
	switch (mRadioPlay) {
	case 0:
		radioChoice = 0;
		SetTimer(LOGO_TIMER, 1, NULL);
		printf("���� ���� ���� ��ư ���õ�\n");
		break;
	case 1:
		radioChoice = 1;
		SetTimer(LOGO_TIMER, 1, NULL);
		printf("�ռ� ���� ���� ��ư ���õ�\n");
		break;
	case 2:
		radioChoice = 2;
		SetTimer(LOGO_TIMER, 1, NULL);
		printf("���� ���� ���� ��ư ���õ�\n");
		break;
	default:
		radioChoice = 0;
		printf("���� ���� ���� ��ư ���õ�\n");
		break;
	}
}



void CMFC_SyntheticDlg::OnBnClickedBtnPause()
{
	// TODO: Add your control notification handler code here
	if (isPauseBtnClicked == false){
		isPlayBtnClicked = false;
		isPauseBtnClicked = true;

		KillTimer(LOGO_TIMER);
		KillTimer(VIDEO_TIMER);
		KillTimer(BIN_VIDEO_TIMER);
		KillTimer(SYN_RESULT_TIMER);
	}

}

bool CMFC_SyntheticDlg::checkSegmentation()
{
	string path = "./";
	path.append(getTextFileName(video_filename));

	FILE *txtFile = fopen(path.c_str(), "r");

	if (txtFile){	//������ ����� �ҷ����� ���
		//������ ������ �̵��Ͽ� ���� ũ�� ����
		fseek(txtFile, 0, SEEK_END);
		if (ftell(txtFile) != 0){	//���� ũ�Ⱑ 0 �� �ƴ� ��� ����
			fclose(txtFile);
			return true;
		}
		else{ //���� ũ�Ⱑ 0 �� ���
			fclose(txtFile);
			return false;
		}
	}
	else{	//������ �ҷ����� �� �� ���
		printf("\nCan't find txt file");
		return false;
	}
}