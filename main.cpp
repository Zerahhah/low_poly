#include<iostream>
#include<vector>
#include<random>
#include<algorithm>
#include<windows.h>
#include<opencv2/opencv.hpp>
#include<opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

struct TrianglePoints {
	cv::Point p1, p2, p3;
	TrianglePoints(cv::Point _p1, cv::Point _p2, cv::Point _p3) :p1(_p1), p2(_p2), p3(_p3) {}
};

class TriangleStyle {
public:
	TriangleStyle(std::string _name, int h_threshold, int l_threshold,int random_num) {
		image_name = _name; 
		high_threshold = h_threshold;
		low_threshold = l_threshold;
		random_amount = random_num;
		start();
	}
	void start() {
		//1. 平滑
		image = cv::imread(image_name, 1);
		cv::GaussianBlur(image, image_blur, cv::Size(5, 5), 0);
		//cv::imshow("blur", image_blur);

		//2.转灰度图
		cv::cvtColor(image_blur, gray_image, CV_RGB2GRAY);
		image_rows = image_blur.rows;//图像行数
		image_cols = image_blur.cols;//图像列数
		//cv::imshow("gray", gray_image);

		//3.取边缘 + 随机点
		//Canny算子
		cv::Canny(gray_image, grad_canny, high_threshold, low_threshold, 3);
		//cv::imshow("Canny", grad_canny);
		for (auto i = 0; i < grad_canny.rows; ++i) {
			for (auto j = 0; j < grad_canny.cols; ++j) {
				if (grad_canny.at<uchar>(i, j) == 255) {
					margin_dots.push_back(cv::Point(j, i));
				}
			}
		}
		/*cv::Mat result_dot(grad_canny.rows, grad_canny.cols, CV_8UC1,cv::Scalar::all(0));
		for (auto &point : margin_dots) {
		cv::circle(result_dot, point, 1, cv::Scalar(255));
		}
		imshow("result_dot",result_dot);*/
		//随机点
		grad_canny.copyTo(grad_plus_random_dot);
		random_numbers.resize(random_amount);
		for (auto &num : random_numbers) {
			num.x = get_random_int(image_cols - 1);
			num.y = get_random_int(image_rows - 1);
			//如果随机坐标已经存在，那么再次随机。
			while (findNumber(margin_dots, num)) {
				num.x = get_random_int(image_cols - 1);
				num.y = get_random_int(image_rows - 1);
			}
			grad_plus_random_dot.at<uchar>(num.y, num.x) = 255;
			margin_dots.push_back(num);//随机坐标放入边缘点中
		}
		/*for (auto &num : random_numbers) {
		std::cout << "x: " << num.x << "  .y: " << num.y << std::endl;
		}
		std::cout << image_rows << "  " << image_cols << std::endl;*/
		//cv::imshow("Plus_Random", grad_plus_random_dot);

		//4.Delaunay算法组成三角形
		cv::Rect rect(0, 0, image_cols, image_rows);
		triangles = delaunayAlgorithm(rect, margin_dots);
		delaunay_image = cv::Mat(image_rows, image_cols, CV_8UC1, cv::Scalar::all(0));
		for (auto triangle : triangles) {
			cv::line(delaunay_image, triangle.p1, triangle.p2, cv::Scalar(255));
			cv::line(delaunay_image, triangle.p1, triangle.p3, cv::Scalar(255));
			cv::line(delaunay_image, triangle.p2, triangle.p3, cv::Scalar(255));
		}
		//cv::imshow("delaunay", delaunay_image);

		//5.取三角形中点颜色，作为三角形的颜色
		result_image = cv::Mat(image_rows, image_cols, CV_8UC3, cv::Scalar(0, 0, 0));
		for (auto triangle : triangles) {
			cv::Point mid_point = cv::Point((int)(triangle.p1.x + triangle.p2.x + triangle.p3.x) / 3, (int)((triangle.p1.y + triangle.p2.y + triangle.p3.y) / 3));
			//std::cout << "mid point X: " << mid_point.x << "  mid point Y: " << mid_point.y << std::endl;
			//防止有三角形中点越界
			if (mid_point.x < 0) { mid_point.x = 0; }
			if (mid_point.x >image_cols) { mid_point.x = image_cols - 1; }
			if (mid_point.y < 0) { mid_point.y = 0; }
			if (mid_point.y >image_rows) { mid_point.y = image_rows - 1; }
			if (mid_point.x < 0 || mid_point.x > image_cols) { std::cout << "mid_point.x X failed" << std::endl; }
			if (mid_point.y < 0 || mid_point.y > image_rows) { std::cout << "mid_point.y Y failed" << std::endl; }

			std::vector<int> temp_circum_point = circumRect(triangle);
			/*std::cout << "min_x:" << temp_circum_point[0] << "   min_y: " << temp_circum_point[1] <<
			"   max_x: " << temp_circum_point[2] << "   max_y: " << temp_circum_point[3] << std::endl;*/

			if (temp_circum_point[0] < 0) { temp_circum_point[0] = 0; }
			if (temp_circum_point[0] >image_cols) { temp_circum_point[0] = image_cols; }
			if (temp_circum_point[1] < 0) { temp_circum_point[1] = 0; }
			if (temp_circum_point[1] >image_rows) { temp_circum_point[1] = image_rows; }
			if (temp_circum_point[2] < 0) { temp_circum_point[2] = 0; }
			if (temp_circum_point[2] >image_cols) { temp_circum_point[2] = image_cols; }
			if (temp_circum_point[3] < 0) { temp_circum_point[3] = 0; }
			if (temp_circum_point[3] >image_rows) { temp_circum_point[3] = image_rows; }



			if (temp_circum_point[0] < 0 || temp_circum_point[0] > image_cols) { std::cout << "min_x X failed" << std::endl; }
			if (temp_circum_point[1] < 0 || temp_circum_point[1] > image_rows) { std::cout << "min_y Y failed" << std::endl; }
			if (temp_circum_point[2] < 0 || temp_circum_point[2] > image_cols) { std::cout << "max_x X failed" << std::endl; }
			if (temp_circum_point[3] < 0 || temp_circum_point[3] > image_rows) { std::cout << "max_y Y failed" << std::endl; }

			for (auto i = temp_circum_point[1]; i < temp_circum_point[3] && i < image_rows; ++i) {
				for (auto j = temp_circum_point[0]; j < temp_circum_point[2] && j < image_cols; ++j) {
					/*
					result_image.at<cv::Vec3b>(i, j)[0] = image.at<cv::Vec3b>(mid_point.y, mid_point.x)[0];
					result_image.at<cv::Vec3b>(i, j)[1] = image.at<cv::Vec3b>(mid_point.y, mid_point.x)[1];
					result_image.at<cv::Vec3b>(i, j)[2] = image.at<cv::Vec3b>(mid_point.y, mid_point.x)[2];*/
					if (isInTriangle(triangle, cv::Point(j, i))) {
						//std::cout << "seccess once..." << std::endl;
						result_image.at<cv::Vec3b>(i, j)[0] = image.at<cv::Vec3b>(mid_point.y, mid_point.x)[0];
						result_image.at<cv::Vec3b>(i, j)[1] = image.at<cv::Vec3b>(mid_point.y, mid_point.x)[1];
						result_image.at<cv::Vec3b>(i, j)[2] = image.at<cv::Vec3b>(mid_point.y, mid_point.x)[2];
					}
				}
			}
		}

	}
	void show() {
		cv::imshow("result", result_image);
	}
	void save() {
		cv::imwrite("三角剖分.jpg", delaunay_image);
		cv::imwrite("结果图.jpg", result_image);
	}
private:
	int image_rows;//图像行数
	int image_cols;//图像列数
	int high_threshold;//canny算子高阈值
	int low_threshold;//canny算子低阈值
	int random_amount;//控制有多少个随机数
	std::string image_name;

	std::vector<cv::Point> margin_dots;//边缘点的存储
	std::vector<cv::Point> random_numbers;//随机点
	std::vector<TrianglePoints> triangles;//三角剖分

	cv::Mat image;
	cv::Mat image_blur;
	cv::Mat gray_image;
	cv::Mat grad_canny;
	cv::Mat grad_plus_random_dot;
	cv::Mat delaunay_image;
	cv::Mat result_image;

	int get_random_int(int upbound) {//随机产生0~upbound整数
		std::random_device seed;
		std::default_random_engine rd(seed());//随机种子
		std::uniform_int_distribution<> u(0, upbound);//分布
		int res = u(rd);
		return  res;
	}

	bool findNumber(std::vector<cv::Point> &vec_points, cv::Point point) {
		for (auto it = vec_points.cbegin(); it != vec_points.cend(); ++it) {
			if (point.x == it->x) {
				if (point.y == it->y) {
					return true;
				}
				else { return false; }
			}
			else { return false; }
		}
		return false;
	}


	/*
	|x1 y1 1|
	|x2 y2 1|
	|x3 y3 1|
	*/
	float calcuArea(cv::Point point1, cv::Point point2, cv::Point point3) {
		float ax = point2.x - point1.x, ay = point2.y - point1.y, bx = point2.x - point3.x, by = point2.y - point3.y;
		return fabs(ax * by - ay * bx) / 2;
	}
	float calcuArea(TrianglePoints &tri) {
		float ax = tri.p2.x - tri.p1.x, ay = tri.p2.y - tri.p1.y, bx = tri.p2.x - tri.p3.x, by = tri.p2.y - tri.p3.y;
		return fabs(ax * by - ay * bx) / 2;
	}

	bool isInTriangle(TrianglePoints &tri, cv::Point point) {
		float s = calcuArea(tri);
		float s1 = calcuArea(tri.p2, tri.p3, point);
		float s2 = calcuArea(tri.p1, tri.p3, point);
		float s3 = calcuArea(tri.p1, tri.p2, point);
		return (s1 + s2 + s3) <= s;
	}


	std::vector<TrianglePoints> delaunayAlgorithm(cv::Rect boundRect, std::vector<cv::Point> &points) {
		if (points.empty()) {
			return std::vector<TrianglePoints>();
		}
		std::vector<TrianglePoints> result;
		std::vector<cv::Vec6f> temp_result;//(x1,y1,x2,y2,x3,y3)
		cv::Subdiv2D subdiv2d(boundRect);
		for (auto point : points) {
			subdiv2d.insert(cv::Point2f((float)point.x, (float)point.y));
		}
		subdiv2d.getTriangleList(temp_result);
		//std::cout << "here" << std::endl;
		for (auto temp_vec : temp_result) {
			cv::Point point1((int)temp_vec[0], (int)temp_vec[1]);
			cv::Point point2((int)temp_vec[2], (int)temp_vec[3]);
			cv::Point point3((int)temp_vec[4], (int)temp_vec[5]);
			result.push_back(TrianglePoints(point1, point2, point3));
		}
		return result;
	}

	//返回左上点和右下点(x1,y1,x2,y2)
	std::vector<int> circumRect(TrianglePoints &tri) {
		std::vector<int> circum_points;
		int min_x = ((tri.p1.x < tri.p2.x) ? (tri.p1.x < tri.p3.x ? tri.p1.x : tri.p3.x) : (tri.p2.x < tri.p3.x) ? tri.p2.x : tri.p3.x);
		int min_y = ((tri.p1.y < tri.p2.y) ? (tri.p1.y < tri.p3.y ? tri.p1.y : tri.p3.y) : (tri.p2.y < tri.p3.y) ? tri.p2.y : tri.p3.y);
		int max_x = ((tri.p1.x > tri.p2.x) ? (tri.p1.x > tri.p3.x ? tri.p1.x : tri.p3.x) : (tri.p2.x > tri.p3.x) ? tri.p2.x : tri.p3.x);
		int max_y = ((tri.p1.y > tri.p2.y) ? (tri.p1.y > tri.p3.y ? tri.p1.y : tri.p3.y) : (tri.p2.y > tri.p3.y) ? tri.p2.y : tri.p3.y);
		circum_points.push_back(min_x);
		circum_points.push_back(min_y);
		circum_points.push_back(max_x);
		circum_points.push_back(max_y);
		return circum_points;
	}
};

int main() {
	TriangleStyle t("adorable-animal-cat-357141.jpg", 300, 100, 700);
	t.show();
	t.save();

	std::cout << "success!" << std::endl;
	cv::waitKey(0);
	system("pause");
	return 0;
}