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
// void saveSegmentation_JPG(component object, Mat frame, string video_path);
void saveSegmentation_TXT(component object, FILE *fp);
int directionChecker(component object, int ROWS, int COLS);
//string allocatingComponentFilename(int timeTag, int currentMsec, int frameCount, int label_num);

// segment ���� �ȿ� Segmentation�� Obj���� jpg���Ϸ� �����ϴ� �Լ�
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS);

string readTxt(string path){
	string result;
	ifstream in(path);
	if (in.is_open()){
		in.seekg(0, ios::end);
		int size = in.tellg();
		result.resize(size);
		in.seekg(0, ios::beg);
		in.read(&result[0], size);
		in.close();
		return result;
	}
	else{
		cout << path << "�� �о�� �� �����ϴ�" << endl;
		return NULL;
	}
}

boolean rewriteTxt(string path, string text){
	ofstream out(path,ios::trunc);
	if (out.is_open()){
		out << text;
		out.close();
		return true;
	}
	else{
		cout << path << "�� �о�� �� �����ϴ�" << endl;
		return false;
	}
}

boolean appendTxt(string path, string text){
	ofstream out(path,ios::app);
	if (out.is_open()){
		out << text;
		out.close();
		return true;
	}
	else{
		cout << path << "�� �о�� �� �����ϴ�" << endl;
		return false;
	}
}

string lineMaker_detail(int timetag, int label, int first, int last, int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8,int c9){
	return to_string(timetag).append(" ").append(to_string(label)).append(" ").append(to_string(first)).append(" ").append(to_string(last)).append(" ").append(to_string(c1)).append(" ").append(to_string(c2)).append(" ").append(to_string(c3)).append(" ")
		.append(to_string(c4)).append(" ").append(to_string(c5)).append(" ").append(to_string(c6)).append(" ").append(to_string(c7)).append(" ").append(to_string(c8)).append(" ").append(to_string(c9)).append("\n");
}

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

bool saveSegmentationData(string fileNameNoExtension, component object, Mat object_frame
	, int currentMsec, int frameCount, FILE *txt_fp, FILE * txt_fp_detail, int ROWS, int COLS
	, vector<pair<int, int>>* vectorDetailTXTInedx, int* detailTxtIndex, int *colorArray) {
	// object�� �⺻ ���� ����
	// object�� �����̸� �Ҵ�
	object.fileName = allocatingComponentFilename(object.timeTag, currentMsec, frameCount, object.label);

	// jpg���Ϸ� ����
	saveSegmentation_JPG(object, object_frame, getObjDirectoryPath(fileNameNoExtension));

	// txt���Ϸ� ����
	saveSegmentation_TXT(object, txt_fp);

	// ���� �� ���� ���� �ؽ�Ʈ ���� ����
	if (object.timeTag == currentMsec){//���� ������Ʈ�� ��ü�� ó�� �� ���
		appendTxt(getDetailTextFilePath(fileNameNoExtension).c_str(),
			lineMaker_detail(object.timeTag, object.label, directionChecker(object, ROWS, COLS), 10
				, 0, 0, 0, 0, 0, 0, 0, 0, 0));
	}
	else 	//ù ������Ʈ�� �ƴ� ��� �ش� ��ü ��ġ�� �̵��Ͽ� last ��ġ �����
		saveColorData(fileNameNoExtension, object, colorArray);

	return true;
}

void saveColorData(string fileNameNoExtension, component object, int *colorArray){
	//���� ���� �ؽ�Ʈ ���� ����
	//�ش� ��ü ��ġ�� �̵��Ͽ� Color ī��Ʈ �����
	int stamp;
	int label;
	int tempFirst;
	int tempLast;
	int colors[COLORS] = { 0, };

	string txt = readTxt(getDetailTextFilePath(fileNameNoExtension).c_str());

	size_t posOfTimetag = txt.find(to_string(object.timeTag).append(" ").append(to_string(object.label)));
	if (posOfTimetag != string::npos){
		int posOfNL = txt.find("\n", posOfTimetag);

		string capture = txt.substr(posOfTimetag, posOfNL - posOfTimetag);
		char *line = new char[capture.length() + 1];
		strcpy(line, capture.c_str());

		char *ptr = strtok(line, " ");
		if (ptr != NULL){
			stamp = atoi(ptr);
			ptr = strtok(NULL, " ");
			if (ptr != NULL){
				label = atoi(ptr);
			}
			ptr = strtok(NULL, " ");
			if (ptr != NULL)
				tempFirst = atoi(ptr);
			ptr = strtok(NULL, " ");
			if (ptr != NULL)
				tempLast = atoi(ptr);

			for (int i = 0; i < COLORS; i++){
				ptr = strtok(NULL, " ");
				if (ptr != NULL)
					colors[i] = atoi(ptr);
			}

			txt.erase(posOfTimetag, posOfNL - posOfTimetag + 1);
			txt.insert(posOfTimetag, lineMaker_detail(stamp, label, tempFirst, tempLast, colors[0] + colorArray[0], colors[1] + colorArray[1], colors[2] + colorArray[2], colors[3] + colorArray[3], colors[4] + colorArray[4],
				colors[5] + colorArray[5], colors[6] + colorArray[6], colors[7] + colorArray[7], colors[8] + colorArray[8]));
			rewriteTxt(getDetailTextFilePath(fileNameNoExtension).c_str(), txt.c_str());
		}
		ptr = NULL;
		delete ptr;
		delete[] line;
	}
	return;
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
		Point(object.width, object.height),
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
	fflush(stdout);
	return;
}

int directionChecker(component object, int ROWS, int COLS){
	int result = 10;
	int padding = 10;	//�����ڸ���� ����� ��ġ ���� �ȼ���
	//��
	if (object.left<padding){
		result += 10;
	}
	//��
	else if (object.right>COLS - padding - 1){
		result += 5;
	}
	else{	//���
	}

	//��
	if (object.top<padding){
		result += 3;
	}
	//��
	else if (object.bottom>ROWS - padding - 1){
		result += 9;
	}
	else{	//���
	}

	return result;
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
	return img(Rect(object.left, object.top, object.width, object.height)).clone();
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

// �⺻ ��� ���� �̸��� ��ȯ�ϴ� �Լ�
string getBackgroundFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name
		+ "/" + RESULT_BACKGROUND_FILENAME + video_name + (".jpg");
}

// ���꿡 ����� ��� ���� �̸��� ��ȯ�ϴ� �Լ�
string getTempBackgroundFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name
		+ "/" + RESULT_BACKGROUND_FILENAME + "_T_" + video_name + (".jpg");
}

// �ռ����� ��½� ����� �÷� ��� ���� �̸��� ��ȯ�ϴ� �Լ�
string getColorBackgroundFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name
		+ "/" + RESULT_BACKGROUND_FILENAME+ "_C_" + video_name + (".jpg");
}

// ���׸�Ʈ���� �������� �������� ����� �����̸��� ��ȯ�ϴ� �Լ� , ���� �� ��� :: /data/(���� �̸�)
string getDirectoryPath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name;
}

// ���׸�Ʈ(object)���� ����� �����̸��� ��ȯ�ϴ� �Լ� , ���� �� ��� :: /data/(���� �̸�)/obj
string getObjDirectoryPath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name + "/" + "obj";
}

// ���׸�Ʈ(object)���� ����� �����̸��� ��ȯ�ϴ� �Լ� , ���� �� ��� :: /data/(���� �̸�)/obj
string getObj_for_colorDirectoryPath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name + "/" + "obj_for_color";
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

//���� �ε�� ����� ȸ�� ����� �ִ� �� Ȯ���ϴ� �Լ�
bool isGrayBackgroundExists(string name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
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
string allocatingComponentFilename(int timeTag, int currentMsec, int frameCount, int label_num) {
	string name;
	return name.append(to_string(timeTag)).append("_")
		.append(to_string(currentMsec)).append("_")
		.append(to_string(frameCount)).append("_")
		.append(to_string(label_num));
}