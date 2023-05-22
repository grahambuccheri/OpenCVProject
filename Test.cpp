#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

//include


using namespace cv;
using namespace std;

Mat imgGray, imgBlur, imgCanny, imgDil, imgErode, imgHSV, mask,bg,img;
String path = "";

//camera settings
int quadAlt;
int airSpeed;
int frameRate;
int focalLength;

//SENSITIVITY SETTING
int absMin = 800;
int minArea = 1000;
int outArea = 0;
//
String getSeverity() {
	//add altitude scaling for fire area 
	 quadAlt = 100;
	outArea = outArea * (100 / quadAlt);
	String out = "ERROR";
	if (outArea < 500) {
		out = "Low Risk";
	}
	else if (outArea >= 5000 && outArea < 15000) {
		out = "Mild Risk";
	}
	else if (outArea >= 15000 && outArea < 50000) {
		out = "High Risk";
	}
	else if (outArea >= 50000) {
		out = "Severe Risk";
	}
	return out;
}
void getContours(Mat imgDil, Mat img) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(imgDil, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//drawContours(img, contours, -1, Scalar(255, 0, 255), 10);
	//filter
	int total = 0;
	int count = 0;
	int totalArea = 0;
	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);
		total += area;
		count += 1;
	}
	int avg = total / count - 500; //Calculate Average Area of Fire
	if (avg > absMin) {
		minArea = avg;
	}
	else {
		minArea = absMin;
	}
	
	cout << minArea << endl;
	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);
		vector<vector<Point>> conPoly(contours.size());
		vector<Rect> boundRect(contours.size());
		if (area >= minArea) {
			float peri = arcLength(contours[i], true);
			totalArea += contourArea(contours[i]);

			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			drawContours(img, conPoly, i, Scalar(255, 0, 255), 5);
			cout << conPoly[i].size() << endl;
			//creates bounding rectangle
			boundRect[i] = boundingRect(conPoly[i]);
			rectangle(img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
		}
		//Draw area for smaller fires
		else if(area <= minArea && area >= 500) {
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			drawContours(img, conPoly, i, Scalar(255, 150, 150), 5);
			cout << conPoly[i].size() << endl;
			//creates bounding rectangle
			boundRect[i] = boundingRect(conPoly[i]);
			rectangle(img, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 255, 0), 5);
		}
	}
	outArea = totalArea;
}

void processImg() {
	 img = imread(path);  // use for image files
	Scalar low = Scalar(0, 0, 0);
	Scalar high = Scalar(100, 100, 100);

	cvtColor(img, imgHSV, COLOR_BGR2HSV);

	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	imgGray.copyTo(bg);


	inRange(imgGray, low, high, mask);
	//End Mask

	GaussianBlur(mask, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);
	Mat kernal = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernal);
	dilate(imgDil, imgDil, kernal);
	getContours(imgDil, img);
}
void main() {
	
	// path = "Resources/forest.png";
	 //path = "Resources/fire.jpeg";
	// path = "Resources/test.png";
	// path = "Resources/lol.png";
	// path = "Resources/campfire.jpg";
	 path = "Resources/house.png";
	// path = "Resources/lowFire.png";

	processImg();
	

	//show image
	imshow("Image", img);
	//imshow("Blurred", imgBlur);
	//imshow("Image Dil", imgDil);
	imshow("dil", imgDil);
	cout << "Total Area: " << outArea;
	cout << "\n" << getSeverity();
	waitKey(0);

}
