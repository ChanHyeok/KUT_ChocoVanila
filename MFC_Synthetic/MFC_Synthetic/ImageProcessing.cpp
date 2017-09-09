#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"


// ��ü ���� �Ǵ� �ռ��� �����ϰ� �ϱ� ����
// �̹����� ����ϰ� ó���ϱ� ���� �Լ���� �����Ǿ� ����

// �̹��� ��Ȱȭ ���� ������� �����Ͽ� ����� �� ����

// �������� ����
Mat morphologyOpening(Mat img_binary) {
	//morphological opening ���� ������ ����
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	return img_binary;
}

Mat morphologyClosing(Mat img_binary) {
	//morphological closing ������ ���� �޿��
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	return img_binary;
}

Mat pretreatmentOperator(Mat img_binary) {
	// ����ȭ
	threshold(img_binary, img_binary, 4, 255, CV_THRESH_BINARY);

	// ������ ����
	GaussianBlur(img_binary, img_binary, Size(11, 11), 0, 0, BORDER_DEFAULT);
	img_binary = morphologyOpening(img_binary);
	img_binary = morphologyClosing(img_binary);
	img_binary = morphologyClosing(img_binary);
//	blur(img_binary, img_binary, Size(11, 11));
//	GaussianBlur(img_binary, img_binary, Size(9, 9), 0, 0, BORDER_DEFAULT);
	medianBlur(img_binary, img_binary, 9);

	threshold(img_binary, img_binary, 10, 255, CV_THRESH_BINARY);

	return img_binary;
}