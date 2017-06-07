// MFC_SyntheticDlg.cpp : implementation file

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

const int FRAMECOUNT_FOR_MAKE_BACKGROUND = 100; // ����� ����� ���� �ʿ��� ������ī��Ʈ
/***  ��������  ***/
Mat m_resultBackground;
segment *m_segmentArray;
Queue segment_queue; // C++ STL�� queue Ű����� ��ġ�� ������ ������ ������
int videoStartMsec, segmentCount, fps;
// ���� millisecond, ���׸�Ʈ ī���ú���, �ʴ� �����Ӽ�

std::string video_filename(""); // �Է¹��� �������� �̸�


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
	pStringStartTime->MoveWindow(box_segmentationX + padding, box_segmentationY + box_segmentationHeight*0.3, 230, 20, TRUE);
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
	char szFilter[] = "Video (*.avi, *.MP4) | *.avi;*.mp4; | All Files(*.*)|*.*||";	//�˻� �ɼ�
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, AfxGetMainWnd());	//���� ���̾�α� ����
	dlg.DoModal();	//���̾�α� ���

	// Path�� �޾ƿͼ� filename���� ��� ������(������� �̸��� ������ �� ���)
	CString cstrImgPath = dlg.GetPathName();
	video_filename = getFileName(cstrImgPath, '\\');
	printf("%s", video_filename);

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
	SetDlgItemText(IDC_STRING_SEARCH_START_TIME_SLIDER, _T("00 : 00 : 00"));
	SetDlgItemText(IDC_STRING_SEARCH_END_TIME_SLIDER, _T("00 : 00 : 00"));
	SetDlgItemText(IDC_STRING_FPS_SLIDER, to_string(fps).c_str());
	m_sliderFps.SetPos(fps);

	SetTimer(VIDEO_TIMER, fps, NULL);

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
	AfxMessageBox("OK ��ư ����");
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
		rect.left, rect.top, rect.right, rect.bottom,
		0, 0, tempImage->width, tempImage->height,
		tempImage->imageData, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	cvReleaseImage(&tempImage);
}

//Ÿ�̸�
void CMFC_SyntheticDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
	switch (nIDEvent){
	case VIDEO_TIMER:
		if (true){
			printf("$");
			Mat temp_frame;
			capture.read(temp_frame);
			DisplayImage(IDC_RESULT_IMAGE, temp_frame, VIDEO_TIMER);
			temp_frame.release();
		}
		break;

	case SYN_RESULT_TIMER:
		if (isPlayBtnClicked == true){
			printf("#");
			//TODO mat�� �ռ��� ����� �־��ش�.
			Mat syntheticResult;
			syntheticResult = getSyntheticFrame(syntheticResult);
			DisplayImage(IDC_RESULT_IMAGE, syntheticResult, SYN_RESULT_TIMER);
			syntheticResult.release();
		}
		break;
	}

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
	if (segmentationTimeInputException(str_startHour, str_startMinute) ) 
		segmentationOperator(&capture, atoi(str_startHour), atoi(str_startMinute));	//Object Segmentation
	
	// ���� �� �Է½� ����ó��
	else {
	}
}

// segmentation ��� ����, ��ü ���� �� ���Ϸ� ����
void segmentationOperator(VideoCapture* vc_Source, int videoStartHour, int videoStartMin)
{
	videoStartMsec = (videoStartHour * 60 + videoStartMin) * 60 * 1000;

	unsigned int COLS = (int)vc_Source->get(CV_CAP_PROP_FRAME_WIDTH);	//���� ����
	unsigned int ROWS = (int)vc_Source->get(CV_CAP_PROP_FRAME_HEIGHT);	//���� ����

	unsigned char* result = (unsigned char*)malloc(sizeof(unsigned char)* ROWS * COLS);

	/* ���� �ؽ�Ʈ ��� */
	char strBuffer[MAX_STR_BUFFER_SIZE] = { 0, }; // fps���
	char timeBuffer[MAX_STR_BUFFER_SIZE] = { 0, }; // time ���
	Scalar color(0, 0, 255); // B/G/R
	int thickness = 3;	// line thickness

	// humanDetector Vector
	vector<component> humanDetectedVector, prevHumanDetectedVector;

	/* Mat */
	Mat frame(ROWS, COLS, CV_8UC3); // Mat(height, width, channel)
	Mat frame_g(ROWS, COLS, CV_8UC1);
	Mat background(ROWS, COLS, CV_8UC1); // ��� �����Ӱ� ���� ������

	//frame ī���Ϳ� ���� millisecond
	int frameCount = 0;
	unsigned int currentMsec;

	// �� ��ü �������� ������ �� �� �ؽ�Ʈ ���� ����
	FILE *fp; // frameInfo�� �ۼ��� File Pointer
	fp = fopen(RESULT_TEXT_FILENAME, "w");	// ������
	fprintf(fp, to_string(videoStartMsec).append("\n").c_str());	//ù�ٿ� ������۽ð� ������
	
	while (1) {
		vc_Source->read(frame); //get single frame
		if (frame.empty()) {	//����ó��. �������� ����
			perror("Empty Frame");
			break;
		}
		//�׷��̽����� ��ȯ
		cvtColor(frame, frame_g, CV_RGB2GRAY);

		// �������κ�
		if (frameCount <= FRAMECOUNT_FOR_MAKE_BACKGROUND) {
			// cvtColor(background, background, CV_RGB2GRAY);
			BackgroundMaker(frame_g, background, ROWS, COLS);
			if (frameCount == (FRAMECOUNT_FOR_MAKE_BACKGROUND - 1)) {
				string background_filename = "background_";
				background_filename.append(video_filename).append(".jpg");
				int background_write_check = imwrite(background_filename, background);
				printf("background Making Complete!!\n");
			}
		}

		// ���� ����
		frame_g = ExtractFg(frame_g, background, ROWS, COLS);

		// ����ȭ
		threshold(frame_g, frame_g, 5, 255, CV_THRESH_BINARY);

		// ������ ����
		frame_g = morphologicalOperation(frame_g);
		blur(frame_g, frame_g, Size(9, 9));

		// MAT������ �󺧸�
		humanDetectedVector.clear();
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
// ������ �̸��κ��� ����
string allocatingComponentFilename(vector<component> humanDetectedVector, int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector) {
	string name;
	return name.append(to_string(timeTag)).append("_")
		.append(to_string(currentMsec)).append("_")
		.append(to_string(frameCount)).append("_")
		.append(to_string(indexOfhumanDetectedVector));
}

// ����� ������ ������ ����� ��, (����� �� ROI�� ���ƴ����� �Ǻ�, ��ġ�� true�ε�??)
bool comparePrevDetection(vector<component> curr_detected, vector<component> prev_detected, int curr_index, int prev_index) {
	return curr_detected[curr_index].left < prev_detected[prev_index].right
		&& curr_detected[curr_index].right < prev_detected[prev_index].left
		&& curr_detected[curr_index].top < prev_detected[prev_index].bottom
		&& curr_detected[curr_index].bottom < prev_detected[prev_index].top;
}

// humanDetectedVector�� �����̸��� timetag�� �ο�
// �׸��� TXT, JPG���Ͽ� �����ϴ� ���
void componentVectorHandling() {
	// TO DO :: �ڵ��� �ߺ��� ���̱� ���� ����� ��� ���� ������ �����ϱ�(�Ű����� ������)
}

vector<component> humanDetectedProcess(vector<component> humanDetectedVector, vector<component> prevHumanDetectedVector
	, Mat frame, int frameCount, int videoStartMsec, unsigned int currentMsec, FILE *fp) {
	int prevTimeTag;
	for (int i = 0; i < humanDetectedVector.size(); i++) {
		// TODO : ���� �����ӿ��� ���������Ӱ� ��ġ�� obj�� �ִ��� �Ǵ��Ѵ�. 
		// ���� ������Ʈ�� ����������Ʈ�� �ΰ��� ��ĥ ��� ��� ó���� ���ΰ�?
		if (prevHumanDetectedVector.empty() == 0) {	//���� �������� ����� ��ü�� ���� ���
			bool findFlag = false;
			for (int j = 0; j < prevHumanDetectedVector.size(); j++) {
				if (comparePrevDetection(humanDetectedVector ,prevHumanDetectedVector, i, j)) {	// �� ROI�� ��ĥ ���
					// ���� TimeTag�� �Ҵ�
					prevTimeTag = prevHumanDetectedVector[j].timeTag;
					
					humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, prevTimeTag, currentMsec, frameCount, i);
					humanDetectedVector[i].timeTag = prevTimeTag;
					saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec);
					saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
					findFlag = true;
					//break;
				}
			}

			if (findFlag == false) { // �� ��ü�� ����
				humanDetectedVector[i].timeTag = currentMsec;
				humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, currentMsec, currentMsec, frameCount, i);
				saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec);
				saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
			}
		}
		else {	// ù �����̰ų� ���� �����ӿ� ����� ��ü�� ���� ���
			// ���ο� �̸� �Ҵ�
			humanDetectedVector[i].timeTag = currentMsec;
			humanDetectedVector[i].fileName = allocatingComponentFilename(humanDetectedVector, currentMsec, currentMsec, frameCount, i);
			saveSegmentation_JPG(humanDetectedVector[i], frame, frameCount, currentMsec, i, videoStartMsec);
			saveSegmentation_TXT(humanDetectedVector[i], frameCount, currentMsec, fp, i);
			// �� ���ٿ� ���� �ڵ��ߺ��� �� ����, ���� �߰� �� ���� ���ɼ��� ����(�˻� ��� ������ ��)
		}
	}
	return humanDetectedVector;
}

// �ռ��� �������� �������� ����
Mat getSyntheticFrame(Mat tempBackGround) {
		int *labelMap = (int*)calloc(m_resultBackground.cols * m_resultBackground.rows, sizeof(int));	//��ħ�� �Ǵ��ϴ� �뵵
		node tempnode;	//DeQueue�� ����� ���� node
		int countOfObj = segment_queue.count;	//ť �ν��Ͻ��� ��� ����
		stringstream ss;

		//ť�� ������� Ȯ���Ѵ�
		if (IsEmpty(&segment_queue)){
			free(labelMap);
			return tempBackGround;
		}

		vector<int> vectorPreNodeIndex; // ��ü�� �ε��� ������ �����ϱ� ���� ����

		for (int k = 0; k < countOfObj; k++){
			tempnode = Dequeue(&segment_queue);
			//if (m_segmentArray[tempnode.indexOfSegmentArray].timeTag == m_segmentArray[tempnode.indexOfSegmentArray].msec)
			vectorPreNodeIndex.push_back(tempnode.indexOfSegmentArray);	//ť�� �ִ� ��ü���� �ε��� ���� ����
			Enqueue(&segment_queue, tempnode.timeTag, tempnode.indexOfSegmentArray);
		}

		m_resultBackground.copyTo(tempBackGround);	//�ӽ÷� �� ��� ����
		
		// ť�� ����ִ� ��ü ���� ��ŭ DeQueue. 
		for (int i = 0; i < countOfObj; i++) {
			//dequeue�� ��ü�� ����Ѵ�.
			tempnode = Dequeue(&segment_queue);
			BOOL isCross= false;
			int curIndex = tempnode.indexOfSegmentArray;
			
			//printf("\n%d : %s", i + 1, m_segmentArray[curIndex].fileName);
			//��ü�� ���� ��ü�� ��ġ���� ��
			if (i != 0 && m_segmentArray[curIndex].timeTag == m_segmentArray[curIndex].msec){	//ó���� �ƴϰ� ���� ����� ��ü�� timetag�� ù ������ �� ��
				for (int j = 0; j< i; j++){	//������ �׸� ��ü ��ο� ��ġ���� �Ǻ�
					if (objectOverlapingDetector(m_segmentArray, vectorPreNodeIndex, curIndex, j) ){
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
				tempBackGround = printObjOnBG(tempBackGround, m_segmentArray[tempnode.indexOfSegmentArray], labelMap);

				//Ÿ���±׸� string���� ��ȯ
				string timetag = "";
				int timetagInSec = (m_segmentArray[tempnode.indexOfSegmentArray].timeTag + videoStartMsec) / 1000;	//������ ���۽ð��� �����ش�.
				ss = timeConvertor(timetagInSec);
				timetag = ss.str();

				//Ŀ�õ� �̹����� Ÿ���±׸� �޾��ش�
				//params : (Mat, String to show, ����� ��ġ, ��Ʈ Ÿ��, ��Ʈ ũ��, ����, ����) 
				putText(tempBackGround, timetag, Point(m_segmentArray[tempnode.indexOfSegmentArray].left + 5, m_segmentArray[tempnode.indexOfSegmentArray].top - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 150, 150), 2);

				//������Ͽ� ���� Ÿ���±׸� ���� ��ü�� �ִ��� Ȯ���Ѵ�. ������ EnQueue
				if (m_segmentArray[tempnode.indexOfSegmentArray + 1].timeTag == m_segmentArray[tempnode.indexOfSegmentArray].timeTag) {
					Enqueue(&segment_queue, tempnode.timeTag, tempnode.indexOfSegmentArray + 1);
				}
			}

		}

		free(labelMap);
		vector<int>().swap(vectorPreNodeIndex);
	return tempBackGround;
}
// ��ü�� ���� ��ħ�� �Ǻ��ϴ� �Լ�, �ϳ��� �����ϸ� ��ġ�� ����
bool objectOverlapingDetector(segment *m_segment, vector<int> preNodeIndex_data, int curIndex, int countOfObj_j) {
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
	

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

// Synthesis Button�� ������ ��쿡 ����Ǵ� �ڵ鷯
void CMFC_SyntheticDlg::OnClickedBtnSynPlay()
{
	isPlayBtnClicked = true;
	char txtBuffer[100] = { 0, };	//�ؽ�Ʈ���� ���� �� ����� buffer
	//frameInfo.txt ������ �ְ�, ������ ������� ������ ���డ���ϴٰ� ǥ��
	FILE *fp;
	string path = "./";
	path.append(RESULT_TEXT_FILENAME);
	fp = fopen(path.c_str(), "r");
	boolean isPlayable = false;
	if (fp){	//������ ����� �ҷ����� ���
		//������ ������ �̵��Ͽ� ���� ũ�� ����
		fseek(fp, 0, SEEK_END);
		if (ftell(fp) != 0)	//���� ũ�Ⱑ 0 �� �ƴ� ��� ����
			isPlayable = true;
	}

	if (isPlayable){
		//*******************************************�ؽ�Ʈ������ �о ����****************************************************************
		m_segmentArray = new segment[BUFFER];  //(segment*)calloc(BUFFER, sizeof(segment));	//�ؽ�Ʈ ���Ͽ��� ���� segment ������ ������ �迭 �ʱ�ȭ
		segmentCount = 0;
		fseek(fp, 0, SEEK_SET);	//������ ó������ �̵�
		fgets(txtBuffer, 99, fp);
		sscanf(txtBuffer, "%d", &videoStartMsec);
		
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
		InitQueue(&segment_queue);

		// ���� ��� ������ �ҷ�����
		m_resultBackground = imread("background.jpg");



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
		//����� ��ü�� ť�� �����ϴ� �κ�
		for (int i = 0; i < segmentCount; i++) {
			//start timetag�� end timetag ���̸� enqueue
			if (m_segmentArray[i].timeTag >= obj1_TimeTag && m_segmentArray[i].timeTag <= obj2_TimeTag && prevTimetag != m_segmentArray[i].timeTag) {	//���� ã�� ���߰� ��ġ�ϴ� Ÿ���±׸� ã���� ���
				Enqueue(&segment_queue, m_segmentArray[i].timeTag, i);	//����ؾ��� ��ü�� ù �������� Ÿ���±׿� ��ġ�� ť�� ����
				prevTimetag = m_segmentArray[i].timeTag;
			}
		}
		/***********/


		//�������� Ÿ�̸� ����
		KillTimer(VIDEO_TIMER);
		//Ÿ�̸� ����	params = timerID, ms, callback�Լ� ��(NULL�̸� OnTimer)
		SetTimer(SYN_RESULT_TIMER, 1000 / m_sliderFps.GetPos(), NULL);
	}
	else{ //���� ���ϴ� ��� segmentation�� �����϶�� ���
		AfxMessageBox("You can't play without segmentation results");
	}


}

// segmentation�� �� ���� �Է¹޴� ���� ������ �������ִ� �Լ�
bool segmentationTimeInputException(CString str_h, CString str_m) {
	// �� :: 1~24, �� :: 1~60
	if ((atoi(str_h) > 0 && atoi(str_h) <= 24)
		&& (atoi(str_m) > 0 && atoi(str_m) <= 60) ){
		return true;
	}

	else if ( (str_h == "0" && atoi(str_h) == 0)
		|| (str_m == "0" && atoi(str_m) == 0) )
		return true;

	else if ((str_h != "0" && atoi(str_h) == 0)
		|| (str_m != "0" && atoi(str_m) == 0))
		return false;

	else
		return false;
}

// Video Path���� file�̸��� ���� ��ȯ�ϴ� �Լ� 
String getFileName(CString f_path, char find_char) {
	// ������ \ ���� ���ڿ�
	// �˻���� :: "Video (*.avi, *.MP4) | *.avi;*.mp4; | All Files(*.*)|*.*||", 
	char final_index;
	for (int i = 0; i < f_path.GetLength(); i++) {
		if (f_path[i] == find_char)
			final_index = i;
	} // '\'�� ã��

	for (int i = final_index + 1; i < f_path.GetLength(); i++) {
		if (f_path[final_index + 1] == NULL) break; // ����ó��
		char c = f_path[i];
		video_filename += c;
	}// �������� ������ '\' ������ ���ڿ��� ������

	return video_filename;
}

//TemporalMedian ������� ����� ����� ����ϱ�
int BackgroundMaker(Mat frameimg, Mat bgimg, int rows, int cols) {
	int cnt = 0; // �����ȼ����� ���� �ȼ����� ���Ͽ� �ٲ��� ������(������) ī������(�������Ͼ���)
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (frameimg.data[i * frameimg.cols + j] > bgimg.data[i * bgimg.cols + j]) {//���� �ȼ��� ��� �ȼ����� Ŭ ��
				if (bgimg.data[i * bgimg.cols + j] == 255) // ������ �̹��� �迭�� ���� 255�� ���� ���( �ִ밪 )
					bgimg.data[i * bgimg.cols + j] = 255;
				else
					bgimg.data[i * bgimg.cols + j] += 1;//1�� ����

			} // ��� �����Ӱ� ���Ͽ� ���� �������� ȭ�� ���� ���� ���, ���� ��� �������� ȭ�Ҹ� ���� 
			else if (frameimg.data[i * frameimg.cols + j] < bgimg.data[i * bgimg.cols + j]) {//���� �ȼ��� ��� �ȼ����� ���� ��
				if (bgimg.data[i * bgimg.cols + j] == 0) // ������ �̹��� �迭�� ���� 0���� ���� ���( �ּҰ� )
					bgimg.data[i *bgimg.cols + j] = 0;
				else
					bgimg.data[i * bgimg.cols + j] -= 1;//1�� ����
			} // ��� �����Ӱ� ���Ͽ� ���� �������� ȭ�� ���� ���� ���, ���� ��� �������� ȭ�Ҹ� ����
			else if (frameimg.data[i * frameimg.cols + j] == bgimg.data[i * bgimg.cols + j]) {
				cnt++;
			}
		}
	}

	return cnt;
}
