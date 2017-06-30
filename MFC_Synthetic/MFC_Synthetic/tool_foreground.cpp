#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>
// openCV �����Լ� MOG2�� �̿��Ͽ� background�� �и��س��� �Լ�
Mat ExtractForegroundToMOG2(Mat frameimg) {
	Mat result_frame;
	Ptr<BackgroundSubtractor> pMOG2;

	pMOG2 = createBackgroundSubtractorMOG2();
	pMOG2->apply(frameimg, result_frame);

	return result_frame;
}

Mat ExtractFg(Mat frameimg, Mat bgimg, int rows, int cols) {
	//   Mat fgimg(col, row, CV_8UC3);
	//   Mat fgimg(row, col, CV_8UC3);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (abs(frameimg.data[i * bgimg.cols + j] - bgimg.data[i * bgimg.cols + j]) < 20)
				frameimg.data[i * bgimg.cols + j] = 0;

		}
	}
	return frameimg;
}
