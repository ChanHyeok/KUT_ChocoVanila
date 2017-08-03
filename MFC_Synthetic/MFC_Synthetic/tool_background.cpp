#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>

//TemporalMedian ������� ����� ����� ����ϱ�
Mat temporalMedianBG(Mat frameimg, Mat bgimg, int rows, int cols) {
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

	return bgimg;
}
