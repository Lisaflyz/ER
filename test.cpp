#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

void change(InputArray _src){
	Mat a = _src.getMat();
	a.resize(10, 10);
	imshow("a", a);
	waitKey(300);
}
int main111(){
	
		Mat src = imread("E:/project/data/training/img_1.jpg");
		change(src);
		imshow("src", src);
	
	waitKey();
	return 0;
}