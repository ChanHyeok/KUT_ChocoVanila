#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <io.h>

stringstream timeConvertor(int t);
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS);
// segment ���� �ȿ� Segmentation�� Obj���� jpg���Ϸ� �����ϴ� �Լ�
// format : msec_frameCount_index 
void saveSegmentation_JPG(component object, Mat frame, int frameCount, int msec, int index, unsigned int videoStartMsec) {
	Mat img = objectCutting(object, frame, frame.rows, frame.cols);

	string folderName = "segment";
	string fileName = object.fileName;
	string fullPath;
	stringstream ss;

	//Ÿ���±׸� string���� ��ȯ
	string timetag = "";
	int timetagInSec = (object.timeTag + videoStartMsec) / 1000;	//������ ���۽ð��� �����ش�.
	ss = timeConvertor(timetagInSec);
	timetag = ss.str();
	ss.str("");

	//Ŀ�õ� �̹����� Ÿ���±׸� �޾��ش�
	//params : (Mat, String to show, ����� ��ġ, ��Ʈ Ÿ��, ��Ʈ ũ��, ����, ����) 
	putText(img, timetag, Point(5, object.bottom - object.top - 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 150, 150), 2);

	//���� �簢���� �׸��� �Լ�
	rectangle(img, Point(0, 0),
		Point(object.right - object.left - 1, object.bottom - object.top - 1),
		Scalar(0, 0, 255), 2);

	if (access("segment", 0) != 0) {	//������ ���� ���
		string folderCreateCommand = "mkdir " + folderName;
		system(folderCreateCommand.c_str());
	}

	// ������ jpg������ ������
	ss << folderName << "/" << fileName << ".jpg";
	fullPath = ss.str();
	ss.str("");

	imwrite(fullPath, img);
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

	//printf("%02d:%02d:%02d \n", hour, min, sec);

	return s;
}

// Segmentation�� Obj�� Data�� txt���Ϸ� �����ϴ� �Լ�
// format : FILE_NAME(sec_frameCount) X Y WIDTH HEIGHT 
void saveSegmentation_TXT(component object, int frameCount, int msec, FILE *fp, int index) {
	string info;
	stringstream ss;
	ss << object.fileName << " " << object.left << " " << object.top << " " << object.right << " " << object.bottom << " " << object.right - object.left << " " << object.bottom - object.top << '\n';
	info = ss.str();
	fprintf(fp, info.c_str());
}

// ROI������ �����ϴ� �Լ�
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS) {
	return img(Rect(object.left, object.top, object.right - object.left, object.bottom - object.top)).clone();
	//�߸� �̹��� ��ȯ
}