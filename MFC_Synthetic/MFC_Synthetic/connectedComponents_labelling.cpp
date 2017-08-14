#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>

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
bool labelSizeFiltering(int width, int height, int MINWIDTH, int MAXWIDTH, int MINHEIGHT, int MAXHEIGHT) {
	return (width > MINWIDTH && height > MINHEIGHT
		&& width < MAXWIDTH && height < MAXHEIGHT);
}

// Component Labelling(opencv�� �Լ� connectedComponentsWithStats�� �̿��Ͽ�)
vector<component> connectedComponentsLabelling(Mat frame, int rows, int cols, int WMIN, int WMAX, int HMIN, int HMAX) {
	vector<component> result;
	result.clear();
	Rect objectRegion(0, 0, 30, 30); // ���̺� ������ �簢��
	component *componentArray = (component*)calloc(100, sizeof(component)); // componentArray�� ���� �Ҵ�

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
		if (labelSizeFiltering(width, height, WMIN, WMAX,HMIN,HMAX)) {
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

	free(componentArray);

	return result;
}