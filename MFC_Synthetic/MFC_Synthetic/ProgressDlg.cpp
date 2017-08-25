// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"

const int PROGRESS_TIMER = 0;
const int PROGRESS_TIMER_SEG = 1;
const int PROGRESS_TIMER_SAVE = 2;

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
	SetWindowText(_T("Loading"));
	isWorkCompleted = false;
	m_ButtonOK.EnableWindow(false);
	vc_Source.open(videoFilePath);
	totalFrame = (int)vc_Source.get(CV_CAP_PROP_FRAME_COUNT);
	vc_Source.set(CV_CAP_PROP_POS_MSEC, 0);	// ���� ���������� �ʱ�ȭ

	frame = Mat(ROWS, COLS, CV_8UC3);
	bg_gray = Mat(ROWS, COLS, CV_8UC1);

	string str;
	int segmentCount;

	switch (mode){
	case PROGRESS_TIMER:	//bg init
		 // Mat(height, width, channel)
		bg = Mat(ROWS, COLS, CV_8UC3);
		
		count = 0;

		if (totalFrame < FRAMES_FOR_MAKE_BACKGROUND){
			FRAMES_FOR_MAKE_BACKGROUND = totalFrame;
		}
		m_ProgressCtrl.SetRange(0, FRAMES_FOR_MAKE_BACKGROUND - 1);

		//ù ������
		vc_Source.read(bg);

		//ù ������ �÷��� ����(�������� ��� ����)
		if (imwrite(getColorBackgroundFilePath(fileNameNoExtension), bg)){
			printf("Color Background Saved Completed\n");
		}

		cvtColor(bg, bg_gray, CV_RGB2GRAY);

		SetTimer(PROGRESS_TIMER, 0, NULL);
		break;
	case PROGRESS_TIMER_SEG:	//segmentation
		m_ProgressCtrl.SetRange(0, totalFrame-1);
		frame_g = Mat(ROWS, COLS, CV_8UC1);
		InitComponentVectorQueue(&prevHumanDetectedVector_queue);
		count = 0;
		
		// �� ��ü �������� ������ �� �� �ؽ�Ʈ ���� ����
		if ((fp = fopen(getTextFilePath(fileNameNoExtension).c_str(), "r+")) == NULL)
			fp = fopen(getTextFilePath(fileNameNoExtension).c_str(), "w+");
		fprintf(fp, to_string(videoStartMsec).append("\n").c_str());	//ù�ٿ� ������۽ð� ������
		
		SetTimer(PROGRESS_TIMER_SEG, 0, NULL);
		break;
	case PROGRESS_TIMER_SAVE:	//save
		segmentArray = new segment[BUFFER];
		segmentCount = readSegmentTxtFile(segmentArray);

		frame = imread(getColorBackgroundFilePath(fileNameNoExtension));

		
		//ť �ʱ�ȭ
		InitQueue(&segment_queue);

		//����� ��ü�� ť�� �����ϴ� �κ�
		for (int i = 0; i < segmentCount; i++) {
			// start timetag�� end timetag ���̸� enqueue
			// ���� ã�� ���߰� ��ġ�ϴ� Ÿ���±׸� ã���� ���
			if (segmentArray[i].timeTag >= obj1_TimeTag && segmentArray[i].timeTag <= obj2_TimeTag) {
				if (segmentArray[i].timeTag == segmentArray[i].msec && ((CMFC_SyntheticDlg *)GetParent())->isDirectionAndColorMatch(segmentArray[i])) {
					//����ؾ��� ��ü�� ù �������� Ÿ���±׿� ��ġ�� ť�� ����
					segmentArray[i].first_timeTagFlag = true;
					Enqueue(&segment_queue, segmentArray[i], i);
				}
			}
			//Ž�� ��, obj2_TimeTag�� ������ ���� �Ϸ�
			else if (segmentArray[i].timeTag > obj2_TimeTag) {
				break;
			}
		}

		//���Ϸ� �������� �����ϱ� ���� �غ�  
		str = "./data/";
		str.append(fileNameNoExtension).append("/").append(fileNameNoExtension).append("_").append(currentDateTime()).append(".avi");
		outputVideo.open(str, VideoWriter::fourcc('X', 'V', 'I', 'D'),25, Size((int)frame.cols, (int)frame.rows), true);

		if (!outputVideo.isOpened())
		{
			cout << "�������� �����ϱ� ���� �ʱ�ȭ �۾� �� ���� �߻�" << endl;
		}
		else {
			SetTimer(PROGRESS_TIMER_SAVE, 0, NULL);
		}
		break;
	default:
			break;
	}
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
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
				imwrite(getTempBackgroundFilePath(fileNameNoExtension), bg_gray);
				m_StaticMessage.SetWindowTextA(_T("��� ���� ����!"));
				m_ButtonOK.EnableWindow(true);
				printf("Background Init Completed\n");
				isWorkCompleted = true;
			}
			else{
				m_StaticMessage.SetWindowTextA(_T("��� ���� ����"));
				m_ButtonOK.EnableWindow(true);
				printf("Background Init Failed!!\n");
			}
		}
		break;
	case PROGRESS_TIMER_SEG:
		if (true){
			text = "("; text.append(to_string(count)).append("/").append(to_string(totalFrame-1)).append(")...���׸����̼� ���� ��");
			m_StaticMessage.SetWindowTextA(text.c_str());
			vc_Source.read(frame); //get single frame
			if (frame.empty()) {	//����ó��. �������� ����
				perror("Empty Frame");
				m_ButtonOK.EnableWindow(true);
				isWorkCompleted = true;
				KillTimer(PROGRESS_TIMER_SEG);
				OnCancel();
				break;
			}

			int curFrameCount = (int)vc_Source.get(CV_CAP_PROP_POS_FRAMES);
			int curFrameCount_nomalized = curFrameCount%FRAMECOUNT_FOR_MAKE_DYNAMIC_BACKGROUND;

			//�׷��̽����� ��ȯ
			cvtColor(frame, frame_g, CV_RGB2GRAY);

			//������ �� ����� ������ �� ���
			if (curFrameCount_nomalized >= (FRAMECOUNT_FOR_MAKE_DYNAMIC_BACKGROUND - FRAMES_FOR_MAKE_BACKGROUND)){
				if (curFrameCount_nomalized == (FRAMECOUNT_FOR_MAKE_DYNAMIC_BACKGROUND - FRAMES_FOR_MAKE_BACKGROUND)){	//���� ����� ù ��� Init
					printf("Background Making Start : %d frame\n", curFrameCount);
					bg_gray = frame_g.clone();
				}
				else{	//��� ����
					temporalMedianBG(frame_g, bg_gray);
				}
			}

			//���� ����� �����ؾ� �� ���
			if (curFrameCount_nomalized == FRAMECOUNT_FOR_MAKE_DYNAMIC_BACKGROUND - 1){
				imwrite(getTempBackgroundFilePath(fileNameNoExtension), bg_gray);
			}

			if (curFrameCount >= FRAMECOUNT_FOR_MAKE_DYNAMIC_BACKGROUND && curFrameCount_nomalized == 0){
				printf("Background Changed, %d frame\n", curFrameCount);
			}

			//���ο� ����� write�Ǳ� �� ������ base gray����� ���
			if (curFrameCount < FRAMECOUNT_FOR_MAKE_DYNAMIC_BACKGROUND - 1){
				frame_g = ExtractFg(frame_g, imread(getBackgroundFilePath(fileNameNoExtension), IMREAD_GRAYSCALE), ROWS, COLS);// ���� ����
			}
			else{	//���ο� ����� ������� �������ʹ� ������� ����� ���
				frame_g = ExtractFg(frame_g, imread(getTempBackgroundFilePath(fileNameNoExtension), IMREAD_GRAYSCALE), ROWS, COLS);// ���� ����
			}



			////TODO �պ���
			// ����ȭ
			threshold(frame_g, frame_g, 5, 255, CV_THRESH_BINARY);

			//// ������ ����
			frame_g = morphologyOpening(frame_g);
			frame_g = morphologyClosing(frame_g);
			frame_g = morphologyClosing(frame_g);
			blur(frame_g, frame_g, Size(11, 11));

			threshold(frame_g, frame_g, 5, 255, CV_THRESH_BINARY);

			// MAT������ �󺧸�
			humanDetectedVector = connectedComponentsLabelling(frame_g, ROWS, COLS, WMIN, WMAX, HMIN, HMAX);

			// ������ ó���Ͽ� ���Ϸ� �����ϱ�
			// humanDetectedVector = humanDetectedProcess(humanDetectedVector, prevHumanDetectedVector,
			//	frame, frameCount, videoStartMsec, currentMsec, fp);
			// humanDetected�� ���� ��쿡�� ����(�Լ�ȣ�� ������� ���Ҹ� ����)
			// ������ ó���Ͽ� Ÿ���±׸� ���� �ο��ϰ� ���Ϸ� �����ϱ�(2)
			if (humanDetectedVector.size() > 0)
				humanDetectedVector = humanDetectedProcess2(humanDetectedVector, prevHumanDetectedVector
				, prevHumanDetectedVector_queue, frame, count, (int)vc_Source.get(CV_CAP_PROP_POS_MSEC), fp, frame_g);

			// ť�� full�� ��� ���ڸ� ����ֱ�
			if (IsComponentVectorQueueFull(&prevHumanDetectedVector_queue))
				RemoveComponentVectorQueue(&prevHumanDetectedVector_queue);

			// ť�� �� ���ึ�� ���͸� ������ �־������
			PutComponentVectorQueue(&prevHumanDetectedVector_queue, humanDetectedVector);

			// ���� �޸� ������ �� ���� ����(prevHumanDetectedVector �޸� ����)
			vector<component>().swap(prevHumanDetectedVector);

			// ���� ������ �����͸� ���� �����Ϳ� �����ϱ�
			prevHumanDetectedVector = humanDetectedVector;

			// ���� �޸� ������ �� ���� ����
			vector<component>().swap(humanDetectedVector);

			m_ProgressCtrl.OffsetPos(1);
			count++;//increase frame count
		}
		break;
	case PROGRESS_TIMER_SAVE:
		text = "...�ռ� ���� ���� ��"; 
		m_StaticMessage.SetWindowTextA(text.c_str());
		if (true){
			Mat bg_copy = frame.clone();
	
			// �ҷ��� ����� �̿��Ͽ� �ռ��� ����
			Mat syntheticResult = ((CMFC_SyntheticDlg *)GetParent())->getSyntheticFrame(&segment_queue, bg_copy, segmentArray);
			if (segment_queue.count == 0){
				printf("���� ���� ��\n");
				text = "...�ռ� ���� ���� �Ϸ�";
				m_StaticMessage.SetWindowTextA(text.c_str());
				KillTimer(PROGRESS_TIMER_SAVE);
				m_ButtonOK.EnableWindow(true);
				isWorkCompleted = true;
				syntheticResult = NULL; syntheticResult.release();
				bg_copy = NULL; bg_copy.release();
				outputVideo.release();
				break;
			}
			else {
				outputVideo << syntheticResult;
				syntheticResult = NULL; syntheticResult.release();
				bg_copy = NULL; bg_copy.release();
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

void CProgressDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	CDialogEx::OnCancel();
	vc_Source = NULL; vc_Source.release();
	frame = NULL; frame.release();
	bg = NULL; bg.release();
	frame_g = NULL;	frame_g.release();

	vector<component>().swap(humanDetectedVector);
	vector<component>().swap(prevHumanDetectedVector);

	segmentArray = NULL;
	delete[] segmentArray;

	if (mode == 1 && fp != NULL)
		fclose(fp);

	//���������� �۾��� �Ϸ���� �ʾ��� ��� ���� ����
	if (!isWorkCompleted){
		((CMFC_SyntheticDlg *)GetParent())->OnCancel();
	}
}
