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
	
	// �������� ���� ���� +- ����
	// HSV ä�η� ����� ������ ������ �����

	// +- 3�� ���� (RGB�̿�)
	 if ( ( H >= 177 && H >= 180)|| (H >= 0 && H <= 3) && R >= 130)
	//if ( ( H >= 179 && H >= 180)|| (H >= 0 && H <= 1) )  //  H :: 0 -> 0
		colorArray[RED]++;

	// +10�� ����, - 10����  (RGB�̿�)
	if (H <= 30 && H >= 10 && R >= 150)
	// if (H <= 24 && H >= 16)  // H :: 39 -> 19.5
		colorArray[ORANGE]++;

	// + 10�� ����, -�� 16 ����  (RGB�̿�)
	 if (H <= 40 && H >= 14 && B <= 150 && abs(R-G) < 30)
	//if (H <= 40 && H >= 18)  // H :: 60 -> 30
		colorArray[YELLOW]++;

	// +- 6�� ����  (RGB�̿�)
	 	if (H <= 62 && H >= 54 && G >= 110) 
	//if (H <= 62 && H >= 54)  // H :: 120 -> 60
		colorArray[GREEN]++;

	// +- 19���� ����  (RGB�̿�)
	 if (H >= 101 && H <= 139 && B >= 130)
	//if (H >= 102 && H <= 138)  // H :: 240 -> 120
		colorArray[BLUE]++;

	//
	if (H <= 154 && H >= 146) // H :: 300 -> 150
		colorArray[MAGENTA]++;

	// RGB�� �̿��Ͽ� ������ �� �����(Black, Gray, White)
	
	// RGB�� < 65
	// if (R >= 0 && R <= 20 && G >= 0 && G <= 20 && B >= 0 && B <= 20) {
	if (sumOfRGB <= 65 && abs(R-B) < 15 && abs(B - G) < 13 && abs(G - R) < 13) {
		colorArray[BLACK]++;
		color_point++;
	}

	// RGB�� > 380
	// if (R >= 90 && R <= 255 && G >= 90 && G <= 255 && B >= 90 && B <= 255) {
	if (sumOfRGB >= 380 && abs(R - B) < 15 && abs(B - G) < 13 && abs(G - R) < 13) {
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
