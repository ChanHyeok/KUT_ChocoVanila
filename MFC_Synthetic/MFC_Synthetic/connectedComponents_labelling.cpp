#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>

// People Area Detection Parameter
const int MINWIDTH = 89;
const int MINHEIGHT = 178;
const int MAXWIDTH = 336;
const int MAXHEIGHT = 325;


component dataAllocateAtComponent(Mat stats, component c, int indexOflables) {
	// ���� �����ϱ�
	c.area = stats.at<int>(indexOflables, CC_STAT_AREA);

	// �簢 ���� �����ϱ�
	c.left = stats.at<int>(indexOflables, CC_STAT_LEFT); // Left
	c.top = stats.at<int>(indexOflables, CC_STAT_TOP); // Top
	c.right = stats.at<int>(indexOflables, CC_STAT_LEFT) + stats.at<int>(indexOflables, CC_STAT_WIDTH); // right
	c.bottom = stats.at<int>(indexOflables, CC_STAT_TOP) + stats.at<int>(indexOflables, CC_STAT_HEIGHT); // Height

	// ���� �ʺ� �����ϱ�
	c.height = stats.at<int>(indexOflables, CC_STAT_HEIGHT);
	c.width = stats.at<int>(indexOflables, CC_STAT_WIDTH);

	return c;
}

// �簢�� ���ְ� ����� �Լ�
Rect savingRectangle(Mat frame, component c) {
	// �簢�� ȭ�鿡 ���ֱ�
	rectangle(frame, Point(c.left, c.top), Point(c.right, c.bottom), Scalar(0, 255, 0), 1);
	// �簢�� ����
	Rect objectRegion(c.left, c.top, c.width, c.height);
	return objectRegion;
}

// ���̺� ũ�⸦ ������� �����ϰ� �Ÿ��� �Լ�
int labelSizeFiltering(Mat frame, int width, int height) {
	if (width > MINWIDTH && height > MINHEIGHT
		&& width < MAXWIDTH && height < MAXHEIGHT)
		return 1; // true
	else
		return 0; // false
}

// Component Labelling(opencv�� �Լ� connectedComponentsWithStats�� �̿��Ͽ�)
vector<component> connectedComponentsLabelling(Mat frame, int rows, int cols) {
	vector<component> result;
	result.clear();
	Rect objectRegion(0, 0, 30, 30); // ���̺� ������ �簢��
	component *componentArray = (component*)calloc(999, sizeof(component)); // componentArray�� ���� �Ҵ�

	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(frame, img_labels,
		stats, centroids, 8, CV_32S); // label ���� ��ȯ

	int index = 0;
	for (int i = 1; i < numOfLables; i++) {
		// height, width�� �̸� ����
		int height = stats.at<int>(i, CC_STAT_HEIGHT);
		int width = stats.at<int>(i, CC_STAT_WIDTH);
		//printf("%d %d %d\n", i, width, height);
		// �����ڽ� �׸���, ���̺� ũ�⸦ ���͸� �Ͽ�(���ũ�⿡ �ش�� ��ŭ)
		if (labelSizeFiltering(frame, width, height)) {
			// ��ȿ�� ���̺� �ε����� ����
			componentArray[index].label = index;

			// Component�� ������ ����
			componentArray[index] = dataAllocateAtComponent(stats, componentArray[index], i);
			// RectŸ�� ������ ���̺�� ������Ʈ ����
			objectRegion = savingRectangle(frame, componentArray[index]);
			result.push_back(componentArray[index]);

			index++;
		}
	}

	return result;
}