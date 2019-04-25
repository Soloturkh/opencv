/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Copyright (C) 2017, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "../precomp.hpp"
#include "layers_common.hpp"

namespace cv
{
namespace dnn
{

namespace util
{

std::string makeName(const std::string& str1, const std::string& str2)
{
    return str1 + str2;
}
bool getParameter(const LayerParams &params, const std::string& nameBase, const std::string& nameAll,
                  std::vector<size_t>& parameter, bool hasDefault = false, const int& defaultValue = 0)
{
    std::string nameH = makeName(nameBase, std::string("_h"));
    std::string nameW = makeName(nameBase, std::string("_w"));
    std::string nameAll_ = nameAll;
    if (nameAll_ == "")
        nameAll_ = nameBase;

    if (params.has(nameH) && params.has(nameW))
    {
        CV_Assert(params.get<int>(nameH) >= 0 && params.get<int>(nameW) >= 0);
        parameter.push_back(params.get<int>(nameH));
        parameter.push_back(params.get<int>(nameW));
        return true;
    }
    else
    {
        if (params.has(nameAll_))
        {
            DictValue param = params.get(nameAll_);
            for (int i = 0; i < param.size(); i++) {
                CV_Assert(param.get<int>(i) >= 0);
                parameter.push_back(param.get<int>(i));
            }
            return true;
        }
        else
        {
            if (hasDefault)
            {
                parameter.push_back(defaultValue);
                return true;
            }
            return false;
        }
    }
}

void getKernelSize(const LayerParams &params, std::vector<size_t>& kernel)
{
    if (!util::getParameter(params, "kernel", "kernel_size", kernel))
        CV_Error(cv::Error::StsBadArg, "kernel_size (or kernel_h and kernel_w) not specified");

    if (kernel.size() == 1)
        kernel.resize(2, kernel[0]);

    for (int i = 0; i < kernel.size(); i++)
        CV_Assert(kernel[i] > 0);
}

void getStrideAndPadding(const LayerParams &params, std::vector<size_t>& pads_begin, std::vector<size_t>& pads_end,
                         std::vector<size_t>& strides, cv::String& padMode)
{
    if (params.has("pad_l") && params.has("pad_t") && params.has("pad_r") && params.has("pad_b")) {
        CV_Assert(params.get<int>("pad_t") >= 0 && params.get<int>("pad_l") >= 0 &&
                  params.get<int>("pad_b") >= 0 && params.get<int>("pad_r") >= 0);
        pads_begin.push_back(params.get<int>("pad_t"));
        pads_begin.push_back(params.get<int>("pad_l"));
        pads_end.push_back(params.get<int>("pad_b"));
        pads_end.push_back(params.get<int>("pad_r"));
    }
    else {
        util::getParameter(params, "pad", "pad", pads_begin, true, 0);
        if (pads_begin.size() < 4)
            pads_end = pads_begin;
        else
        {
            pads_end = std::vector<size_t>(pads_begin.begin() + pads_begin.size() / 2, pads_begin.end());
            pads_begin.resize(pads_begin.size() / 2);
        }
        CV_Assert(pads_begin.size() == pads_end.size());
    }
    util::getParameter(params, "stride", "stride", strides, true, 1);

    padMode = "";
    if (params.has("pad_mode"))
        padMode = params.get<String>("pad_mode");

    for (int i = 0; i < strides.size(); i++)
        CV_Assert(strides[i] > 0);
}

void checkSize(size_t expected_size, std::vector<size_t>& param) {
    CV_Assert(param.size() >= 1 && param.size() <= expected_size);
    param.resize(expected_size, param[0]);
}
}

void getPoolingKernelParams(const LayerParams &params, std::vector<size_t>& kernel, bool &globalPooling,
                            std::vector<size_t>& pads_begin, std::vector<size_t>& pads_end,
                            std::vector<size_t>& strides, cv::String &padMode)
{
    util::getStrideAndPadding(params, pads_begin, pads_end, strides, padMode);
    globalPooling = params.has("global_pooling") &&
                    params.get<bool>("global_pooling");
    int size = 2;
    if (globalPooling)
    {
        if(params.has("kernel_h") || params.has("kernel_w") || params.has("kernel_size"))
        {
            CV_Error(cv::Error::StsBadArg, "In global_pooling mode, kernel_size (or kernel_h and kernel_w) cannot be specified");
        }
        for (int i = 0; i < pads_begin.size(); i++) {
            if (pads_begin[i] != 0 || pads_end[i] != 0)
                CV_Error(cv::Error::StsBadArg, "In global_pooling mode, pads must be = 0");
        }
        for (int i = 0; i < strides.size(); i++) {
            if (strides[i] != 1)
                CV_Error(cv::Error::StsBadArg, "In global_pooling mode, strides must be = 1");
        }
    }
    else
    {
        util::getKernelSize(params, kernel);
        size = kernel.size();
    }
    util::checkSize(size, pads_begin);
    util::checkSize(size, pads_end);
    util::checkSize(size, strides);
}

void getConvolutionKernelParams(const LayerParams &params, std::vector<size_t>& kernel, std::vector<size_t>& pads_begin,
                                std::vector<size_t>& pads_end, std::vector<size_t>& strides, std::vector<size_t>& dilations, cv::String &padMode)
{
    util::getKernelSize(params, kernel);
    util::getStrideAndPadding(params, pads_begin, pads_end, strides, padMode);
    util::getParameter(params, "dilation", "dilation", dilations, true, 1);

    util::checkSize(kernel.size(), pads_begin);
    util::checkSize(kernel.size(), pads_end);
    util::checkSize(kernel.size(), strides);
    util::checkSize(kernel.size(), dilations);

    for (int i = 0; i < dilations.size(); i++)
        CV_Assert(dilations[i] > 0);
}

// From TensorFlow code:
// Total padding on rows and cols is
// Pr = (R' - 1) * S + Kr - R
// Pc = (C' - 1) * S + Kc - C
// where (R', C') are output dimensions, (R, C) are input dimensions, S
// is stride, (Kr, Kc) are filter dimensions.
// We pad Pr/2 on the left and Pr - Pr/2 on the right, Pc/2 on the top
// and Pc - Pc/2 on the bottom.  When Pr or Pc is odd, this means
// we pad more on the right and bottom than on the top and left.
void getConvPoolOutParams(const std::vector<int>& inp, const std::vector<size_t>& kernel,
                          const std::vector<size_t>& stride, const String &padMode,
                          const std::vector<size_t>& dilation, std::vector<int>& out)
{
    if (padMode == "VALID")
    {
        for (int i = 0; i < inp.size(); i++)
            out.push_back((inp[i] - dilation[i] * (kernel[i] - 1) - 1 + stride[i]) / stride[i]);
    }
    else if (padMode == "SAME")
    {
        for (int i = 0; i < inp.size(); i++)
            out.push_back((inp[i] - 1 + stride[i]) / stride[i]);
    }
    else
        CV_Error(Error::StsError, "Unsupported padding mode");
}

void getConvPoolPaddings(const std::vector<int>& inp, const std::vector<int>& out,
                         const std::vector<size_t>& kernel, const std::vector<size_t>& strides,
                         const String &padMode, const std::vector<size_t>& dilation,
                         std::vector<size_t>& pads_begin, std::vector<size_t>& pads_end)
{
    CV_Assert_N(kernel.size() == pads_begin.size() || pads_begin.size() == 1, pads_begin.size() == pads_end.size());
    pads_begin.resize(kernel.size(), pads_begin[0]);
    pads_end.resize(kernel.size(), pads_end[0]);
    CV_Assert(pads_begin.size() == pads_end.size() && pads_begin.size() == kernel.size());
    if (padMode == "VALID")
    {
        std::fill(pads_begin.begin(), pads_begin.end(), 0);
        std::fill(pads_end.begin(), pads_end.end(), 0);
    }
    else if (padMode == "SAME")
    {
        for (int i = 0; i < pads_begin.size(); i++) {
            int pad = ((out[i] - 1) * strides[i] + dilation[i] * (kernel[i] - 1) + 1 - inp[i]) / 2;
            pads_begin[i] = pads_end[i] = std::max(0, pad);
        }
    }
}

}
}
