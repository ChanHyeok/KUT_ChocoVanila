#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>

Mat Syn_Background_Foreground(Mat background, Mat firstForegroundImage, Mat secondForegroundImage, int rows, int cols) {
	Mat result(rows, cols, CV_8UC1);
	// �ռ��� ������ �����
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) { // ���� ������ ��ġ�� �κ�(���氪�� 0�� �ƴ� �κ�)
			if (firstForegroundImage.data[i * cols + j] > 20 || secondForegroundImage.data[i * cols + j] > 20) {	//�����̹������� ���� �κ�
				result.data[i * cols + j] =
					(firstForegroundImage.data[i * cols + j] + secondForegroundImage.data[i * cols + j]) / 2;
			}
			else { // ������ �ƴ� �κ�
				result.data[i * cols + j] = background.data[i * cols + j];
			}
		}
	}
	return result;
}

Mat printObjOnBG(Mat background, segment obj, int* labelMap, string loadedFrameFileName) {
	Mat frame = loadJPGObjectFile(obj, loadedFrameFileName);

	// To do ;: loadJPGObjectFile �ٽ� �ű��
	for (int i = obj.top; i < obj.top + obj.height; i++) {
		for (int j = obj.left; j < obj.left + obj.width; j++) {
			Vec3b colorB = background.at<Vec3b>(Point(j, i));
			Vec3b colorO = frame.at<Vec3b>(Point(j - obj.left, i - obj.top));
			if (labelMap[i * background.cols + j] == 1) {	//���� ��ü�� �̹� ���� ���
				colorB[0] = (int)((colorB[0] + colorO[0]) *0.5);
				colorB[1] = (int)((colorB[1] + colorO[1]) *0.5);
				colorB[2] = (int)((colorB[2] + colorO[2]) *0.5);
				background.at<Vec3b>(Point(j, i)) = colorB;
			} //Blending
			else {
				labelMap[i * background.cols + j] = 1;		//��ü�� �׷ȴٰ� ǥ��
				colorB[0] = colorO[0];
				colorB[1] = colorO[1];
				colorB[2] = colorO[2];
				background.at<Vec3b>(Point(j, i)) = colorB;

			}
		}
	}

	frame.release();
	return background;
}