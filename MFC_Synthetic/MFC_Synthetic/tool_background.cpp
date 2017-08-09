#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>

//TemporalMedian ������� ����� ����� ����ϱ�
int temporalMedianBG(Mat frameimg, Mat bgimg, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (frameimg.data[i * frameimg.cols + j] > bgimg.data[i * bgimg.cols + j]) {//���� �ȼ��� ��� �ȼ����� Ŭ ��
				(bgimg.data[i * bgimg.cols + j] >= 255) ? 0 : bgimg.data[i * bgimg.cols + j]++;
			}
			else if (frameimg.data[i * frameimg.cols + j] < bgimg.data[i * bgimg.cols + j]) {//���� �ȼ��� ��� �ȼ����� ���� ��
				(bgimg.data[i * bgimg.cols + j] <= 0) ? 0 : bgimg.data[i * bgimg.cols + j]--;
			}
		}
	}
	return 0;
}
