#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>

using namespace cv;
using namespace std;

int sharpenRadius = 1;
Mat image, sharpen;
const char* window_name1 = "multiScaleSharpen";

//multi-scale detail boosting
Mat multiScaleSharpen(Mat, int);
// define a trackbar callback
static void onTrackbar(int, void*)
{
    sharpen = multiScaleSharpen(image, sharpenRadius *2+1);
    imshow(window_name1, sharpen);
}
static void help(const char** argv)
{
    printf("\nThis sample demonstrates multiScaleSharpen image processing\n"
           "Call:\n"
           "    %s [image_name -- Default is lena.jpg]\n\n", argv[0]);
}
const char* keys =
{
    "{help h||}{@image |lena.jpg|input image name}"
};
int main( int argc, const char** argv )
{
    help(argv);
    CommandLineParser parser(argc, argv, keys);
    string filename = parser.get<string>(0);
    image = imread(samples::findFile(filename), IMREAD_COLOR);
    if(image.empty())
    {
        printf("Cannot read image file: %s\n", filename.c_str());
        help(argv);
        return -1;
    }
    // Create a window
    namedWindow(window_name1, 1);
    // create a toolbar
    createTrackbar("multiScaleSharpen default", window_name1, &sharpenRadius, 7, onTrackbar);
    // Show the image
    onTrackbar(0, 0);
    // Wait for a key stroke; the same function arranges events processing
    waitKey(0);
    return 0;
}
Mat multiScaleSharpen(Mat Src, int Radius)
{
    int rows = Src.rows;
    int cols = Src.cols;
    int channels = Src.channels();
    Mat B1, B2, B3;
    GaussianBlur(Src, B1, Size(Radius, Radius), 1.0, 1.0);
    GaussianBlur(Src, B2, Size(Radius * 2 - 1, Radius * 2 - 1), 2.0, 2.0);
    GaussianBlur(Src, B3, Size(Radius * 4 - 1, Radius * 4 - 1), 4.0, 4.0);
    double w1 = 0.5;
    double w2 = 0.5;
    double w3 = 0.25;
    cv::Mat dest = cv::Mat::zeros(Src.size(), Src.type());
    for (int i = 0; i < rows; i++)
    {
        uchar* src_ptr = Src.ptr<uchar>(i);
        uchar* dest_ptr = dest.ptr<uchar>(i);
        uchar* B1_ptr = B1.ptr<uchar>(i);
        uchar* B2_ptr = B2.ptr<uchar>(i);
        uchar* B3_ptr = B3.ptr<uchar>(i);
        for (int j = 0; j < cols; j++)
        {
            for (int c = 0; c < channels; c++)
            {
                int  D1 = src_ptr[3 * j + c] - B1_ptr[3 * j + c];
                int  D2 = B1_ptr[3 * j + c] - B2_ptr[3 * j + c];
                int  D3 = B2_ptr[3 * j + c] - B3_ptr[3 * j + c];
                int  sign = (D1 > 0) ? 1 : -1;
                dest_ptr[3 * j + c] = saturate_cast<uchar>((1 - w1 * sign)*D1 - w2 * D2 + w3 * D3 + src_ptr[3 * j + c]);
            }
        }
    }
    return dest;
}
