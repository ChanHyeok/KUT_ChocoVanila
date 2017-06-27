#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"


// ��ü ���� �Ǵ� �ռ��� �����ϰ� �ϱ� ����
// �̹����� ����ϰ� ó���ϱ� ���� �Լ���� �����Ǿ� ����

// �̹��� ��Ȱȭ ���� ������� �����Ͽ� ����� �� ����

// �������� ����
Mat morphologicalOperation(Mat img_binary) {
	//morphological opening ���� ������ ����
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//morphological closing ������ ���� �޿��
	dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	return img_binary;
}

Mat BlendingOperation(Mat background, Mat frame) {
	double alpha = 0.5, beta;
	beta = 1.0 - alpha;
	addWeighted(frame, alpha, background, beta, 0.0, background);

	// ��ħ�� ���¿� ���� ������ ������ ����(alpha�� beta���� �̿��Ͽ�)


	return background;


}

