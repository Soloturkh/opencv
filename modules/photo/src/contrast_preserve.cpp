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


#include "precomp.hpp"
#include "opencv2/photo.hpp"
#include "math.h"
#include <vector>
#include <limits>
#include "contrast_preserve.hpp"

using namespace std;
using namespace cv;

void cv::decolor(InputArray _src, OutputArray _dst, OutputArray _color_boost)
{
    Mat I = _src.getMat();
    _dst.create(I.size(), CV_8UC1);
    Mat dst = _dst.getMat();

    _color_boost.create(I.size(), CV_8UC3);
    Mat color_boost = _color_boost.getMat();

    CV_Assert(!I.empty() && (I.channels() == 3));

    // Parameter Setting
    const int maxIter = 15;
    const double tol = .0001;
    int iterCount = 0;
    double E = 0;
    double pre_E = std::numeric_limits<double>::infinity();

    // Initialization
    Decolor obj;
    obj.init();

    Mat img = Mat(I.size(), CV_32FC3);
    I.convertTo(img, CV_32FC3, 1.0 / 255.0);

    vector <double> Cg;
    vector <std::vector <double> > polyGrad;
    vector <Vec3i> comb;
    vector <double> alf;

    obj.grad_system(img, polyGrad, Cg, comb);
    obj.weak_order(img, alf);

    // Solver
    Mat Mt(static_cast<int>(polyGrad.size()), static_cast<int>(polyGrad[0].size()), CV_32FC1);
    obj.wei_update_matrix(polyGrad, Cg, Mt);

    vector <double> wei;
    obj.wei_inti(comb, wei);

    //////////////////////////////// main loop starting ////////////////////////////////////////

    {
        vector <double> temp;
        vector <double> temp1;
        vector <double> temp2;
        vector <double> G_pos;
        vector <double> G_neg;
        vector <double> EXPsum;
        vector <double> EXPterm;
        vector <double> wei1;

        while (sqrt(pow(E - pre_E, 2)) > tol)
        {
            iterCount += 1;
            pre_E = E;

            temp.clear();
            temp1.clear();
            temp2.clear();
            G_pos.clear();
            G_neg.clear();
            EXPsum.clear();
            EXPterm.clear();
            wei1.clear();

            temp.resize(polyGrad[0].size());
            temp1.resize(polyGrad[0].size());
            for (unsigned int i = 0; i < polyGrad[0].size(); i++)
            {
                double val = 0.0;
                for (unsigned int j = 0; j < polyGrad.size(); j++)
                    val = val + (polyGrad[j][i] * wei[j]);
                temp[i] = val - Cg[i];
                temp1[i] = val + Cg[i];
            }

            G_pos.resize(alf.size());
            G_neg.resize(alf.size());
            for (unsigned int i = 0; i < alf.size(); i++)
            {
                const double pos = ((1 + alf[i]) / 2) * exp((-1.0 * 0.5 * pow(temp[i], 2)) / pow(obj.sigma, 2));
                const double neg = ((1 - alf[i]) / 2) * exp((-1.0 * 0.5 * pow(temp1[i], 2)) / pow(obj.sigma, 2));
                G_pos[i] = pos;
                G_neg[i] = neg;
            }

            EXPsum.resize(G_pos.size());
            for (unsigned int i = 0; i < G_pos.size(); i++)
                EXPsum[i] = (G_pos[i] + G_neg[i]);

            temp2.resize(EXPsum.size());
            for (unsigned int i = 0; i < EXPsum.size(); i++)
            {
                if (EXPsum[i] == 0)
                    temp2[i] = 1.0;
                else
                    temp2[i] = 0.0;
            }

            EXPterm.resize(G_pos.size());
            for (unsigned int i = 0; i < G_pos.size(); i++)
                EXPterm[i] = ((G_pos[i] - G_neg[i]) / (EXPsum[i] + temp2[i]));

            wei1.resize(polyGrad.size());
            for (unsigned int i = 0; i < polyGrad.size(); i++)
            {
                double val1 = 0.0;
                for (unsigned int j = 0; j < polyGrad[0].size(); j++)
                {
                    val1 = val1 + (Mt.at<float>(i, j) * EXPterm[j]);
                }
                wei1[i] = val1;
            }

            for (unsigned int i = 0; i < wei.size(); i++)
                wei[i] = wei1[i];

            E = obj.energyCalcu(Cg, polyGrad, wei);

            if (iterCount > maxIter)
                break;
        }
    }

    Mat Gray = Mat::zeros(img.size(), CV_32FC1);
    obj.grayImContruct(wei, img, Gray);

    Gray.convertTo(dst, CV_8UC1, 255);

    ///////////////////////////////////       Contrast Boosting   /////////////////////////////////

    Mat lab;
    cvtColor(I, lab, COLOR_BGR2Lab);

    vector <Mat> lab_channel;
    split(lab, lab_channel);

    dst.copyTo(lab_channel[0]);

    merge(lab_channel, lab);

    cvtColor(lab, color_boost, COLOR_Lab2BGR);
}
