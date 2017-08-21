#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"

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
	// +10�� ����, - 10����  (RGB�̿�)  // H :: 39 -> 19.5
	if (H <= 30 && H >= 10 && R >= 130) {
		colorArray[ORANGE]++;
		hsv_flag = true;
	}

	// + 8�� ����, -�� 10 ����  (RGB�̿�) // H :: 60 -> 30
	if (H <= 38 && H >= 18 && B <= 130 && abs(R - G) < 25) {
		colorArray[YELLOW]++;
		hsv_flag = true;
	}

	// +- 6�� ����  (RGB�̿�)
	 if (H <= 62 && H >= 54 && G >= 130)  {
		colorArray[GREEN]++;
		hsv_flag = true;
	}
	// +- 22���� ����  (RGB�̿�)
	 if (H >= 98 && H <= 142 && B >= 120) {
		 colorArray[BLUE]++;
		 hsv_flag = true;
	 }

	 if (H <= 154 && H >= 146) { // H :: 300 -> 150
		 colorArray[MAGENTA]++;
		 hsv_flag = true;
	 }

	// hsv�� ���� �ʾƼ� RGB�� �̿��Ͽ� ������ �����
	if (hsv_flag == false) {
		// R > 150 && G, B < 110
		if (R >= 150 && G <= 110 && B <= 110) {
			colorArray[RED]++;
		}
		// R > 150 && 60 < GB���� < 110  &&  B < 110
		if (R >= 150 && diff_RG >= 60 && diff_RG  <= 110 && B <= 110) {
			colorArray[ORANGE]++;
		}

		// < GB���� < 40  &&  B < 90 ����� ����
		if (R >= 150 &&  G >= 150 && diff_RG <= 40 && B <= 90) {
			colorArray[YELLOW]++;
		}

		// G > 150 && R, B < 110 
		if (G >= 150 && R <= 110 && B <= 110) {
			colorArray[GREEN]++;
		}

		// B > 150 && R, G < 110 
		// blue�� ���⿡�� ����Ƽġ ���� (b 10  RG 20 ����)
		if (B >= 140 && R <= 90 && G <= 90) {
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
	if (sumOfRGB <= 65 && diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {
		colorArray[BLACK]++;
		color_point++;
	}

	// RGB�� > 380
	// if (R >= 90 && R <= 255 && G >= 90 && G <= 255 && B >= 90 && B <= 255) {
	if (sumOfRGB >= 380 && diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {
		colorArray[WHITE]++;
		color_point++;
	}

	// 20 < RGB < 50
	if (R >= 20 && R <= 50 && G >= 20 && G <= 50 && B >= 20 && B <= 50) {	// Gray���� �Ǻ�
		colorArray[GRAY]++;
		color_point++;
	}

	return color_point;
}
