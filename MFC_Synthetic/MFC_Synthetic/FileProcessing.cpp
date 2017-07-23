#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <opencv2\opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <direct.h>
#include <io.h>

// 파일 처리와 관련된 모든 함수들을 선언합니다.
// segment의 fileName할당, JPG 파일 저장, txt파일 저장
void saveSegmentation_JPG(component object, Mat frame, string video_path);
void saveSegmentation_TXT(component object, FILE *fp);
string allocatingComponentFilename(int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector);

// segment 폴더 안에 Segmentation된 Obj만을 jpg파일로 저장하는 함수
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS);

// Video Path에서 file이름만 빼서 확장자를 제거하여 반환하는 함수
String getFileName(CString f_path, char find_char, BOOL extension) {
	// 마지막 \ 뒤의 문자열
	// 검색대상 :: "Video (*.avi, *.MP4) | *.avi;*.mp4; | All Files(*.*)|*.*||",
	string f_name;
	char final_index;
	for (int i = 0; i < f_path.GetLength(); i++) {
		if (f_path[i] == find_char)
			final_index = i;
	} // '\'를 찾고

	for (int i = final_index + 1; i < f_path.GetLength(); i++) {
		if (f_path[final_index + 1] == NULL) break; // 예외처리
		if (extension == false){
			if (f_path[i] == '.') break; // 확장자 제거 (. 이후에 문자는 무시)
			// 폴더를 만드는데 확장자가 붙으면 확장자에 맞는 파일이 생성되는 오류가 있어서 일단 뺌
			// 혹시나 확장자가 필요할 경우 함수를 하나 더 생성하던지 해야할 듯
		}
		char c = f_path[i];
		f_name += c;
	}// 마지막에 나오는 '\' 이후의 문자열을 추출함

	return f_name;
}

// 전체 segment 데이터의 파일들을 저장하는 모듈
bool saveSegmentationData(string video_name, component object, Mat object_frame
	, int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector, FILE *txt_fp) {

	// object의 파일이름 할당
	object.fileName = allocatingComponentFilename(timeTag, currentMsec, frameCount, indexOfhumanDetectedVector);

	// jpg파일로 저장
	saveSegmentation_JPG(object, object_frame, getObjDirectoryPath(video_name));

	// txt파일로 저장
	saveSegmentation_TXT(object, txt_fp);

	return true;
}

void saveSegmentation_JPG(component object, Mat frame, string video_path) {
	string fullPath;
	// 저장할 이미지를 받아옴
	Mat img = objectCutting(object, frame, frame.rows, frame.cols);

	// 저장 될 경로를 따라 파일 이름을 생성
	stringstream ss;
	ss << video_path << "/" << object.fileName << ".jpg";
	fullPath = ss.str();
	ss.str("");

	// 저장하면서 빨간 사각형을 그리는 함수
	rectangle(img, Point(0, 0),
		Point(object.width, object.height),
		Scalar(0, 0, 255), 2);

	// 해당 이미지를 얻어온 파일이름을 통해 jpg파일로 저장
	int check_writeFullPath = imwrite(fullPath, img);
}

// Segmentation된 Obj의 Data를 txt파일로 저장하는 함수
// format : FILE_NAME(sec_frameCount) X Y WIDTH HEIGHT
void saveSegmentation_TXT(component object, FILE *fp) {
	string info;
	stringstream ss;
	ss << object.fileName << " " << object.left << " " << object.top << " " << object.right << " " << object.bottom
		<< " " << object.right - object.left << " " << object.bottom - object.top << '\n';
	info = ss.str();
	fprintf(fp, info.c_str());
}

// jpg로 저장된 object를 반환해 주는 함수
Mat loadJPGObjectFile(segment obj, string file_name) {
	stringstream ss;
	String fullPath;

	// segment_파일이름 폴더 내에 파일 객체 이름 포맷으로 stringstream에 저장함
	ss << getObjDirectoryPath(file_name) << "/" << obj.fileName.c_str();
	fullPath = ss.str();
	ss.str("");
	Mat frame = imread(fullPath);
	return frame;
}

// ROI영역만 추출하는 함수
Mat objectCutting(component object, Mat img, unsigned int ROWS, unsigned int COLS) {
	return img(Rect(object.left, object.top, object.width, object.height)).clone();
	//잘린 이미지 반환
}

// 텍스트 파일(세그먼트 정보가 저장된) 이름을 반환하는 함수
string getTextFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name 
		+ "/" + RESULT_TEXT_FILENAME + video_name + (".txt");
}

// 배경 파일 이름을 반환하는 함수
string getBackgroundFilePath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name 
		+ "/" + RESULT_BACKGROUND_FILENAME + video_name + (".jpg");
}

// 세그먼트들이 통합적인 정보들이 저장된 폴더이름을 반환하는 함수 , 폴더 및 경로 :: /data/(비디오 이름)
string getDirectoryPath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name;
}

// 세그먼트(object)들이 저장된 폴더이름을 반환하는 함수 , 폴더 및 경로 :: /data/(비디오 이름)/obj
string getObjDirectoryPath(string video_name) {
	return SEGMENTATION_DATA_DIRECTORY_NAME + "/" + video_name + "/" + "obj";
}

// 프로젝트 내에 해당 디렉토리가 있는 지 체크하는 함수
bool isDirectory(string dir_name) {
	// string type을 const char* 로 바꾸는 연산
	std::vector<char> writable(dir_name.begin(), dir_name.end());
	writable.push_back('\0');
	char *ptr_name = &writable[0];

	// 폴더가 있을 경우에는 _access(ptr, 0) 값을 0을 반환하여 true, 그렇지 않으면 false.
	return _access(ptr_name, 0) == 0;
}

// data 폴더를 생성하는 함수
int makeDataRootDirectory() {
	// 세그먼테이션 결과 데이터가 들어있는 폴더가 없는 경우 만들어 줌
	return _mkdir(SEGMENTATION_DATA_DIRECTORY_NAME.c_str());
}

// data 폴더 안에 해당 비디오의 경로 및 이름을 갖는 디렉토리를 생성하는 함수
int makeDataSubDirectory(string video_path) {
	// 해당 비디오의 세그먼테이션 결과가 들어갈 폴더경로에 하위 폴더 생성(/data/(video_name))
	return _mkdir(video_path.c_str());
}

// 파일의 이름부분을 저장
string allocatingComponentFilename(int timeTag, int currentMsec, int frameCount, int indexOfhumanDetectedVector) {
	string name;
	return name.append(to_string(timeTag)).append("_")
		.append(to_string(currentMsec)).append("_")
		.append(to_string(frameCount)).append("_")
		.append(to_string(indexOfhumanDetectedVector));
}