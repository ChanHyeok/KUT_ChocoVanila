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
