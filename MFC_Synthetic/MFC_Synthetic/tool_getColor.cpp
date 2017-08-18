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

	// �������� ���� ���� +- ����
	// HSV ä�η� ����� ������ ������ �����

	// +- 2�� ����
	if ( ( H >= 179 && H >= 180)|| (H >= 0 && H <= 1) )  //  H :: 0 -> 0
		colorArray[RED]++;

	// +- 6�� ����
	if (H <= 26 && H >= 14)  // H :: 39 -> 19.5
		colorArray[ORANGE]++;

	// + 10�� ����, -�� 12 ����
	if (H <= 40 && H >= 18)  // H :: 60 -> 30
		colorArray[YELLOW]++;

	// +- 6�� ����
	if (H <= 62 && H >= 54)  // H :: 120 -> 60
		colorArray[GREEN]++;

	// +- 24���� ����
	if (H >= 96 && H <= 144)  // H :: 240 -> 120
		colorArray[BLUE]++;

	//
	if (H <= 154 && H >= 146) // H :: 300 -> 150
		colorArray[MAGENTA]++;

	// RGB�� �̿��Ͽ� ������ �� �����(Black, Gray, White)
	// RGB < 20
	if (R >= 0 && R <= 20 && G >= 0 && G <= 20 && B >= 0 && B <= 20) {
		colorArray[BLACK]++;
		color_point++;
	}

	// RGB > 90
	if (R >= 90 && R <= 255 && G >= 90 && G <= 255 && B >= 90 && B <= 255) {
		colorArray[WHITE]++;
		color_point++;
	}

	// 25 < RGB < 45
	if (R >= 25 && R <= 45 && G >= 25 && G <= 45 && B >= 25 && B <= 45) {	// Gray���� �Ǻ�
		colorArray[GRAY]++;
		color_point++;
	}

	return color_point;
}
