#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <direct.h>
#include <io.h>

// ���� ó���� ���õ� ��� �Լ����� �����մϴ�.
// segment�� fileName�Ҵ�, JPG ���� ����, txt���� ����
void saveSegmentation_JPG(component object, Mat frame, string video_path);
void saveSegmentation_TXT(component object, FILE *fp);
string allocatingComponentFilename(int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector);

// segment ���� �ȿ� Segmentation�� Obj���� jpg���Ϸ� �����ϴ� �Լ�
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS);

// Video Path���� file�̸��� ���� Ȯ���ڸ� �����Ͽ� ��ȯ�ϴ� �Լ�
String getFileName(CString f_path, char find_char, BOOL extension) {
	// ������ \ ���� ���ڿ�
	// �˻���� :: "Video (*.avi, *.MP4) | *.avi;*.mp4; | All Files(*.*)|*.*||",
	string f_name;
	char final_index;
	for (int i = 0; i < f_path.GetLength(); i++) {
		if (f_path[i] == find_char)
			final_index = i;
	} // '\'�� ã��

	for (int i = final_index + 1; i < f_path.GetLength(); i++) {
		if (f_path[final_index + 1] == NULL) break; // ����ó��
		if (extension == false){
			if (f_path[i] == '.') break; // Ȯ���� ���� (. ���Ŀ� ���ڴ� ����)
			// ������ ����µ� Ȯ���ڰ� ������ Ȯ���ڿ� �´� ������ �����Ǵ� ������ �־ �ϴ� ��
			// Ȥ�ó� Ȯ���ڰ� �ʿ��� ��� �Լ��� �ϳ� �� �����ϴ��� �ؾ��� ��
		}
		char c = f_path[i];
		f_name += c;
	}// �������� ������ '\' ������ ���ڿ��� ������

	return f_name;
}

// ��ü segment �������� ���ϵ��� �����ϴ� ���
bool saveSegmentationData(string video_name, component object, Mat object_frame
	, int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector, FILE *txt_fp) {

	// object�� �����̸� �Ҵ�
	object.fileName = allocatingComponentFilename(timeTag, currentMsec, frameCount, indexOfhumanDetectedVector);

	// jpg���Ϸ� ����
	saveSegmentation_JPG(object, object_frame, getObjDirectoryPath(video_name));

	// txt���Ϸ� ����
	saveSegmentation_TXT(object, txt_fp);

	return true;
}

void saveSegmentation_JPG(component object, Mat frame, string video_path) {
	string fullPath;
	// ������ �̹����� �޾ƿ�
	Mat img = objectCutting(object, frame, frame.rows, frame.cols);

	// ���� �� ��θ� ���� ���� �̸��� ����
	stringstream ss;
	ss << video_path << "/" << object.fileName << ".jpg";
	fullPath = ss.str();
	ss.str("");

	// �����ϸ鼭 ���� �簢���� �׸��� �Լ�
	rectangle(img, Point(0, 0),
		Point(object.right - object.left - 1, object.bottom - object.top - 1),
		Scalar(0, 0, 255), 2);

	// �ش� �̹����� ���� �����̸��� ���� jpg���Ϸ� ����
	int check_writeFullPath = imwrite(fullPath, img);
}

// Segmentation�� Obj�� Data�� txt���Ϸ� �����ϴ� �Լ�
// format : FILE_NAME(sec_frameCount) X Y WIDTH HEIGHT
void saveSegmentation_TXT(component object, FILE *fp) {
	string info;
	stringstream ss;
	ss << object.fileName << " " << object.left << " " << object.top << " " << object.right << " " << object.bottom
		<< " " << object.right - object.left << " " << object.bottom - object.top << '\n';
	info = ss.str();
	fprintf(fp, info.c_str());
}

// jpg�� ����� object�� ��ȯ�� �ִ� �Լ�
Mat loadJPGObjectFile(segment obj, string file_name) {
	stringstream ss;
	String fullPath;

	// segment_�����̸� ���� ���� ���� ��ü �̸� �������� stringstream�� ������
	ss << getObjDirectoryPath(file_name) << "/" << obj.fileName.c_str();
	fullPath = ss.str();
	ss.str("");
	Mat frame = imread(fullPath);
	return frame;
}

// ROI������ �����ϴ� �Լ�
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS) {
	return img(Rect(object.left, object.top, object.right - object.left, object.bottom - object.top)).clone();
	//�߸� �̹��� ��ȯ
}

// �ؽ�Ʈ ����(���׸�Ʈ ������ �����) �̸��� ��ȯ�ϴ� �Լ�
string getTextFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name 
		+ "/" + RESULT_TEXT_FILENAME + video_name + (".txt");
}

// �ؽ�Ʈ ����(��ġ ������ ����Ǵ�) �̸��� ��ȯ�ϴ� �Լ�
string getDetailTextFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name
		+ "/" + RESULT_TEXT_DETAIL_FILENAME + video_name + (".txt");
}

// ��� ���� �̸��� ��ȯ�ϴ� �Լ�
string getBackgroundFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name 
		+ "/" + RESULT_BACKGROUND_FILENAME + video_name + (".jpg");
}

// ���׸�Ʈ���� �������� �������� ����� �����̸��� ��ȯ�ϴ� �Լ� , ���� �� ��� :: /data/(���� �̸�)
string getDirectoryPath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name;
}

// ���׸�Ʈ(object)���� ����� �����̸��� ��ȯ�ϴ� �Լ� , ���� �� ��� :: /data/(���� �̸�)/obj
string getObjDirectoryPath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name + "/" + "obj";
}

// ������Ʈ ���� �ش� ���丮�� �ִ� �� üũ�ϴ� �Լ�
bool isDirectory(string dir_name) {
	// string type�� const char* �� �ٲٴ� ����
	std::vector<char> writable(dir_name.begin(), dir_name.end());
	writable.push_back('\0');
	char *ptr_name = &writable[0];

	// ������ ���� ��쿡�� _access(ptr, 0) ���� 0�� ��ȯ�Ͽ� true, �׷��� ������ false.
	return _access(ptr_name, 0) == 0;
}

// data ������ �����ϴ� �Լ�
int makeDataRootDirectory() {
	// ���׸����̼� ��� �����Ͱ� ����ִ� ������ ���� ��� ����� ��
	return _mkdir(SEGMENTATION_DATA_DIRECTORY_NAME.c_str());
}

// data ���� �ȿ� �ش� ������ ��� �� �̸��� ���� ���丮�� �����ϴ� �Լ�
int makeDataSubDirectory(string video_path) {
	// �ش� ������ ���׸����̼� ����� �� ������ο� ���� ���� ����(/data/(video_name))
	return _mkdir(video_path.c_str());
}

// ������ �̸��κ��� ����
string allocatingComponentFilename(int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector) {
	string name;
	return name.append(to_string(timeTag)).append("_")
		.append(to_string(currentMsec)).append("_")
		.append(to_string(frameCount)).append("_")
		.append(to_string(indexOfhumanDetectedVector));
}