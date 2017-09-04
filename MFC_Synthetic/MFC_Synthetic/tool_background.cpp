#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>

//TemporalMedian ������� ����� ����� ����ϱ�
Mat temporalMedianBG(Mat frameimg, Mat bgimg) {
	for (int i = 0; i < bgimg.rows; i++) {
		for (int j = 0; j < bgimg.cols; j++) {
			bgimg.data[i * bgimg.cols + j] = (int)(bgimg.data[i * bgimg.cols + j] * 0.9 + frameimg.data[i * frameimg.cols + j] * 0.);
			//if (frameimg.data[i * frameimg.cols + j] > bgimg.data[i * bgimg.cols + j]) {//���� �ȼ��� ��� �ȼ����� Ŭ ��
			//	(bgimg.data[i * bgimg.cols + j] >= 255) ? bgimg.data[i * bgimg.cols + j] = 255 : bgimg.data[i * bgimg.cols + j]++;
			//}
			//else if (frameimg.data[i * frameimg.cols + j] < bgimg.data[i * bgimg.cols + j]) {//���� �ȼ��� ��� �ȼ����� ���� ��
			//	(bgimg.data[i * bgimg.cols + j] <= 0) ? bgimg.data[i * bgimg.cols + j] = 0 : bgimg.data[i * bgimg.cols + j]--;
			//}
		}
	}
	return bgimg;
}

Mat averageBG(Mat frameimg, unsigned int* bgimg) {
	for (int i = 0; i < frameimg.rows*frameimg.cols; i++) {
		bgimg[i] += frameimg.data[i];
	}
	return frameimg;
}

void setArrayToZero(unsigned int* arr,int ROWS, int COLS){
	for (int i = 0; i < ROWS*COLS; i++){
		arr[i] = 0;
	}
}

Mat accIntArrayToMat(Mat image, unsigned int* arr, int bgFrameCount){
	for (int i = 0; i < image.rows*image.cols; i++){
		image.data[i] = (int)(arr[i] / (double)bgFrameCount);
	}
	return image;
}