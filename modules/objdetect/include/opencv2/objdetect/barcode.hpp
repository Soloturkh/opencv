// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
// Copyright (c) 2020-2021 darkliang wangberlinT Certseeds

#ifndef OPENCV_OBJDETECT_BARCODE_HPP
#define OPENCV_OBJDETECT_BARCODE_HPP

#include <opencv2/core.hpp>
#include <opencv2/objdetect/graphical_code_detector.hpp>

namespace cv {
namespace barcode {

//! @addtogroup objdetect_barcode
//! @{

class CV_EXPORTS_W_SIMPLE BarcodeDetector : public cv::GraphicalCodeDetector
{
public:
    /** @brief Initialize the BarcodeDetector.
    */
    CV_WRAP BarcodeDetector();
    /** @brief Initialize the BarcodeDetector.
     *
     * Parameters allow to load _optional_ Super Resolution DNN model for better quality.
     * @param prototxt_path prototxt file path for the super resolution model
     * @param model_path model file path for the super resolution model
     */
    CV_WRAP BarcodeDetector(CV_WRAP_FILE_PATH const std::string &prototxt_path, CV_WRAP_FILE_PATH const std::string &model_path);
    ~BarcodeDetector();

    /** @brief Decodes barcode in image once it's found by the detect() method.
     *
     * @param img grayscale or color (BGR) image containing bar code.
     * @param points vector of rotated rectangle vertices found by detect() method (or some other algorithm).
     * For N detected barcodes, the dimensions of this array should be [N][4].
     * Order of four points in vector<Point2f> is bottomLeft, topLeft, topRight, bottomRight.
     * @param decoded_info UTF8-encoded output vector of string or empty vector of string if the codes cannot be decoded.
     * @param decoded_type vector strings, specifies the type of these barcodes
     * @return true if at least one valid barcode have been found
     */
    CV_WRAP bool decodeWithType(InputArray img,
                             InputArray points,
                             CV_OUT std::vector<std::string> &decoded_info,
                             CV_OUT std::vector<std::string> &decoded_type) const;

    /** @brief Both detects and decodes barcode

     * @param img grayscale or color (BGR) image containing barcode.
     * @param decoded_info UTF8-encoded output vector of string(s) or empty vector of string if the codes cannot be decoded.
     * @param decoded_type vector of strings, specifies the type of these barcodes
     * @param points optional output vector of vertices of the found  barcode rectangle. Will be empty if not found.
     * @return true if at least one valid barcode have been found
     */
    CV_WRAP bool detectAndDecodeWithType(InputArray img,
                                      CV_OUT std::vector<std::string> &decoded_info,
                                      CV_OUT std::vector<std::string> &decoded_type,
                                      OutputArray points = noArray()) const;

    /** @brief Returns detector downsample threshold

     * @param thresh output parameter for returning the value.
     */
    CV_WRAP void getDownSampleThresh(CV_OUT double& thresh) const;

    /** @brief Set detector downsampling threshold. Input will be resized if minimum dimension is larger than threshold (default 512)

     * @param thresh downsapling limit to apply if min(shape[:2])
     */
    CV_WRAP BarcodeDetector& setDownSampleThresh(double thresh);

    /** @brief Returns detector window sizes

     * @param sizes output parameter for returning input scales.
     */
    CV_WRAP void getDetectorScales(CV_OUT std::vector<double>& sizes) const;

    /** @brief Set detector window sizes

     * @param sizes window sizes with respect to minimum dimension of image (default [0.01, 0.03, 0.06, 0.08])
     */
    CV_WRAP BarcodeDetector& setDetectorScales(std::vector<double> sizes);

    /** @brief Returns detector gradient magnitude threshold

     * @param thresh output parameter for returning the value.
     */
    CV_WRAP void getGradientThresh(CV_OUT double& thresh) const;

    /** @brief Set detector gradient magnitude threshold

     * @param thresh gradient magnitude threshold (default 64)
     */
    CV_WRAP BarcodeDetector& setGradientThresh(double thresh);
};
//! @}

}} // cv::barcode::

#endif // OPENCV_OBJDETECT_BARCODE_HPP
