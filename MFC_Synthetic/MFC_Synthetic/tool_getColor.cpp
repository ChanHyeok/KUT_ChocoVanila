#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"

// ���� �� ����ȭ �Լ�
// opencv���� �����ϴ� H(0-180), S(0-255), V(0-255)��  ������ H(0-360),S(0-100), V(0-100) ���������� �ٲ���
int getColor_H(int h) {
	return int(h * 2);
}
int getColor_S(int s) {
	return int(s * 100 / 255);
}
int getColor_V(int v) {
	return int(v * 100 / 255);
}


//���� ������ �����ϴ� �Լ�
/*opencv HSV range
H : 180 S : 255 V : 255
*/

int colorPicker(Vec3b pixel) {
	unsigned char H = pixel[0];
	unsigned char S = pixel[1];
	unsigned char V = pixel[2];

	//Black���� White���� �Ǻ�
	if (V <= 38) {
		return BLACK;
	}
	else if (S <= 38 && V >= 166) {
		return WHITE;
	}
	else if (S <= 25) {	//Gray���� �Ǻ�
		return GRAY;
	}
	else if (H >= 165 || H <= 8) {
		return RED;
	}
	else if (H <= 22) {
		return ORANGE;
	}
	else if (H <= 37) {
		return YELLOW;
	}
	else if (H <= 85) {
		return GREEN;
	}
	else if (H <= 140) {
		return BLUE;
	}
	else if (H <= 164) {
		return MAGENTA;
	}
	else
		return-1;
}
