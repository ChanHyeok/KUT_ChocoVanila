#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\xphoto\white_balance.hpp>

int* getColorData(Mat frame, component *object, Mat binary, Mat bg, int frameCount, int currentMsec) {
	Mat temp = frame.clone();
	int *colorArray = new int[COLORS];
	for (int i = 0; i < COLORS; i++)
		colorArray[i] = 0;

	// �� �׻��� ����� ���� �˰��� ����
	cv::xphoto::createGrayworldWB()->balanceWhite(temp, temp);

	//���� ������ ���� RGB, HSV�� ��ȯ�ϱ�
	Mat frame_hsv, frame_rgb;
	cvtColor(temp, frame_hsv, CV_BGR2HSV);
	cvtColor(temp, frame_rgb, CV_BGR2RGB);

	int sum_of_color_array[6] = { 0 , };

	// �� �����ӿ��� ��ȿ�� color�� �����ϴ� ������ �ϴ� Ƚ��, component�� color_count�� ����
	int temp_color_count = 0;
	for (int i = object->top; i < object->bottom; i++) {
		Vec3b* ptr_temp = temp.ptr<Vec3b>(i);
		Vec3b* ptr_color_hsv = frame_hsv.ptr<Vec3b>(i);
		Vec3b* ptr_color_rgb = frame_rgb.ptr<Vec3b>(i);

		for (int j = object->left + 1; j < object->right; j++) {
			// ���� ������ ����
			if (isColorDataOperation(frame, bg, binary, i, j)) {
				temp_color_count++;
				int color_check = colorPicker(ptr_color_hsv[j], ptr_color_rgb[j], colorArray);

				// �� component�� color ����� ���ϱ� ���� �ӽ� �迭�� ���� ����
				for (int c = 0; c < 3; c++) {
					sum_of_color_array[c] += ptr_color_hsv[j][c];
					sum_of_color_array[c + 3] += ptr_color_rgb[j][c];
				}
			}
			else {
				ptr_temp[j] = Vec3b(0, 0, 0);
			}

		}
	}

	// ��ä��, ��ä���� �뷱���� ���߱� ���� ����, white�� black�� weight ����
	colorArray[BLACK] *= 0.63;
	colorArray[WHITE] *= 0.77;

	// blue�� �뷱�� �����ֱ�
	colorArray[BLUE] *= 0.93;

	// object�� �� ����(hsv, rgb) ��� ��ҿ� �� ���� ī��Ʈ�� ������ ����,
	for (int c = 0; c < 3; c++) {
		object->hsv_avarage[c] = 0;
		object->rgb_avarage[c] = 0;
		if (sum_of_color_array[c] > 0 && sum_of_color_array[c + 3] > 0) {
			object->hsv_avarage[c] = sum_of_color_array[c] / object->area;
			object->rgb_avarage[c] = sum_of_color_array[c + 3] / object->area;
		}
	}
	object->color_count = temp_color_count;

	// color�� ���� obj�� jpg���Ϸ� ����
	// ���� ����
	component temp_object = *object;
	temp_object.fileName = allocatingComponentFilename(temp_object.timeTag, currentMsec, frameCount, temp_object.label);
	//	saveSegmentation_JPG(temp_object, temp, getObj_for_colorDirectoryPath(fileNameNoExtension));

		// Ȯ�� �ڵ�
		/*
		printf("timatag = %d) [", object.timeTag);
		for (int i = 0; i < 6; i++) {
		double color_value = (double)temp_color_array[i] / (double)get_color_data_count;
		printf("%.0lf ", color_value);
		}
		printf("] rate = %.2lf \n", rate_of_color_operation);
		*/

		//printf("%10d : ", object.timeTag);
		//for (int i = 0; i < COLORS;i++)
		//	printf("%d ",colorArray[i]);
		//printf("\n");

	temp = NULL;
	temp.release();
	return colorArray;
}

//���� ������ �����ϴ� �Լ�
/*opencv HSV range
H : 180 S : 255 V : 255
*/

int colorPicker(Vec3b pixel_hsv, Vec3b pixel_rgb, int *colorArray) {
	// ����� ������ ���� (0�� ��� ����)
	int color_point = 1;

	// HSV, RGB �� ���� �Ҵ��ϱ�
	unsigned char H = pixel_hsv[0];
	unsigned char S = pixel_hsv[1];
	unsigned char V = pixel_hsv[2];

	unsigned char R = pixel_rgb[0];
	unsigned char G = pixel_rgb[1];
	unsigned char B = pixel_rgb[2];

	// RGB�� ��
	int sumOfRGB = R + G + B;
	int diff_RG = abs(R - G);
	int diff_GB = abs(G - B);
	int diff_BR = abs(B - R);
	bool hsv_flag = false;

	// �������� ���� ���� +- ����
	// HSV ä�η� ����� ������ ������ �����

	// +- 3�� ���� (RGB�̿�)  //  H :: 0 -> 0
	if ((H >= 177 && H >= 180) || (H >= 0 && H <= 3) && R >= 130) {
		colorArray[RED]++;
		hsv_flag = true;
	}
	// +8�� ����, - 8����  (RGB�̿�)  // H :: 30 -> 15
	if (H <= 7 && H >= 23 && R >= 130) {
		colorArray[ORANGE]++;
		hsv_flag = true;
	}

	// + 8�� ����, -�� 10 ����  (RGB�̿�) // H :: 60 -> 30
	if (H <= 38 && H >= 20 && B <= 130 && abs(R - G) < 25) {
		colorArray[YELLOW]++;
		hsv_flag = true;
	}

	// +15 - 10�� ����  (RGB�̿�) // H :: 120 -> 60
	if (H <= 50 && H >= 75 && G >= 130) {
		colorArray[GREEN]++;
		hsv_flag = true;
	}
	// + 10, -20���� ����  (RGB�̿�)  // H :: 240 -> 120
	if (H >= 100 && H <= 130 && B >= 140) {
		colorArray[BLUE]++;
		hsv_flag = true;
	}

	// +- 6���� ����
	if (H <= 156 && H >= 144) { // H :: 300 -> 150
		colorArray[MAGENTA]++;
		hsv_flag = true;
	}

	// hsv�� ���� �ʾƼ� RGB�� �̿��Ͽ� ������ �����
	if (hsv_flag == false) {
		// R > 150 && G, B < 110
		if (R >= 150 && G <= 110 && B <= 110 
			|| (R >= 100 && diff_RG <= 40 && diff_BR <= 40 && G <= 50 && B <= 50)) {
			colorArray[RED]++;
		}
		// R > 150 && 60 < GB���� < 110  &&  B < 110
		if (R >= 150 && diff_RG >= 60 && diff_RG <= 110 && B <= 110) {
			colorArray[ORANGE]++;
		}

		// < GB���� < 40  &&  B < 90 ����� ����
		if (R >= 150 && G >= 150 && diff_RG <= 40 && B <= 90) {
			colorArray[YELLOW]++;
		}

		// G > 150 && R, B < 110 
		if (G >= 150 && R <= 110 && B <= 110
			|| (G >= 100 && diff_RG <= 40 && diff_GB <= 40 && R <= 50 && B <= 50)) {
			colorArray[GREEN]++;
		}

		// B > 160 && R, G < 100 
		if ((B >= 150 && R <= 100 && G <= 100)
			|| (B >= 100 && diff_BR <= 40 && diff_GB <= 40 && R <= 50 && G <= 50)) {
			colorArray[BLUE]++;
		}

		// R > 150 && B > 150 && G < 110 && BR���� < 60)
		if (R >= 150 && B >= 150 && diff_BR <= 60 && G <= 110) { // H :: 300 -> 150
			colorArray[MAGENTA]++;
		}
	}


	// RGB�� �̿��Ͽ� ������ �� �����(Black, Gray, White)

	// RGB�� < 65
	// if (R >= 0 && R <= 20 && G >= 0 && G <= 20 && B >= 0 && B <= 20) {
	if (sumOfRGB <= 55 && diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {
		colorArray[BLACK]++;
		color_point++;
	}

	// RGB�� > 380
	// if (R >= 90 && R <= 255 && G >= 90 && G <= 255 && B >= 90 && B <= 255) {
	if (sumOfRGB >= 400 && diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {
		colorArray[WHITE]++;
		color_point++;
	}

	// 25 < RGB < 50
	if (R >= 25 && R <= 50 && G >= 25 && G <= 50 && B >= 25 && B <= 50
		&& diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {	// Gray���� �Ǻ�
		colorArray[GRAY]++;
		color_point++;
	}

	return color_point;
}
