#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
//#include <opencv2\xphoto\white_balance.hpp>

// 색 항상성
Mat grayWorld(Mat frame) {
	cv::Scalar sumImg = sum(frame);
	cv::Scalar illum = sumImg / (frame.rows*frame.cols);
	std::vector<cv::Mat> rgbChannels(3);
	cv::split(frame, rgbChannels);
	cv::Mat redImg = rgbChannels[2];
	cv::Mat graanImg = rgbChannels[1];
	cv::Mat blueImg = rgbChannels[0];
	double scale = (illum(0) + illum(1) + illum(2)) / 3;
	redImg = redImg*scale / illum(2);
	graanImg = graanImg*scale / illum(1);
	blueImg = blueImg*scale / illum(0);
	rgbChannels[0] = blueImg;
	rgbChannels[1] = graanImg;
	rgbChannels[2] = redImg;
	merge(rgbChannels, frame);
	return frame;
}

int* getColorData(Mat frame, component *object, Mat binary, Mat bg, int frameCount, int currentMsec) {
	Mat temp = frame.clone();
	int *colorArray = new int[COLORS];
	for (int i = 0; i < COLORS; i++)
		colorArray[i] = 0;

	// 색 항상성을 고려한 보간 알고리즘 적용
	// cv::xphoto::createGrayworldWB()->balanceWhite(temp, temp);
	temp = grayWorld(temp);

	//원본 프레임 각각 RGB, HSV로 변환하기
	Mat frame_hsv, frame_rgb;
	cvtColor(temp, frame_hsv, CV_BGR2HSV);
	cvtColor(temp, frame_rgb, CV_BGR2RGB);

	int sum_of_color_array[6] = { 0 , };

	// 한 프레임에서 유효한 color을 추출하는 연산을 하는 횟수, component의 color_count에 저장
	int temp_color_count = 0;
	for (int i = object->top; i < object->bottom; i++) {
		Vec3b* ptr_temp = temp.ptr<Vec3b>(i);
		Vec3b* ptr_color_hsv = frame_hsv.ptr<Vec3b>(i);
		Vec3b* ptr_color_rgb = frame_rgb.ptr<Vec3b>(i);

		for (int j = object->left + 1; j < object->right; j++) {
			// 색상 데이터 저장
			if (isColorDataOperation(frame, bg, binary, i, j)) {
				temp_color_count++;
				int color_check = colorPicker(ptr_color_hsv[j], ptr_color_rgb[j], colorArray);

				// 한 component의 color 평균을 구하기 위해 임시 배열에 합을 구함
				for (int c = 0; c < 3; c++) {
					sum_of_color_array[c] += ptr_color_hsv[j][c];
					sum_of_color_array[c + 3] += ptr_color_rgb[j][c];
				}
			}
		}
	}

	// 무채색, 유채색의 밸런스를 맞추기 위한 연산, white와 black의 weight 조절
	colorArray[BLACK] *= 0.9;
	colorArray[WHITE] *= 0.95;

	// blue의 밸런스 맞춰주기
	colorArray[BLUE] *= 0.95;

	// object의 색 영역(hsv, rgb) 평균 요소와 색 최종 카운트에 데이터 삽입,
	for (int c = 0; c < 3; c++) {
		object->hsv_avarage[c] = 0;
		object->rgb_avarage[c] = 0;
		if (sum_of_color_array[c] > 0 && sum_of_color_array[c + 3] > 0) {
			object->hsv_avarage[c] = sum_of_color_array[c] / object->area;
			object->rgb_avarage[c] = sum_of_color_array[c + 3] / object->area;
		}
	}
	object->color_count = temp_color_count;
	// 확인 코드
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

//색상 정보를 검출하는 함수
/*opencv HSV range
H : 180 S : 255 V : 255
*/

int colorPicker(Vec3b pixel_hsv, Vec3b pixel_rgb, int *colorArray) {
	// 검출된 색깔의 갯수 (0일 경우 에러)
	int color_point = 1;

	// HSV, RGB 값 각각 할당하기
	unsigned char H = pixel_hsv[0];
	unsigned char S = pixel_hsv[1];
	unsigned char V = pixel_hsv[2];

	unsigned char R = pixel_rgb[0];
	unsigned char G = pixel_rgb[1];
	unsigned char B = pixel_rgb[2];

	// RGB의 합
	int sumOfRGB = R + G + B;
	int diff_RG = abs(R - G);
	int diff_GB = abs(G - B);
	int diff_BR = abs(B - R);
	bool hsv_flag = false;
	bool blue_flag = false;

	// 원색에서 차이 범위 +- 조정
	// HSV 채널로 충분히 검출이 가능한 색상들

	// +- 3로 감소 (RGB이용)  //  H :: 0 -> 0
	if ((H >= 177 && H >= 180) || (H >= 0 && H <= 3) && R >= 130) {
		colorArray[RED]++;
		hsv_flag = true;
	}
	// +10로 증가, - 10증가  (RGB이용)  // H :: 30 -> 15
	if (H <= 5 && H >= 25 /* && R >= 130 */ ) {
		colorArray[ORANGE]++;
		hsv_flag = true;
	}

	// + 10로 증가, -로 10 증가  (RGB이용) // H :: 60 -> 30
	if (H <= 40 && H >= 20 && B <= 130 /*&& abs(R - G) < 25*/) {
		colorArray[YELLOW]++;
		hsv_flag = true;
	}

	// +20 - 15로 증가  (RGB이용) // H :: 120 -> 60
	if (H <= 45 && H >= 80/* && G >= 130*/) {
		colorArray[GREEN]++;
		hsv_flag = true;
	}
	// + 12, -20으로 증가  (RGB이용)  // H :: 240 -> 120
	if (H >= 100 && H <= 132 && B >= 130) {
		colorArray[BLUE]++;
		blue_flag = true;
	}

	// +- 6으로 증가
	if (H <= 156 && H >= 144) { // H :: 300 -> 150
		colorArray[MAGENTA]++;
		hsv_flag = true;
	}

	// hsv로 쉽지 않아서 RGB를 이용하여 검출한 색상들
	if (hsv_flag == false) {
		// R > 150 && G, B < 110
		if (R >= 150 && G <= 110 && B <= 110 
			|| (R >= 100 && diff_RG >= 40 && diff_BR >= 40 && G <= 50 && B <= 50)) {
			colorArray[RED]++;
		}
		// R > 150 && 60 < GB차이 < 110  &&  B < 110
		if (R >= 150 && diff_RG >= 60 && diff_RG <= 110 && B <= 110) {
			colorArray[ORANGE]++;
		}

		// < GB차이 < 40  &&  B < 90 노란색 조정
		if (R >= 150 && G >= 150 && diff_RG <= 40 && B <= 80) {
			colorArray[YELLOW]++;
		}

		// G > 150 && R, B < 110 
		if (G >= 150 && R <= 110 && B <= 110
			|| (G >= 80 && diff_RG >= 40 && diff_GB >= 40 && R <= 50 && B <= 50)) {
			blue_flag == true;
			colorArray[GREEN]++;
		}

		if (B >= 150 && R <= 110 && G <= 110
			|| (B >= 90 && diff_GB >= 40 && diff_BR >= 40 && R <= 50 && G <= 50)) {
			colorArray[BLUE]++;
		}
		
		// R > 150 && B > 150 && G < 110 && BR차이 < 60)
		if (R >= 150 && B >= 150 && diff_BR <= 60 && G <= 110) { // H :: 300 -> 150
			colorArray[MAGENTA]++;
		}
	}


	// RGB를 이용하여 검출을 할 색상들(Black, Gray, White)
	// black, white가 검출된 경우 gray 검출 x
	bool isGrayEnable = false;
	bool isWhite = false;
	bool isBlack = false;

	// RGB합 < 70
	// if (R >= 0 && R <= 20 && G >= 0 && G <= 20 && B >= 0 && B <= 20) {
	if (sumOfRGB <= 70 && diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {
		isGrayEnable = true;
		isBlack = true;
		colorArray[BLACK]++;
		color_point++;
	}

	// RGB합 > 435
	// if (R >= 90 && R <= 255 && G >= 90 && G <= 255 && B >= 90 && B <= 255) {
	if (sumOfRGB >= 435 && diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {
		isGrayEnable = true;
		isWhite = true;
		colorArray[WHITE]++;
		color_point++;
	}

	// 25 < RGB < 50
	if ((isGrayEnable == false) && R >= 25 && R <= 50 && G >= 25 && G <= 50 && B >= 25 && B <= 50
		&& diff_RG < 15 && diff_GB < 15 && diff_BR < 15) {	// Gray인지 판별
		colorArray[GRAY]++;
		color_point++;
	}

	// black, white 검출 시 HSV 영역에 blue 삭제
	if (((isWhite == true) || (isBlack == true)) && (blue_flag == true) && H >= 107 && H <= 127)
		colorArray[BLUE]--;

	return color_point;
}