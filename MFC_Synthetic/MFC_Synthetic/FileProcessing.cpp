#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <io.h>

// ���� ó���� ���õ� ��� �Լ����� �����մϴ�.
// JPG ���� ����, txt���� ����

// segment ���� �ȿ� Segmentation�� Obj���� jpg���Ϸ� �����ϴ� �Լ�
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS);

// Video Path���� file�̸��� ���� ��ȯ�ϴ� �Լ� 
String getFileName(CString f_path, char find_char) {
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
		char c = f_path[i];
		f_name += c;
	}// �������� ������ '\' ������ ���ڿ��� ������

	return f_name;
}

void saveSegmentation_JPG(component object,
	Mat frame, int frameCount, int msec, int index, unsigned int videoStartMsec, string video_fname) {
	Mat img = objectCutting(object, frame, frame.rows, frame.cols);

	std::string folderName = RESULT_FOLDER_NAME;
	string fileName = object.fileName;
	string fullPath;
	stringstream ss;

	// folderName :: segment_������ ���� ���� �̸�(ex. segment_xxx.mp4)
	folderName.append(video_fname);

	//���� �簢���� �׸��� �Լ�
	rectangle(img, Point(0, 0),
		Point(object.right - object.left - 1, object.bottom - object.top - 1),
		Scalar(0, 0, 255), 2);

	if (!isDirectory(folderName, fileName)) {	//������ ���� ���
		string folderCreateCommand = "mkdir " + folderName;
		system(folderCreateCommand.c_str());
	}

	// ������ jpg������ ������
	ss << folderName << "/" << fileName << ".jpg";
	fullPath = ss.str();
	ss.str("");

	int check_writeFullPath = imwrite(fullPath, img);
}
// Segmentation�� Obj�� Data�� txt���Ϸ� �����ϴ� �Լ�
// format : FILE_NAME(sec_frameCount) X Y WIDTH HEIGHT 
void saveSegmentation_TXT(component object, int frameCount, int msec, FILE *fp, int index) {
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
	ss << RESULT_FOLDER_NAME << file_name << "/" << obj.fileName.c_str();
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
string getTextFileName(string video_name) {
	return RESULT_TEXT_FILENAME + video_name + (".txt");
}

// ��� ���� �̸��� ��ȯ�ϴ� �Լ�
string getBackgroundFilename(string video_name) {
	return RESULT_BACKGROUND_FILENAME + video_name + (".jpg");
}

// ���׸�Ʈ���� ����� �����̸��� ��ȯ�ϴ� �Լ�
string getDirectoryName(string video_name) {
	return RESULT_FOLDER_NAME + video_name;
}

// ������Ʈ ���� �ش� ���丮�� �ִ� �� üũ�ϴ� �Լ�
bool isDirectory(string dir_name, string video_name) {
	// string type�� folderName�� const char* �� �ٲپ� access �Լ��� �ִ� ����
	std::vector<char> writable(dir_name.begin(), dir_name.end());
	writable.push_back('\0');
	char* ptr = &writable[0];

	return _access(ptr, 0) == 0;
	// ������ ���� ��쿡�� _access(ptr, 0) ���� 0�� ��ȯ�Ͽ� true, �׷��� ������ false.
}