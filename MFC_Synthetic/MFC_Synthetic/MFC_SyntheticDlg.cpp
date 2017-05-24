
// MFC_SyntheticDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VIDEO_TIMER 1
#define SYN_RESULT_TIMER 2
#define MAX_STR_BUFFER_SIZE  128 // ���ڿ� ��¿� �� ���� ����

int fps;
Mat m_resultBackground;
segment *m_segmentArray;
int segmentCount;
Queue queue;

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
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFC_SyntheticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SYN_SLIDER_START_TIME, m_sliderSearchStartTime);
	DDX_Control(pDX, IDC_SYN_SLIDER_END_TIME, m_sliderSearchEndTime);
	DDX_Control(pDX, IDC_SYN_SLIDER_FPS, m_sliderFps);
}

BEGIN_MESSAGE_MAP(CMFC_SyntheticDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMFC_SyntheticDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_SEGMENTATION, &CMFC_SyntheticDlg::OnBnClickedBtnSegmentation)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_SYN_PLAY, &CMFC_SyntheticDlg::OnClickedBtnSynPlay)
END_MESSAGE_MAP()


// CMFC_SyntheticDlg message handlers

BOOL CMFC_SyntheticDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ShowWindow(SW_SHOWMAXIMIZED);	//��üȭ��
	this->GetWindowRect(m_rectCurHist);	//���̾�α� ũ�⸦ ����

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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
	int dialogHeight = m_rectCurHist.bottom-50;//�۾�ǥ���� ũ�� ����
	int padding = 10;
	int controlBoxHeight = dialogHeight*0.25;
	SetWindowPos(&wndTop, 0, 0, dialogWidth, dialogHeight, SWP_NOMOVE);//���̾�α� ũ�� ����

	//Picture Control
	int pictureContorlWidth = dialogWidth - padding * 4;
	int pictureContorlHeight = dialogHeight - controlBoxHeight - padding * 3;
	pResultImage->MoveWindow(padding, padding, pictureContorlWidth, pictureContorlHeight, TRUE);

	//group box - segmetation
	CWnd *pGroupSegmentation = GetDlgItem(IDC_GROUP_SEG);
	CWnd *pStringStartTime = GetDlgItem(IDC_SEG_STRING_VIDEO_START_TIME);
	CWnd *pStringColon = GetDlgItem(IDC_SEG_STRING_COLON);
	m_pEditBoxStartHour = (CEdit *)GetDlgItem(IDC_SEG_EDITBOX_START_HOUR);	
	m_pEditBoxStartMinute = (CEdit *)GetDlgItem(IDC_SEG_EDITBOX_START_MINUTE); 
	CButton *pButtonSegmentation = (CButton *)GetDlgItem(IDC_BTN_SEG_SEGMENTATION);
	int box_segmentationX = padding;
	int box_segmentationY = padding*2 + pictureContorlHeight;
	int box_segmentationWidth = pictureContorlWidth*0.5;
	int box_segmentationHeight = controlBoxHeight - padding * 3;
	pGroupSegmentation->MoveWindow(box_segmentationX, box_segmentationY, box_segmentationWidth, box_segmentationHeight, TRUE);
	pStringStartTime->MoveWindow(box_segmentationX + padding, box_segmentationY + box_segmentationHeight*0.3, 200, 20, TRUE);
	m_pEditBoxStartHour->MoveWindow(box_segmentationX + padding, box_segmentationY + box_segmentationHeight*0.3+30, 20, 20, TRUE);
	pStringColon->MoveWindow(box_segmentationX + padding+25, box_segmentationY + box_segmentationHeight*0.3 + 30, 20, 20, TRUE);
	m_pEditBoxStartMinute->MoveWindow(box_segmentationX + padding + 35, box_segmentationY + box_segmentationHeight*0.3 + 30, 20, 20, TRUE);
	pButtonSegmentation->MoveWindow(box_segmentationX + box_segmentationWidth - padding - 100, box_segmentationY + box_segmentationHeight - 30, 100, 20, TRUE);
	

	//group box - synthetic
	CWnd *pGroupSynthetic = GetDlgItem(IDC_GROUP_SYN);
	CWnd *pStringSearchStartTime = GetDlgItem(IDC_STRING_SEARCH_START_TIME);
	CWnd *pStringSearchEndTime = GetDlgItem(IDC_STRING_SEARCH_END_TIME);
	CWnd *pStringFps = GetDlgItem(IDC_STRING_FPS);
	CWnd *pStringSearchStartTimeSlider = GetDlgItem(IDC_STRING_SEARCH_START_TIME_SLIDER);
	CWnd *pStringSearchEndTimeSlider = GetDlgItem(IDC_STRING_SEARCH_END_TIME_SLIDER);
	CWnd *pStringFpsSlider = GetDlgItem(IDC_STRING_FPS_SLIDER);
	CButton *pButtonPlay = (CButton *)GetDlgItem(IDC_BTN_SYN_PLAY);
	int box_syntheticX = padding + box_segmentationWidth;
	int box_syntheticY = padding * 2 + pictureContorlHeight;
	int box_syntheticWidth = pictureContorlWidth*0.5;
	int box_syntheticHeight = controlBoxHeight - padding * 3;
	pGroupSynthetic->MoveWindow(box_syntheticX, box_syntheticY, box_syntheticWidth, box_syntheticHeight, TRUE);
	pStringSearchStartTime->MoveWindow(box_syntheticX + padding, box_syntheticY + box_syntheticHeight*0.3, 100, 20, TRUE);
	m_sliderSearchStartTime.MoveWindow(box_syntheticX + padding, box_syntheticY + box_syntheticHeight*0.3+20+padding, 140, 20, TRUE);
	pStringSearchStartTimeSlider->MoveWindow(box_syntheticX + padding+40, box_syntheticY + box_syntheticHeight*0.3 + 40 + padding*2, 140, 20, TRUE);
	
	pStringSearchEndTime->MoveWindow(box_syntheticX + padding + 150, box_syntheticY + box_syntheticHeight*0.3, 100, 20, TRUE);
	m_sliderSearchEndTime.MoveWindow(box_syntheticX + padding + 150, box_syntheticY + box_syntheticHeight*0.3 + 20 + padding, 140, 20, TRUE);
	pStringSearchEndTimeSlider->MoveWindow(box_syntheticX + padding + 40 + 150, box_syntheticY + box_syntheticHeight*0.3 + 40 + padding * 2, 140, 20, TRUE);

	pStringFps->MoveWindow(box_syntheticX + padding + 300, box_syntheticY + box_syntheticHeight*0.3, 100, 20, TRUE);
	m_sliderFps.MoveWindow(box_syntheticX + padding+300, box_syntheticY + box_syntheticHeight*0.3 + 20 + padding, 140, 20, TRUE);
	pStringFpsSlider->MoveWindow(box_syntheticX + padding + 70 + 300, box_syntheticY + box_syntheticHeight*0.3 + 40 + padding * 2, 30, 20, TRUE);

	pButtonPlay->MoveWindow(box_syntheticX + box_syntheticWidth - padding - 100, box_segmentationY + box_syntheticHeight - 30, 100, 20, TRUE);


	
	/*
	slider m_sliderSearchStartTime, m_sliderSearchEndTime, m_sliderFps ����
	*/
	m_sliderSearchStartTime.SetRange(0, 500);
	m_sliderSearchEndTime.SetRange(0, 500);
	m_sliderFps.SetRange(0, 100);

	//***************************************************************************************************************
	
	cimage_mfc = NULL;
	isPlayBtnClicked = false;


	//����� ���� �о��
	//���� ���̾�α� ȣ���ؼ� segmentation �� ���� ����	
	//TODO : ������ Ȯ���ڷ� �����ϱ�
	char szFilter[] = "All Files(*.*)|*.*||";	//�˻� �ɼ�
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, AfxGetMainWnd());	//���� ���̾�α� ����
	dlg.DoModal();	//���̾�α� ���

	CString cstrImgPath = dlg.GetPathName();
	capture.open((string)cstrImgPath);
	if (!capture.isOpened()) { //����ó��. �ش��̸��� ������ ���� ���
		perror("No Such File!\n");
		::SendMessage(GetSafeHwnd(), WM_CLOSE, NULL, NULL);	//���̾� �α� ����
	}

	fps = capture.get(CV_CAP_PROP_FPS);

	//edit box default
	m_pEditBoxStartHour->SetWindowTextA("0");
	m_pEditBoxStartMinute->SetWindowTextA("0");
	//slider default
	SetDlgItemText(IDC_STRING_SEARCH_START_TIME_SLIDER, _T("00:00:00"));
	SetDlgItemText(IDC_STRING_SEARCH_END_TIME_SLIDER, _T("00:00:00"));
	SetDlgItemText(IDC_STRING_FPS_SLIDER, to_string(fps).c_str());
	m_sliderFps.SetPos(fps);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFC_SyntheticDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
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


void CMFC_SyntheticDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	AfxMessageBox("OK ��ư ����");

	//char szFilter[] = "Image (*.BMP, *.GIF, *.JPG, *.PNG) | *.BMP;*.GIF;*.JPG;*.PNG;*.bmp;*.gif;*.jpg;*.png | All Files(*.*)|*.*||";
	//CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, AfxGetMainWnd());
	//if (dlg.DoModal() == IDOK)
	//{
	//	CString cstrImgPath = dlg.GetPathName();

	//AfxMessageBox(cstrImgPath); 
	////
	//Mat src = imread(string(cstrImgPath));
	//DisplayImage(IDC_RESULT_IMAGE, src);
	//}

}


// ���÷��� �Լ�
void CMFC_SyntheticDlg::DisplayImage(int IDC_PICTURE_TARGET, Mat targetMat, int TIMER_ID){

	if (targetMat.empty()) {	//����ó��. �������� ����
		perror("Empty Frame");
		KillTimer(TIMER_ID);
		return ;
	}
	BITMAPINFO bitmapInfo;
	memset(&bitmapInfo, 0, sizeof(bitmapInfo));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biWidth = targetMat.cols;
	bitmapInfo.bmiHeader.biHeight = -targetMat.rows;

	IplImage *tempImage;

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
		rect.left, rect.top, rect.right, rect.bottom,
		0, 0, tempImage->width, tempImage->height,
		tempImage->imageData, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	cvReleaseImage(&tempImage);
}

//Ÿ�̸�
void CMFC_SyntheticDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	CDialogEx::OnTimer(nIDEvent);

	switch (nIDEvent){
	case VIDEO_TIMER:
		printf(".");
		if (isPlayBtnClicked == true){
			//capture->read(mat_frame);
			//DisplayImage(IDC_RESULT_IMAGE, mat_frame);
		}
		break;

	case SYN_RESULT_TIMER:
		printf("+");
		if (isPlayBtnClicked == true){
			//TODO mat�� �ռ��� ����� �־��ش�.
			Mat syntheticResult;
			syntheticResult = getSyntheticFrame(syntheticResult);
			//capture->read(mat_frame);
			DisplayImage(IDC_RESULT_IMAGE, syntheticResult, SYN_RESULT_TIMER);
			printf("ASD");
			syntheticResult.release();
		}
		break;
	}

}

void CMFC_SyntheticDlg::OnBnClickedBtnSegmentation()
{
	//TODO : Edit box�� ���� �Է½� ����ó�� (atoi()�δ� ���ڿ� null�� ���� 0 �� �Ѵ� 0 ���� ��µ�)
	//Edit Box�κ��� ���� �ð��� ���� �о��
	CString startHour;
	m_pEditBoxStartHour->GetWindowTextA(startHour);
	CString startMinute;
	m_pEditBoxStartMinute->GetWindowTextA(startMinute);

	humonDetector(&capture, atoi(startHour), atoi(startMinute));	//Object Segmentation
	}


void humonDetector(VideoCapture* vc_Source, int videoStartHour, int videoStartMin)
{
	int videoStartMsec = (videoStartHour * 60 + videoStartMin) * 60 * 1000;

	unsigned int COLS = (int)vc_Source->get(CV_CAP_PROP_FRAME_WIDTH);	//���� ����
	unsigned int ROWS = (int)vc_Source->get(CV_CAP_PROP_FRAME_HEIGHT);	//���� ����

	unsigned char* result = (unsigned char*)malloc(sizeof(unsigned char)* ROWS * COLS);

	/* ���� �ؽ�Ʈ ��� */
	char strBuffer[MAX_STR_BUFFER_SIZE] = { 0, }; // fps���
	char timeBuffer[MAX_STR_BUFFER_SIZE] = { 0, }; // time ���
	Scalar color(0, 0, 255); // B/G/R
	int thickness = 3;	// line thickness

	vector<component> humanDetectedVector;
	vector<component> prevHumanDetectedVector;
	unsigned int currentMsec;



	/* Mat */
	Mat frame(ROWS, COLS, CV_8UC3); // Mat(height, width, channel)
	Mat frame_g(ROWS, COLS, CV_8UC1);
	Mat background(ROWS, COLS, CV_8UC1); // ��� �����Ӱ� ���� ������

	//frame ī����
	int frameCount = 0;

	// �� ��ü �������� ������ �� �� �ؽ�Ʈ ���� ����
	FILE *fp; // frameInfo�� �ۼ��� File Pointer
	fp = fopen(RESULT_TEXT_FILENAME, "w");	// ������

	// ���� background ���
	//TODO ��� �ڵ� �����ϱ�
	background = imread("background.jpg");
	cvtColor(background, background, CV_RGB2GRAY);

	//MessageBox(0, "Just on second!\nSegmentation in progress...", "Dude, Wait", NULL);

	while (1) {
		vc_Source->read(frame); //get single frame
		if (frame.empty()) {	//����ó��. �������� ����
			perror("Empty Frame");
			break;
		}
		//�׷��̽����� ��ȯ
		cvtColor(frame, frame_g, CV_RGB2GRAY);

		// ��� ����
		// background = TemporalMedianBg(frame_g, background, ROWS, COLS);

		// ���� ����
		frame_g = ExtractFg(frame_g, background, ROWS, COLS);

		// ����ȭ
		threshold(frame_g, frame_g, 5, 255, CV_THRESH_BINARY);

		// ������ ����
		frame_g = morphologicalOperation(frame_g);
		blur(frame_g, frame_g, Size(9, 9));

		// MAT������ �󺧸�
		humanDetectedVector.clear();
		// humanDetectedVector = connectedComponentLabelling_sequencial(frame_g, ROWS, COLS);
		humanDetectedVector = connectedComponentsLabelling(frame_g, ROWS, COLS);

		// ������ ���� ��������
		currentMsec = vc_Source->get(CV_CAP_PROP_POS_MSEC);

		// ������ ó���Ͽ� ���Ϸ� �����ϱ�
		humanDetectedVector = humanDetectedProcess(humanDetectedVector, prevHumanDetectedVector,
			frame, frameCount, videoStartMsec, currentMsec, fp);

		// ���� ������ �����͸� ���� �����Ϳ� �����ϱ�
		prevHumanDetectedVector = humanDetectedVector;

		frameCount++;	//increase frame count
	}

	//�޸� ����
	free(result); 	frame.release(); frame_g.release(); background.release();
	vector<component>().swap(humanDetectedVector);
	vector<component>().swap(prevHumanDetectedVector);
	fclose(fp);	// �ؽ�Ʈ ���� �ݱ�

	//HWND hWnd = ::FindWindow(NULL, "Dude, Wait");
	//if (hWnd){ ::PostMessage(hWnd, WM_CLOSE, 0, 0); }
	MessageBox(0, "Done!!", "ding-dong", MB_OK);
}

Mat morphologicalOperation(Mat img_binary) {
	//morphological opening ���� ������ ����
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//morphological closing ������ ���� �޿��
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	return img_binary;
}

string allocatingComponentFilename(vector<component> humanDetectedVector, int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector) {
	string name;
	name.append(to_string(timeTag)).append("_")
		.append(to_string(currentMsec)).append("_")
		.append(to_string(frameCount)).append("_")
		.append(to_string(indexOfhumanDetectedVector));
	return name;
}


vector<component> humanDetectedProcess(vector<component> humanDetectedVector, vector<component> prevHumanDetectedVector
	, Mat frame, int frameCount, int videoStartMsec, unsigned int currentMsec, FILE *fp)
{
	int prevTimeTag;
	for (int i = 0; i < humanDetectedVector.size(); i++) {
		// TODO : ���� �����ӿ��� ���������Ӱ� ��ġ�� obj�� �ִ��� �Ǵ��Ѵ�. 
		// ���� ������Ʈ�� ����������Ʈ�� �ΰ��� ��ĥ ��� ��� ó���� ���ΰ�?
		if (prevHumanDetectedVector.empty() == 0) {	//���� �������� ����� ��ü�� ���� ���
			bool findFlag = false;
			for (int j = 0; j < prevHumanDetectedVector.size(); j++) {
				if (humanDetectedVector[i].left < prevHumanDetectedVector[j].right
					&& humanDetectedVector[i].right > prevHumanDetectedVector[j].left
					&& humanDetectedVector[i].top < prevHumanDetectedVector[j].bottom
					&& humanDetectedVector[i].bottom > prevHumanDetectedVector[j].top) {	// �� ROI�� ��ĥ ���

					prevTimeTag = prevHumanDetectedVector[j].timeTag;
					humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, prevTimeTag, currentMsec, frameCount, i);

					humanDetectedVector[i].timeTag = prevTimeTag;
					//printf("\n@@@@@@@@@@@@@@@@@@@@@\n %s�� ���� %s �� ��ħ%d, %d\n@@@@@@@@@@@@@@@@\n", prevHumanDetectedVector[j].fileName.c_str(), humanDetectedVector[i].fileName.c_str());
					saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec);
					saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
					findFlag = true;
					//break;
				}
			}

			if (findFlag == false) {
				humanDetectedVector[i].timeTag = currentMsec;
				humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, currentMsec, currentMsec, frameCount, i);
				//printf("\n*********************************\n ����ä %s ����\n*********************************\n", humanDetectedVector[i].fileName.c_str());

				saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec);
				saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
			}
		}
		else {	// ù �����̰ų� ���� �����ӿ� ����� ��ü�� ���� ���
			// ���ο� �̸� �Ҵ�
			humanDetectedVector[i].timeTag = currentMsec;
			humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, currentMsec, currentMsec, frameCount, i);
			//printf("\n*********************************\n ����ä %s ����\n*********************************\n", humanDetectedVector[i].fileName.c_str());
			saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec);
			saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
		}
	}
	return humanDetectedVector;
}



Mat getSyntheticFrame(Mat tempBackGround) {
	//���
		int *labelMap = (int*)calloc(m_resultBackground.cols * m_resultBackground.rows, sizeof(int));	//��ħ�� �Ǵ��ϴ� �뵵
		node tempnode;	//DeQueue�� ����� ���� node
		int countOfObj = queue.count;	//ť �ν��Ͻ��� ��� ����

		//ť�� ������� Ȯ���Ѵ�
		if (IsEmpty(&queue))
			return tempBackGround;

		m_resultBackground.copyTo(tempBackGround);	//�ӽ÷� �� ��� ����



		//DeQueue�� ť�� ����ִ� ��ü ���� ��ŭ �Ѵ�. 
		for (int i = 0; i < countOfObj; i++) {
			//dequeue�� ��ü�� ����Ѵ�.
			tempnode = Dequeue(&queue);

			//if (tempnode.timeTag == 66920)
			//printf("\n@ %d / %s", tempnode.indexOfSegmentArray, m_segmentArray[tempnode.indexOfSegmentArray].fileName);
			//��濡 ��ü�� �ø��� �Լ�
			tempBackGround = printObjOnBG(tempBackGround, m_segmentArray[tempnode.indexOfSegmentArray], labelMap);

			//������Ͽ� ���� Ÿ���±׸� ���� ��ü�� �ִ��� Ȯ���Ѵ�. ������ EnQueue
			if (m_segmentArray[tempnode.indexOfSegmentArray + 1].timeTag == m_segmentArray[tempnode.indexOfSegmentArray].timeTag) {
				Enqueue(&queue, tempnode.timeTag, tempnode.indexOfSegmentArray + 1);
				//printf("@ %d", tempnode.indexOfSegmentArray + 1);
			}

		}

	
		free(labelMap);
	return tempBackGround;
}

//slider control�� �����̸� �߻��ϴ� �ݹ�
void CMFC_SyntheticDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	if (pScrollBar == (CScrollBar*)&m_sliderSearchStartTime)
		SetDlgItemText(IDC_STRING_SEARCH_START_TIME_SLIDER, timeConvertor(m_sliderSearchStartTime.GetPos()).str().c_str());
	else if (pScrollBar == (CScrollBar*)&m_sliderSearchEndTime)
		SetDlgItemText(IDC_STRING_SEARCH_END_TIME_SLIDER, timeConvertor(m_sliderSearchEndTime.GetPos()).str().c_str());
	else if (pScrollBar == (CScrollBar*)&m_sliderFps)
		SetDlgItemText(IDC_STRING_FPS_SLIDER, to_string(m_sliderFps.GetPos()).c_str());
	

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CMFC_SyntheticDlg::OnClickedBtnSynPlay()
{
	// TODO: Add your control notification handler code here
	isPlayBtnClicked = true;


	//frameInfo.txt ������ �ְ�, ������ ������� ������ ���డ���ϴٰ� ǥ��
	FILE *f;
	string path = "./";
	path.append(RESULT_TEXT_FILENAME);
	f = fopen(path.c_str(), "r");
	boolean isPlayable = false;
	if (f){
		fseek(f, 0, SEEK_END);
		if (ftell(f) != 0)
			isPlayable = true;
	}

	if (isPlayable){	//segment ������ ���� ��쿡�� ����
		//*******************************************�ؽ�Ʈ������ �о ����****************************************************************
		m_segmentArray = new segment[BUFFER];  //(segment*)calloc(BUFFER, sizeof(segment));	//�ؽ�Ʈ ���Ͽ��� ���� segment ������ ������ �迭 �ʱ�ȭ

		segmentCount = 0;
		char txtBuffer[100] = { 0, };	//�ؽ�Ʈ���� ���� �� ����� buffer
		// frameInfo.txt ���Ͽ��� �����͸� ���� �Ͽ� segment array �ʱ�ȭ
		FILE *fp = NULL;
		fp = fopen(RESULT_TEXT_FILENAME, "r");
		if (fp == NULL) {	//����ó��. �ؽ�Ʈ ������ ã�� �� ����
			perror("No File!");
			exit(1);
		}
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
		segment tmp_segment;
		for (int i = 0; i < segmentCount; i++) {
			for (int j = 0; j < segmentCount - 1; j++) {
				if (m_segmentArray[j].timeTag > m_segmentArray[j + 1].timeTag) {
					// m_segmentArray[segmentCount]�� m_segmentArray[segmentCount + 1]�� ��ü
					tmp_segment = m_segmentArray[j + 1];
					m_segmentArray[j + 1] = m_segmentArray[j];
					m_segmentArray[j] = tmp_segment;
				}
			}
		}

		//���� Ȯ�� �ڵ�
		//{
		//for (int i = 0; i < segmentCount; i++)
		//cout << m_segmentArray[i].fileName << endl;
		//}

		fclose(fp);	// �ؽ�Ʈ ���� �ݱ�
		//****************************************************************************************************************

		//ť �ʱ�ȭ
		InitQueue(&queue);

		// ���� ��� ������ �ҷ�����
		m_resultBackground = imread("background.jpg");



		/************************************/
		//TODO timetag �Է¹ޱ�
		unsigned int obj1_TimeTag = m_sliderSearchStartTime.GetPos() * 1000;	//�˻��� TimeTag1
		unsigned int obj2_TimeTag = m_sliderSearchEndTime.GetPos() * 1000;	//�˻��� TimeTag2

		if (obj1_TimeTag > obj2_TimeTag){
			AfxMessageBox("Search start time can't larger than end time");
			return;
		}

		bool find1 = false;
		bool find2 = false;

		int prevTimetag = 0;
		//����� ��ü�� ť�� �����ϴ� �κ�
		for (int i = 0; i < segmentCount; i++) {
			//start timetag�� end timetag ���̸� enqueue
			if (m_segmentArray[i].timeTag >= obj1_TimeTag && m_segmentArray[i].timeTag <= obj2_TimeTag && prevTimetag != m_segmentArray[i].timeTag) {	//���� ã�� ���߰� ��ġ�ϴ� Ÿ���±׸� ã���� ���
				Enqueue(&queue, m_segmentArray[i].timeTag, i);	//����ؾ��� ��ü�� ù �������� Ÿ���±׿� ��ġ�� ť�� ����
				prevTimetag = m_segmentArray[i].timeTag;
			}
		}
		/***********/


		//Ÿ�̸� ����	params = timerID, ms, callback�Լ� ��(NULL�̸� OnTimer)
		SetTimer(SYN_RESULT_TIMER, 1000 / m_sliderFps.GetPos(), NULL);
	}
	else{ //segment ������ ���� ��� segmentation�� �����϶�� ���
		AfxMessageBox("You can't play without segmentation results");
	}


}
