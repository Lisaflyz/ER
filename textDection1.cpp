/*
* textdetection.cpp
*
* A demo program of the Extremal Region Filter algorithm described in
* Neumann L., Matas J.: Real-Time Scene Text Localization and Recognition, CVPR 2012
*
* Created on: Sep 23, 2013
*     Author: Lluis Gomez i Bigorda <lgomez AT cvc.uab.es>
*/



#include "erfilter.hpp"
#include  "opencv2/highgui.hpp"
#include  "opencv2/imgproc.hpp"
#include  <vector>
#include  <iostream>
#include  <iomanip>
#include <string>


using namespace std;
using namespace cv;
using namespace text;

void show_help_and_exit(const char *cmd);//帮助
void groups_draw(Mat &src, vector<Rect> &groups);
void er_show(vector<Mat> &channels, vector<vector<ERStat> > &regions);

int main(int argc, const char * argv[])
{
	cout << endl << argv[0] << endl << endl;
	cout << "Demo program of the Extremal Region Filter algorithm described in " << endl;
	cout << "Neumann L., Matas J.: Real-Time Scene Text Localization and Recognition, CVPR 2012" << endl << endl;

	// if (argc < 2) show_help_and_exit(argv[0]);

	// namedWindow("grouping",WINDOW_NORMAL);
	//string path1 = "E:/project/data/test/";
	//string path2 = "E:/project/data/result/";
	
		Mat src = imread("E:/project/data/test/img_6.jpg");
		//imshow("src", src);
		// Extract channels to be processed individually
		vector<Mat> channels;
		//默认为 ERFILTER_NM_RGBLGrad
		computeNMChannels(src, channels);

		int cn = (int)channels.size();
		// Append negative channels to detect ER- (bright regions over dark background)
		//只需加入原图的反向图即可处理
		for (int c = 0; c < cn - 1; c++)
			channels.push_back(255 - channels[c]);

		// Create ERFilter objects with the 1st and 2nd stage default classifiers
		Ptr<ERFilter> er_filter1 = createERFilterNM1(loadClassifierNM1("trained_classifierNM1.xml"), 16, 0.00015f, 0.13f, 0.2f, true, 0.1f);
		Ptr<ERFilter> er_filter2 = createERFilterNM2(loadClassifierNM2("trained_classifierNM2.xml"), 0.5);

		vector<vector<ERStat> > regions(channels.size());
		// Apply the default cascade classifier to each independent channel (could be done in parallel)
		cout << "Extracting Class Specific Extremal Regions from " << (int)channels.size() << " channels ..." << endl;
		cout << "    (...) this may take a while (...)" << endl << endl;
		for (int c = 0; c < (int)channels.size(); c++)
		{
			er_filter1->run(channels[c], regions[c]);
			er_filter2->run(channels[c], regions[c]);
		}

		// Detect character groups
		cout << "Grouping extracted ERs ... ";
		vector< vector<Vec2i> > region_groups;
		vector<Rect> groups_boxes;
		erGrouping(src, channels, regions, region_groups, groups_boxes, ERGROUPING_ORIENTATION_HORIZ);
		//erGrouping(src, channels, regions, region_groups, groups_boxes, ERGROUPING_ORIENTATION_ANY, "./trained_classifier_erGrouping.xml", 0.5);

		// 在原图上画出矩形框，并将含矩形框的图存到文件夹draw groups
		groups_draw(src, groups_boxes);
		imshow("grouping",src);
		


		cout << "Done!" << endl << endl;
		cout << "Press 'e' to show the extracted Extremal Regions, any other key to exit." << endl << endl;
		if (waitKey(-1) == 101)
			er_show(channels, regions);

		// memory clean-up
		//内存清理
		er_filter1.release();
		er_filter2.release();
		regions.clear();
		if (!groups_boxes.empty())
		{
			groups_boxes.clear();
		}
	

	return 0;
}

// helper functions
//错误提示
void show_help_and_exit(const char *cmd)
{
	cout << "    Usage: " << cmd << " <input_image> " << endl;
	cout << "    Default classifier files (trained_classifierNM*.xml) must be in current directory" << endl << endl;
	exit(-1);
}

//将矩形画在原图上，多通道和单通道的划线的颜色不同
void groups_draw(Mat &src, vector<Rect> &groups)
{
	for (int i = (int)groups.size() - 1; i >= 0; i--)
	{
		if (src.type() == CV_8UC3)
			rectangle(src, groups.at(i).tl(), groups.at(i).br(), Scalar(0, 255, 255), 3, 8);
		else
			rectangle(src, groups.at(i).tl(), groups.at(i).br(), Scalar(255), 3, 8);
	}
}

void er_show(vector<Mat> &channels, vector<vector<ERStat> > &regions)
{
	for (int c = 0; c<(int)channels.size(); c++)
	{
		Mat dst = Mat::zeros(channels[0].rows + 2, channels[0].cols + 2, CV_8UC1);
		for (int r = 0; r<(int)regions[c].size(); r++)
		{
			ERStat er = regions[c][r];
			if (er.parent != NULL) // deprecate the root region 根区域不执行
			{
				int newMaskVal = 255;
				//四邻域、填充值为255、只填充掩膜、当前像素与种子像素之间的差
				int flags = 4 + (newMaskVal << 8) + FLOODFILL_FIXED_RANGE + FLOODFILL_MASK_ONLY;
				floodFill(channels[c], dst, Point(er.pixel%channels[c].cols, er.pixel / channels[c].cols),
					Scalar(255), 0, Scalar(er.level), Scalar(0), flags);
			}
		}
		char buff[10]; char *buff_ptr = buff;
		sprintf(buff, "channel %d", c);
		imshow(buff_ptr, dst);
	}
	waitKey(-1);
}
