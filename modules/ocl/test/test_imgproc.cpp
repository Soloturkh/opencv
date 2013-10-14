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
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Copyright (C) 2010-2012, Multicoreware, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Niko Li, newlife20080214@gmail.com
//    Jia Haipeng, jiahaipeng95@gmail.com
//    Shengen Yan, yanshengen@gmail.com
//    Jiang Liyuan, lyuan001.good@163.com
//    Rock Li, Rock.Li@amd.com
//    Wu Zailong, bullet@yeah.net
//    Xu Pang, pangxu010@163.com
//    Sen Liu, swjtuls1987@126.com
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other oclMaterials provided with the distribution.
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

#include "test_precomp.hpp"

#ifdef HAVE_OPENCL

using namespace cv;
using namespace std;
using namespace testing;

MatType nulltype = -1;

#define ONE_TYPE(type)  testing::ValuesIn(typeVector(type))
#define NULL_TYPE  testing::ValuesIn(typeVector(nulltype))

vector<MatType> typeVector(MatType type)
{
    vector<MatType> v;
    v.push_back(type);
    return v;
}

typedef struct
{
    short x;
    short y;
} COOR;

COOR do_meanShift(int x0, int y0, uchar *sptr, uchar *dptr, int sstep, Size size, int sp, int sr, int maxIter, float eps, int *tab)
{

    int isr2 = sr * sr;
    int c0, c1, c2, c3;
    int iter;
    uchar *ptr = NULL;
    uchar *pstart = NULL;
    int revx = 0, revy = 0;
    c0 = sptr[0];
    c1 = sptr[1];
    c2 = sptr[2];
    c3 = sptr[3];
    // iterate meanshift procedure
    for(iter = 0; iter < maxIter; iter++ )
    {
        int count = 0;
        int s0 = 0, s1 = 0, s2 = 0, sx = 0, sy = 0;

        //mean shift: process pixels in window (p-sigmaSp)x(p+sigmaSp)
        int minx = x0 - sp;
        int miny = y0 - sp;
        int maxx = x0 + sp;
        int maxy = y0 + sp;

        //deal with the image boundary
        if(minx < 0) minx = 0;
        if(miny < 0) miny = 0;
        if(maxx >= size.width) maxx = size.width - 1;
        if(maxy >= size.height) maxy = size.height - 1;
        if(iter == 0)
        {
            pstart = sptr;
        }
        else
        {
            pstart = pstart + revy * sstep + (revx << 2); //point to the new position
        }
        ptr = pstart;
        ptr = ptr + (miny - y0) * sstep + ((minx - x0) << 2); //point to the start in the row

        for( int y = miny; y <= maxy; y++, ptr += sstep - ((maxx - minx + 1) << 2))
        {
            int rowCount = 0;
            int x = minx;
#if CV_ENABLE_UNROLLED
            for( ; x + 4 <= maxx; x += 4, ptr += 16)
            {
                int t0, t1, t2;
                t0 = ptr[0], t1 = ptr[1], t2 = ptr[2];
                if(tab[t0 - c0 + 255] + tab[t1 - c1 + 255] + tab[t2 - c2 + 255] <= isr2)
                {
                    s0 += t0;
                    s1 += t1;
                    s2 += t2;
                    sx += x;
                    rowCount++;
                }
                t0 = ptr[4], t1 = ptr[5], t2 = ptr[6];
                if(tab[t0 - c0 + 255] + tab[t1 - c1 + 255] + tab[t2 - c2 + 255] <= isr2)
                {
                    s0 += t0;
                    s1 += t1;
                    s2 += t2;
                    sx += x + 1;
                    rowCount++;
                }
                t0 = ptr[8], t1 = ptr[9], t2 = ptr[10];
                if(tab[t0 - c0 + 255] + tab[t1 - c1 + 255] + tab[t2 - c2 + 255] <= isr2)
                {
                    s0 += t0;
                    s1 += t1;
                    s2 += t2;
                    sx += x + 2;
                    rowCount++;
                }
                t0 = ptr[12], t1 = ptr[13], t2 = ptr[14];
                if(tab[t0 - c0 + 255] + tab[t1 - c1 + 255] + tab[t2 - c2 + 255] <= isr2)
                {
                    s0 += t0;
                    s1 += t1;
                    s2 += t2;
                    sx += x + 3;
                    rowCount++;
                }
            }
#endif
            for(; x <= maxx; x++, ptr += 4)
            {
                int t0 = ptr[0], t1 = ptr[1], t2 = ptr[2];
                if(tab[t0 - c0 + 255] + tab[t1 - c1 + 255] + tab[t2 - c2 + 255] <= isr2)
                {
                    s0 += t0;
                    s1 += t1;
                    s2 += t2;
                    sx += x;
                    rowCount++;
                }
            }
            if(rowCount == 0)
                continue;
            count += rowCount;
            sy += y * rowCount;
        }

        if( count == 0 )
            break;

        int x1 = sx / count;
        int y1 = sy / count;
        s0 = s0 / count;
        s1 = s1 / count;
        s2 = s2 / count;

        bool stopFlag = (x0 == x1 && y0 == y1) || (abs(x1 - x0) + abs(y1 - y0) +
                        tab[s0 - c0 + 255] + tab[s1 - c1 + 255] + tab[s2 - c2 + 255] <= eps);

        //revise the pointer corresponding to the new (y0,x0)
        revx = x1 - x0;
        revy = y1 - y0;

        x0 = x1;
        y0 = y1;
        c0 = s0;
        c1 = s1;
        c2 = s2;

        if( stopFlag )
            break;
    } //for iter

    dptr[0] = (uchar)c0;
    dptr[1] = (uchar)c1;
    dptr[2] = (uchar)c2;
    dptr[3] = (uchar)c3;

    COOR coor;
    coor.x = (short)x0;
    coor.y = (short)y0;
    return coor;
}

void meanShiftFiltering_(const Mat &src_roi, Mat &dst_roi, int sp, int sr, TermCriteria crit)
{
    if( src_roi.empty() )
        CV_Error( CV_StsBadArg, "The input image is empty" );

    if( src_roi.depth() != CV_8U || src_roi.channels() != 4 )
        CV_Error( CV_StsUnsupportedFormat, "Only 8-bit, 4-channel images are supported" );

    CV_Assert( (src_roi.cols == dst_roi.cols) && (src_roi.rows == dst_roi.rows) );
    CV_Assert( !(dst_roi.step & 0x3) );

    if( !(crit.type & TermCriteria::MAX_ITER) )
        crit.maxCount = 5;
    int maxIter = std::min(std::max(crit.maxCount, 1), 100);
    float eps;
    if( !(crit.type & TermCriteria::EPS) )
        eps = 1.f;
    eps = (float)std::max(crit.epsilon, 0.0);

    int tab[512];
    for(int i = 0; i < 512; i++)
        tab[i] = (i - 255) * (i - 255);
    uchar *sptr = src_roi.data;
    uchar *dptr = dst_roi.data;
    int sstep = (int)src_roi.step;
    int dstep = (int)dst_roi.step;
    Size size = src_roi.size();

    for(int i = 0; i < size.height; i++, sptr += sstep - (size.width << 2),
            dptr += dstep - (size.width << 2))
    {
        for(int j = 0; j < size.width; j++, sptr += 4, dptr += 4)
        {
            do_meanShift(j, i, sptr, dptr, sstep, size, sp, sr, maxIter, eps, tab);
        }
    }
}

void meanShiftProc_(const Mat &src_roi, Mat &dst_roi, Mat &dstCoor_roi, int sp, int sr, TermCriteria crit)
{

    if( src_roi.empty() )
        CV_Error( CV_StsBadArg, "The input image is empty" );
    if( src_roi.depth() != CV_8U || src_roi.channels() != 4 )
        CV_Error( CV_StsUnsupportedFormat, "Only 8-bit, 4-channel images are supported" );
    CV_Assert( (src_roi.cols == dst_roi.cols) && (src_roi.rows == dst_roi.rows) &&
               (src_roi.cols == dstCoor_roi.cols) && (src_roi.rows == dstCoor_roi.rows));
    CV_Assert( !(dstCoor_roi.step & 0x3) );

    if( !(crit.type & TermCriteria::MAX_ITER) )
        crit.maxCount = 5;
    int maxIter = std::min(std::max(crit.maxCount, 1), 100);
    float eps;
    if( !(crit.type & TermCriteria::EPS) )
        eps = 1.f;
    eps = (float)std::max(crit.epsilon, 0.0);

    int tab[512];
    for(int i = 0; i < 512; i++)
        tab[i] = (i - 255) * (i - 255);
    uchar *sptr = src_roi.data;
    uchar *dptr = dst_roi.data;
    short *dCoorptr = (short *)dstCoor_roi.data;
    int sstep = (int)src_roi.step;
    int dstep = (int)dst_roi.step;
    int dCoorstep = (int)dstCoor_roi.step >> 1;
    Size size = src_roi.size();

    for(int i = 0; i < size.height; i++, sptr += sstep - (size.width << 2),
            dptr += dstep - (size.width << 2), dCoorptr += dCoorstep - (size.width << 1))
    {
        for(int j = 0; j < size.width; j++, sptr += 4, dptr += 4, dCoorptr += 2)
        {
            *((COOR *)dCoorptr) = do_meanShift(j, i, sptr, dptr, sstep, size, sp, sr, maxIter, eps, tab);
        }
    }

}

PARAM_TEST_CASE(ImgprocTestBase, MatType, MatType, MatType, MatType, MatType, bool)
{
    int type1, type2, type3, type4, type5;
    Scalar val;
    // set up roi
    int roicols;
    int roirows;
    int src1x;
    int src1y;
    int src2x;
    int src2y;
    int dstx;
    int dsty;
    int dst1x;
    int dst1y;
    int maskx;
    int masky;

    //mat
    Mat mat1;
    Mat mat2;
    Mat mask;
    Mat dst;
    Mat dst1; //bak, for two outputs

    //mat with roi
    Mat mat1_roi;
    Mat mat2_roi;
    Mat mask_roi;
    Mat dst_roi;
    Mat dst1_roi; //bak

    //ocl mat
    ocl::oclMat clmat1;
    ocl::oclMat clmat2;
    ocl::oclMat clmask;
    ocl::oclMat cldst;
    ocl::oclMat cldst1; //bak

    //ocl mat with roi
    ocl::oclMat clmat1_roi;
    ocl::oclMat clmat2_roi;
    ocl::oclMat clmask_roi;
    ocl::oclMat cldst_roi;
    ocl::oclMat cldst1_roi;

    virtual void SetUp()
    {
        type1 = GET_PARAM(0);
        type2 = GET_PARAM(1);
        type3 = GET_PARAM(2);
        type4 = GET_PARAM(3);
        type5 = GET_PARAM(4);
        Size size(MWIDTH, MHEIGHT);
        double min = 1, max = 20;

        if(type1 != nulltype)
        {
            mat1 = randomMat(size, type1, min, max, false);
            clmat1 = mat1;
        }
        if(type2 != nulltype)
        {
            mat2 = randomMat(size, type2, min, max, false);
            clmat2 = mat2;
        }
        if(type3 != nulltype)
        {
            dst  = randomMat(size, type3, min, max, false);
            cldst = dst;
        }
        if(type4 != nulltype)
        {
            dst1 = randomMat(size, type4, min, max, false);
            cldst1 = dst1;
        }
        if(type5 != nulltype)
        {
            mask = randomMat(size, CV_8UC1, 0, 2,  false);
            threshold(mask, mask, 0.5, 255., type5);
            clmask = mask;
        }
        val = Scalar(rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0));
    }

    void random_roi()
    {
#ifdef RANDOMROI
        //randomize ROI
        roicols = rng.uniform(1, mat1.cols);
        roirows = rng.uniform(1, mat1.rows);
        src1x   = rng.uniform(0, mat1.cols - roicols);
        src1y   = rng.uniform(0, mat1.rows - roirows);
        src2x   = rng.uniform(0, mat2.cols - roicols);
        src2y   = rng.uniform(0, mat2.rows - roirows);
        dstx    = rng.uniform(0, dst.cols  - roicols);
        dsty    = rng.uniform(0, dst.rows  - roirows);
        dst1x    = rng.uniform(0, dst1.cols  - roicols);
        dst1y    = rng.uniform(0, dst1.rows  - roirows);
        maskx   = rng.uniform(0, mask.cols - roicols);
        masky   = rng.uniform(0, mask.rows - roirows);
#else
        roicols = mat1.cols;
        roirows = mat1.rows;
        src1x = 0;
        src1y = 0;
        src2x = 0;
        src2y = 0;
        dstx = 0;
        dsty = 0;
        dst1x = 0;
        dst1y = 0;
        maskx = 0;
        masky = 0;
#endif


        if(type1 != nulltype)
        {
            mat1_roi = mat1(Rect(src1x, src1y, roicols, roirows));
            clmat1_roi = clmat1(Rect(src1x, src1y, roicols, roirows));
        }
        if(type2 != nulltype)
        {
            mat2_roi = mat2(Rect(src2x, src2y, roicols, roirows));
            clmat2_roi = clmat2(Rect(src2x, src2y, roicols, roirows));
        }
        if(type3 != nulltype)
        {
            dst_roi  = dst(Rect(dstx, dsty, roicols, roirows));
            cldst_roi = cldst(Rect(dstx, dsty, roicols, roirows));
        }
        if(type4 != nulltype)
        {
            dst1_roi = dst1(Rect(dst1x, dst1y, roicols, roirows));
            cldst1_roi = cldst1(Rect(dst1x, dst1y, roicols, roirows));
        }
        if(type5 != nulltype)
        {
            mask_roi = mask(Rect(maskx, masky, roicols, roirows));
            clmask_roi = clmask(Rect(maskx, masky, roicols, roirows));
        }
    }

    void Near(double threshold)
    {
        Mat cpu_cldst;
        cldst.download(cpu_cldst);
        EXPECT_MAT_NEAR(dst, cpu_cldst, threshold);
    }
};
////////////////////////////////equalizeHist//////////////////////////////////////////

typedef ImgprocTestBase EqualizeHist;

OCL_TEST_P(EqualizeHist, Mat)
{
    if (mat1.type() != CV_8UC1 || mat1.type() != dst.type())
    {
        cout << "Unsupported type" << endl;
        EXPECT_DOUBLE_EQ(0.0, 0.0);
    }
    else
    {
        for(int j = 0; j < LOOP_TIMES; j++)
        {
            random_roi();
            equalizeHist(mat1_roi, dst_roi);
            ocl::equalizeHist(clmat1_roi, cldst_roi);
            Near(1.1);
        }
    }
}


////////////////////////////////copyMakeBorder////////////////////////////////////////////

typedef ImgprocTestBase CopyMakeBorder;

OCL_TEST_P(CopyMakeBorder, Mat)
{
    int bordertype[] = {BORDER_CONSTANT, BORDER_REPLICATE, BORDER_REFLECT, BORDER_WRAP, BORDER_REFLECT_101};
    int top = rng.uniform(0, 10);
    int bottom = rng.uniform(0, 10);
    int left = rng.uniform(0, 10);
    int right = rng.uniform(0, 10);
    if (mat1.type() != dst.type())
    {
        cout << "Unsupported type" << endl;
        EXPECT_DOUBLE_EQ(0.0, 0.0);
    }
    else
    {
        for(size_t i = 0; i < sizeof(bordertype) / sizeof(int); i++)
            for(int j = 0; j < LOOP_TIMES; j++)
            {
                random_roi();
#ifdef RANDOMROI
                if(((bordertype[i] != BORDER_CONSTANT) && (bordertype[i] != BORDER_REPLICATE)) && (mat1_roi.cols <= left) || (mat1_roi.cols <= right) || (mat1_roi.rows <= top) || (mat1_roi.rows <= bottom))
                {
                    continue;
                }
                if((dstx >= left) && (dsty >= top) && (dstx + cldst_roi.cols + right <= cldst_roi.wholecols) && (dsty + cldst_roi.rows + bottom <= cldst_roi.wholerows))
                {
                    dst_roi.adjustROI(top, bottom, left, right);
                    cldst_roi.adjustROI(top, bottom, left, right);
                }
                else
                {
                    continue;
                }
#endif
                cv::copyMakeBorder(mat1_roi, dst_roi, top, bottom, left, right, bordertype[i] | BORDER_ISOLATED, Scalar(1.0));
                ocl::copyMakeBorder(clmat1_roi, cldst_roi, top, bottom, left, right,  bordertype[i] | BORDER_ISOLATED, Scalar(1.0));

                Mat cpu_cldst;
#ifndef RANDOMROI
                cldst_roi.download(cpu_cldst);
                EXPECT_MAT_NEAR(dst_roi, cpu_cldst, 0.0);
#else
                cldst.download(cpu_cldst);
                EXPECT_MAT_NEAR(dst, cpu_cldst, 0.0);
#endif

            }
    }
}



////////////////////////////////cornerMinEigenVal//////////////////////////////////////////

struct CornerMinEigenVal : ImgprocTestBase {};

OCL_TEST_P(CornerMinEigenVal, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {

        random_roi();
        int blockSize = 3, apertureSize = 3;//1 + 2 * (rand() % 4);
        //int borderType = BORDER_CONSTANT;
        //int borderType = BORDER_REPLICATE;
        int borderType = BORDER_REFLECT;
        cornerMinEigenVal(mat1_roi, dst_roi, blockSize, apertureSize, borderType);
        ocl::cornerMinEigenVal(clmat1_roi, cldst_roi, blockSize, apertureSize, borderType);
        Near(1.);
    }
}



////////////////////////////////cornerHarris//////////////////////////////////////////

typedef ImgprocTestBase CornerHarris;

OCL_TEST_P(CornerHarris, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {

        random_roi();
        int blockSize = 3, apertureSize = 3; //1 + 2 * (rand() % 4);
        double k = 2;
        //int borderType = BORDER_CONSTANT;
        //int borderType = BORDER_REPLICATE;
        int borderType = BORDER_REFLECT;
        cornerHarris(mat1_roi, dst_roi, blockSize, apertureSize, k, borderType);
        ocl::cornerHarris(clmat1_roi, cldst_roi, blockSize, apertureSize, k, borderType);
        Near(1.);
    }
}


////////////////////////////////integral/////////////////////////////////////////////////

typedef ImgprocTestBase Integral;

OCL_TEST_P(Integral, Mat1)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        ocl::integral(clmat1_roi, cldst_roi);
        integral(mat1_roi, dst_roi);
        Near(0);
    }
}

OCL_TEST_P(Integral, Mat2)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        ocl::integral(clmat1_roi, cldst_roi, cldst1_roi);
        integral(mat1_roi, dst_roi, dst1_roi);
        Near(0);

        Mat cpu_cldst1;
        cldst1.download(cpu_cldst1);
        EXPECT_MAT_NEAR(dst1, cpu_cldst1, 0.0);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// warpAffine  & warpPerspective

PARAM_TEST_CASE(WarpTestBase, MatType, int)
{
    int type;
    Size size;
    int interpolation;

    //src mat
    Mat mat1;
    Mat dst;

    // set up roi
    int src_roicols;
    int src_roirows;
    int dst_roicols;
    int dst_roirows;
    int src1x;
    int src1y;
    int dstx;
    int dsty;


    //src mat with roi
    Mat mat1_roi;
    Mat dst_roi;

    //ocl dst mat for testing
    ocl::oclMat gdst_whole;

    //ocl mat with roi
    ocl::oclMat gmat1;
    ocl::oclMat gdst;

    virtual void SetUp()
    {
        type = GET_PARAM(0);
        interpolation = GET_PARAM(1);
        size = Size(MWIDTH, MHEIGHT);

        mat1 = randomMat(size, type, 5, 16, false);
        dst  = randomMat(size, type, 5, 16, false);
    }

    void random_roi()
    {
#ifdef RANDOMROI
        //randomize ROI
        src_roicols = rng.uniform(1, mat1.cols);
        src_roirows = rng.uniform(1, mat1.rows);
        dst_roicols = rng.uniform(1, dst.cols);
        dst_roirows = rng.uniform(1, dst.rows);
        src1x   = rng.uniform(0, mat1.cols - src_roicols);
        src1y   = rng.uniform(0, mat1.rows - src_roirows);
        dstx    = rng.uniform(0, dst.cols  - dst_roicols);
        dsty    = rng.uniform(0, dst.rows  - dst_roirows);
#else
        src_roicols = mat1.cols;
        src_roirows = mat1.rows;
        dst_roicols = dst.cols;
        dst_roirows = dst.rows;
        src1x   = 0;
        src1y   = 0;
        dstx    = 0;
        dsty    = 0;
#endif


        mat1_roi = mat1(Rect(src1x, src1y, src_roicols, src_roirows));
        dst_roi  = dst(Rect(dstx, dsty, dst_roicols, dst_roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx, dsty, dst_roicols, dst_roirows));


        gmat1 = mat1_roi;
    }

};

/////warpAffine

typedef WarpTestBase WarpAffine;

OCL_TEST_P(WarpAffine, Mat)
{
    static const double coeffs[2][3] =
    {
        {cos(CV_PI / 6), -sin(CV_PI / 6), 100.0},
        {sin(CV_PI / 6), cos(CV_PI / 6), -100.0}
    };
    Mat M(2, 3, CV_64F, (void *)coeffs);

    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        warpAffine(mat1_roi, dst_roi, M, size, interpolation);
        ocl::warpAffine(gmat1, gdst, M, size, interpolation);

        Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
    }

}


// warpPerspective

typedef WarpTestBase WarpPerspective;

OCL_TEST_P(WarpPerspective, Mat)
{
    static const double coeffs[3][3] =
    {
        {cos(3.14 / 6), -sin(3.14 / 6), 100.0},
        {sin(3.14 / 6), cos(3.14 / 6), -100.0},
        {0.0, 0.0, 1.0}
    };
    Mat M(3, 3, CV_64F, (void *)coeffs);

    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        warpPerspective(mat1_roi, dst_roi, M, size, interpolation);
        ocl::warpPerspective(gmat1, gdst, M, size, interpolation);

        Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////
// remap
//////////////////////////////////////////////////////////////////////////////////////////////////

PARAM_TEST_CASE(Remap, MatType, MatType, MatType, int, int)
{
    int srcType;
    int map1Type;
    int map2Type;
    Scalar val;

    int interpolation;
    int bordertype;

    Mat src;
    Mat dst;
    Mat map1;
    Mat map2;

    //std::vector<ocl::Info> oclinfo;

    int src_roicols;
    int src_roirows;
    int dst_roicols;
    int dst_roirows;
    int map1_roicols;
    int map1_roirows;
    int map2_roicols;
    int map2_roirows;
    int srcx;
    int srcy;
    int dstx;
    int dsty;
    int map1x;
    int map1y;
    int map2x;
    int map2y;

    Mat src_roi;
    Mat dst_roi;
    Mat map1_roi;
    Mat map2_roi;

    //ocl mat for testing
    ocl::oclMat gdst;

    //ocl mat with roi
    ocl::oclMat gsrc_roi;
    ocl::oclMat gdst_roi;
    ocl::oclMat gmap1_roi;
    ocl::oclMat gmap2_roi;

    virtual void SetUp()
    {
        srcType = GET_PARAM(0);
        map1Type = GET_PARAM(1);
        map2Type = GET_PARAM(2);
        interpolation = GET_PARAM(3);
        bordertype = GET_PARAM(4);

        Size srcSize = Size(MWIDTH, MHEIGHT);
        Size map1Size = Size(MWIDTH, MHEIGHT);
        double min = 5, max = 16;

        if(srcType != nulltype)
        {
            src = randomMat(srcSize, srcType, min, max, false);
        }
        if((map1Type == CV_16SC2 && map2Type == nulltype) || (map1Type == CV_32FC2 && map2Type == nulltype))
        {
            map1 = randomMat(map1Size, map1Type, min, max, false);
        }
        else if (map1Type == CV_32FC1 && map2Type == CV_32FC1)
        {
            map1 = randomMat(map1Size, map1Type, min, max, false);
            map2 = randomMat(map1Size, map1Type, min, max, false);
        }

        else
        {
            cout << "The wrong input type" << endl;
            return;
        }

        dst = randomMat(map1Size, srcType, min, max, false);
        switch (src.channels())
        {
        case 1:
            val = Scalar(rng.uniform(0.0, 10.0), 0, 0, 0);
            break;
        case 2:
            val = Scalar(rng.uniform(0.0, 10.0), rng.uniform(0.0, 10.0), 0, 0);
            break;
        case 3:
            val = Scalar(rng.uniform(0.0, 10.0), rng.uniform(0.0, 10.0), rng.uniform(0.0, 10.0), 0);
            break;
        case 4:
            val = Scalar(rng.uniform(0.0, 10.0), rng.uniform(0.0, 10.0), rng.uniform(0.0, 10.0), rng.uniform(0.0, 10.0));
            break;
        }

    }
    void random_roi()
    {
        dst_roicols = rng.uniform(1, dst.cols);
        dst_roirows = rng.uniform(1, dst.rows);

        src_roicols = rng.uniform(1, src.cols);
        src_roirows = rng.uniform(1, src.rows);


        srcx = rng.uniform(0, src.cols - src_roicols);
        srcy = rng.uniform(0, src.rows - src_roirows);
        dstx = rng.uniform(0, dst.cols - dst_roicols);
        dsty = rng.uniform(0, dst.rows - dst_roirows);
        map1_roicols = dst_roicols;
        map1_roirows = dst_roirows;
        map2_roicols = dst_roicols;
        map2_roirows = dst_roirows;
        map1x = dstx;
        map1y = dsty;
        map2x = dstx;
        map2y = dsty;

        if((map1Type == CV_16SC2 && map2Type == nulltype) || (map1Type == CV_32FC2 && map2Type == nulltype))
        {
            map1_roi = map1(Rect(map1x, map1y, map1_roicols, map1_roirows));
            gmap1_roi = map1_roi;
        }

        else if (map1Type == CV_32FC1 && map2Type == CV_32FC1)
        {
            map1_roi = map1(Rect(map1x, map1y, map1_roicols, map1_roirows));
            gmap1_roi = map1_roi;
            map2_roi = map2(Rect(map2x, map2y, map2_roicols, map2_roirows));
            gmap2_roi = map2_roi;
        }
        src_roi = src(Rect(srcx, srcy, src_roicols, src_roirows));
        dst_roi = dst(Rect(dstx, dsty, dst_roicols, dst_roirows));
        gsrc_roi = src_roi;
        gdst = dst;
        gdst_roi = gdst(Rect(dstx, dsty, dst_roicols, dst_roirows));
    }
};

OCL_TEST_P(Remap, Mat)
{
    if((interpolation == 1 && map1Type == CV_16SC2) || (map1Type == CV_32FC1 && map2Type == nulltype) || (map1Type == CV_16SC2 && map2Type == CV_32FC1) || (map1Type == CV_32FC2 && map2Type == CV_32FC1))
    {
        cout << "Don't support the dataType" << endl;
        return;
    }
    int bordertype[] = {BORDER_CONSTANT, BORDER_REPLICATE/*,BORDER_REFLECT,BORDER_WRAP,BORDER_REFLECT_101*/};

    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        remap(src_roi, dst_roi, map1_roi, map2_roi, interpolation, bordertype[0], val);
        ocl::remap(gsrc_roi, gdst_roi, gmap1_roi, gmap2_roi, interpolation, bordertype[0], val);
        Mat cpu_dst;
        gdst.download(cpu_dst);

        if(interpolation == 0)
            EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
        EXPECT_MAT_NEAR(dst, cpu_dst, 2.0);
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////////
// resize

PARAM_TEST_CASE(Resize, MatType, Size, double, double, int)
{
    int type;
    Size dsize;
    double fx, fy;
    int interpolation;

    //src mat
    Mat mat1;
    Mat dst;

    // set up roi
    int src_roicols;
    int src_roirows;
    int dst_roicols;
    int dst_roirows;
    int src1x;
    int src1y;
    int dstx;
    int dsty;

    //src mat with roi
    Mat mat1_roi;
    Mat dst_roi;

    //ocl dst mat for testing
    ocl::oclMat gdst_whole;

    //ocl mat with roi
    ocl::oclMat gmat1;
    ocl::oclMat gdst;

    virtual void SetUp()
    {
        type = GET_PARAM(0);
        dsize = GET_PARAM(1);
        fx = GET_PARAM(2);
        fy = GET_PARAM(3);
        interpolation = GET_PARAM(4);

        Size size(MWIDTH, MHEIGHT);

        if(dsize == Size() && !(fx > 0 && fy > 0))
        {
            cout << "invalid dsize and fx fy" << endl;
            return;
        }

        if(dsize == Size())
        {
            dsize.width = (int)(size.width * fx);
            dsize.height = (int)(size.height * fy);
        }

        mat1 = randomMat(size, type, 5, 16, false);
        dst  = randomMat(dsize, type, 5, 16, false);

    }

    void random_roi()
    {
#ifdef RANDOMROI
        //randomize ROI
        src_roicols = rng.uniform(1, mat1.cols);
        src_roirows = rng.uniform(1, mat1.rows);
        dst_roicols = (int)(src_roicols * fx);
        dst_roirows = (int)(src_roirows * fy);
        src1x   = rng.uniform(0, mat1.cols - src_roicols);
        src1y   = rng.uniform(0, mat1.rows - src_roirows);
        dstx    = rng.uniform(0, dst.cols  - dst_roicols);
        dsty    = rng.uniform(0, dst.rows  - dst_roirows);
#else
        src_roicols = mat1.cols;
        src_roirows = mat1.rows;
        dst_roicols = dst.cols;
        dst_roirows = dst.rows;
        src1x   = 0;
        src1y   = 0;
        dstx    = 0;
        dsty    = 0;
#endif
        dsize.width = dst_roicols;
        dsize.height = dst_roirows;
        mat1_roi = mat1(Rect(src1x, src1y, src_roicols, src_roirows));
        dst_roi  = dst(Rect(dstx, dsty, dst_roicols, dst_roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx, dsty, dst_roicols, dst_roirows));

        dsize.width = (int)(mat1_roi.size().width * fx);
        dsize.height = (int)(mat1_roi.size().height * fy);

        gmat1 = mat1_roi;
    }

};

OCL_TEST_P(Resize, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        // resize(mat1_roi, dst_roi, dsize, fx, fy, interpolation);
        // ocl::resize(gmat1, gdst, dsize, fx, fy, interpolation);
        if(dst_roicols < 1 || dst_roirows < 1) continue;
        resize(mat1_roi, dst_roi, dsize, fx, fy, interpolation);
        ocl::resize(gmat1, gdst, dsize, fx, fy, interpolation);

        Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
    }

}


/////////////////////////////////////////////////////////////////////////////////////////////////
//threshold

PARAM_TEST_CASE(Threshold, MatType, ThreshOp)
{
    int type;
    int threshOp;

    //src mat
    Mat mat1;
    Mat dst;

    // set up roi
    int roicols;
    int roirows;
    int src1x;
    int src1y;
    int dstx;
    int dsty;

    //src mat with roi
    Mat mat1_roi;
    Mat dst_roi;

    //ocl dst mat for testing
    ocl::oclMat gdst_whole;

    //ocl mat with roi
    ocl::oclMat gmat1;
    ocl::oclMat gdst;

    virtual void SetUp()
    {
        type = GET_PARAM(0);
        threshOp = GET_PARAM(1);

        Size size(MWIDTH, MHEIGHT);

        mat1 = randomMat(size, type, 5, 16, false);
        dst  = randomMat(size, type, 5, 16, false);
    }

    void random_roi()
    {
#ifdef RANDOMROI
        //randomize ROI
        roicols = rng.uniform(1, mat1.cols);
        roirows = rng.uniform(1, mat1.rows);
        src1x   = rng.uniform(0, mat1.cols - roicols);
        src1y   = rng.uniform(0, mat1.rows - roirows);
        dstx    = rng.uniform(0, dst.cols  - roicols);
        dsty    = rng.uniform(0, dst.rows  - roirows);
#else
        roicols = mat1.cols;
        roirows = mat1.rows;
        src1x   = 0;
        src1y   = 0;
        dstx    = 0;
        dsty    = 0;
#endif

        mat1_roi = mat1(Rect(src1x, src1y, roicols, roirows));
        dst_roi  = dst(Rect(dstx, dsty, roicols, roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx, dsty, roicols, roirows));


        gmat1 = mat1_roi;
    }

};

OCL_TEST_P(Threshold, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        double maxVal = randomDouble(20.0, 127.0);
        double thresh = randomDouble(0.0, maxVal);

        threshold(mat1_roi, dst_roi, thresh, maxVal, threshOp);
        ocl::threshold(gmat1, gdst, thresh, maxVal, threshOp);

        Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1);
    }

}

PARAM_TEST_CASE(MeanShiftTestBase, MatType, MatType, int, int, TermCriteria)
{
    int type, typeCoor;
    int sp, sr;
    TermCriteria crit;
    //src mat
    Mat src;
    Mat dst;
    Mat dstCoor;

    //set up roi
    int roicols;
    int roirows;
    int srcx;
    int srcy;
    int dstx;
    int dsty;

    //src mat with roi
    Mat src_roi;
    Mat dst_roi;
    Mat dstCoor_roi;

    //ocl dst mat
    ocl::oclMat gdst;
    ocl::oclMat gdstCoor;

    //ocl mat with roi
    ocl::oclMat gsrc_roi;
    ocl::oclMat gdst_roi;
    ocl::oclMat gdstCoor_roi;

    virtual void SetUp()
    {
        type     = GET_PARAM(0);
        typeCoor = GET_PARAM(1);
        sp       = GET_PARAM(2);
        sr       = GET_PARAM(3);
        crit     = GET_PARAM(4);

        // MWIDTH=256, MHEIGHT=256. defined in utility.hpp
        Size size = Size(MWIDTH, MHEIGHT);

        src = randomMat(size, type, 5, 16, false);
        dst = randomMat(size, type, 5, 16, false);
        dstCoor = randomMat(size, typeCoor, 5, 16, false);

    }

    void random_roi()
    {
#ifdef RANDOMROI
        //randomize ROI
        roicols = rng.uniform(1, src.cols);
        roirows = rng.uniform(1, src.rows);
        srcx = rng.uniform(0, src.cols - roicols);
        srcy = rng.uniform(0, src.rows - roirows);
        dstx = rng.uniform(0, dst.cols - roicols);
        dsty = rng.uniform(0, dst.rows - roirows);
#else
        roicols = src.cols;
        roirows = src.rows;
        srcx = 0;
        srcy = 0;
        dstx = 0;
        dsty = 0;
#endif
        src_roi = src(Rect(srcx, srcy, roicols, roirows));
        dst_roi = dst(Rect(dstx, dsty, roicols, roirows));
        dstCoor_roi = dstCoor(Rect(dstx, dsty, roicols, roirows));

        gdst = dst;
        gdstCoor = dstCoor;

        gsrc_roi = src_roi;
        gdst_roi = gdst(Rect(dstx, dsty, roicols, roirows));  //gdst_roi
        gdstCoor_roi = gdstCoor(Rect(dstx, dsty, roicols, roirows));
    }
};

/////////////////////////meanShiftFiltering/////////////////////////////

typedef MeanShiftTestBase MeanShiftFiltering;

OCL_TEST_P(MeanShiftFiltering, Mat)
{

    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        Mat cpu_gdst;
        gdst.download(cpu_gdst);

        ::meanShiftFiltering_(src_roi, dst_roi, sp, sr, crit);
        ocl::meanShiftFiltering(gsrc_roi, gdst_roi, sp, sr, crit);

        gdst.download(cpu_gdst);
        EXPECT_MAT_NEAR(dst, cpu_gdst, 0.0);
    }
}

///////////////////////////meanShiftProc//////////////////////////////////

typedef MeanShiftTestBase MeanShiftProc;

OCL_TEST_P(MeanShiftProc, Mat)
{

    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        Mat cpu_gdst;
        Mat cpu_gdstCoor;

        meanShiftProc_(src_roi, dst_roi, dstCoor_roi, sp, sr, crit);
        ocl::meanShiftProc(gsrc_roi, gdst_roi, gdstCoor_roi, sp, sr, crit);

        gdst.download(cpu_gdst);
        gdstCoor.download(cpu_gdstCoor);
        EXPECT_MAT_NEAR(dst, cpu_gdst, 0.0);
        EXPECT_MAT_NEAR(dstCoor, cpu_gdstCoor, 0.0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
//hist

void calcHistGold(const Mat &src, Mat &hist)
{
    hist.create(1, 256, CV_32SC1);
    hist.setTo(Scalar::all(0));

    int *hist_row = hist.ptr<int>();
    for (int y = 0; y < src.rows; ++y)
    {
        const uchar *src_row = src.ptr(y);

        for (int x = 0; x < src.cols; ++x)
            ++hist_row[src_row[x]];
    }
}

PARAM_TEST_CASE(HistTestBase, MatType, MatType)
{
    int type_src;

    //src mat
    Mat src;
    Mat dst_hist;
    //set up roi
    int roicols;
    int roirows;
    int srcx;
    int srcy;
    //src mat with roi
    Mat src_roi;
    //ocl dst mat, dst_hist and gdst_hist don't have roi
    ocl::oclMat gdst_hist;
    //ocl mat with roi
    ocl::oclMat gsrc_roi;

    virtual void SetUp()
    {
        type_src   = GET_PARAM(0);

        Size size = Size(MWIDTH, MHEIGHT);

        src = randomMat(size, type_src, 0, 256, false);

    }

    void random_roi()
    {
#ifdef RANDOMROI
        //randomize ROI
        roicols = rng.uniform(1, src.cols);
        roirows = rng.uniform(1, src.rows);
        srcx = rng.uniform(0, src.cols - roicols);
        srcy = rng.uniform(0, src.rows - roirows);
#else
        roicols = src.cols;
        roirows = src.rows;
        srcx = 0;
        srcy = 0;
#endif
        src_roi = src(Rect(srcx, srcy, roicols, roirows));

        gsrc_roi = src_roi;
    }
};

///////////////////////////calcHist///////////////////////////////////////

typedef HistTestBase CalcHist;

OCL_TEST_P(CalcHist, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        Mat cpu_hist;

        calcHistGold(src_roi, dst_hist);
        ocl::calcHist(gsrc_roi, gdst_hist);

        gdst_hist.download(cpu_hist);
        EXPECT_MAT_NEAR(dst_hist, cpu_hist, 0.0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// CLAHE

PARAM_TEST_CASE(CLAHE_Test, Size, double)
{
    Size gridSize;
    double clipLimit;

    Mat src;
    Mat dst_gold;

    ocl::oclMat g_src;
    ocl::oclMat g_dst;

    virtual void SetUp()
    {
        gridSize = GET_PARAM(0);
        clipLimit = GET_PARAM(1);

        src = randomMat(Size(MWIDTH, MHEIGHT), CV_8UC1, 0, 256, false);
        g_src.upload(src);
    }
};

OCL_TEST_P(CLAHE_Test, Accuracy)
{
    Ptr<CLAHE> clahe = ocl::createCLAHE(clipLimit, gridSize);
    clahe->apply(g_src, g_dst);
    Mat dst(g_dst);

    Ptr<CLAHE> clahe_gold = createCLAHE(clipLimit, gridSize);
    clahe_gold->apply(src, dst_gold);

    EXPECT_MAT_NEAR(dst_gold, dst, 1.0);
}

///////////////////////////Convolve//////////////////////////////////

PARAM_TEST_CASE(ConvolveTestBase, MatType, bool)
{
    int type;
    //src mat
    Mat mat1;
    Mat mat2;
    Mat dst;
    Mat dst1; //bak, for two outputs
    // set up roi
    int roicols;
    int roirows;
    int src1x;
    int src1y;
    int src2x;
    int src2y;
    int dstx;
    int dsty;
    //src mat with roi
    Mat mat1_roi;
    Mat mat2_roi;
    Mat dst_roi;
    Mat dst1_roi; //bak
    //ocl dst mat for testing
    ocl::oclMat gdst_whole;
    ocl::oclMat gdst1_whole; //bak
    //ocl mat with roi
    ocl::oclMat gmat1;
    ocl::oclMat gmat2;
    ocl::oclMat gdst;
    ocl::oclMat gdst1;   //bak
    virtual void SetUp()
    {
        type = GET_PARAM(0);

        Size size(MWIDTH, MHEIGHT);

        mat1 = randomMat(size, type, 5, 16, false);
        mat2 = randomMat(size, type, 5, 16, false);
        dst  = randomMat(size, type, 5, 16, false);
        dst1  = randomMat(size, type, 5, 16, false);
    }
    void random_roi()
    {
#ifdef RANDOMROI
        //randomize ROI
        roicols = rng.uniform(1, mat1.cols);
        roirows = rng.uniform(1, mat1.rows);
        src1x   = rng.uniform(0, mat1.cols - roicols);
        src1y   = rng.uniform(0, mat1.rows - roirows);
        dstx    = rng.uniform(0, dst.cols  - roicols);
        dsty    = rng.uniform(0, dst.rows  - roirows);
#else
        roicols = mat1.cols;
        roirows = mat1.rows;
        src1x = 0;
        src1y = 0;
        dstx = 0;
        dsty = 0;
#endif
        src2x   = rng.uniform(0, mat2.cols - roicols);
        src2y   = rng.uniform(0, mat2.rows - roirows);
        mat1_roi = mat1(Rect(src1x, src1y, roicols, roirows));
        mat2_roi = mat2(Rect(src2x, src2y, roicols, roirows));
        dst_roi  = dst(Rect(dstx, dsty, roicols, roirows));
        dst1_roi = dst1(Rect(dstx, dsty, roicols, roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx, dsty, roicols, roirows));

        gdst1_whole = dst1;
        gdst1 = gdst1_whole(Rect(dstx, dsty, roicols, roirows));

        gmat1 = mat1_roi;
        gmat2 = mat2_roi;
        //end
    }

};

typedef ConvolveTestBase Convolve;

void conv2( Mat x, Mat y, Mat z)
{
    int N1 = x.rows;
    int M1 = x.cols;
    int N2 = y.rows;
    int M2 = y.cols;

    int i, j;
    int m, n;


    float *kerneldata = (float *)(x.data);
    float *srcdata = (float *)(y.data);
    float *dstdata = (float *)(z.data);

    for(i = 0; i < N2; i++)
        for(j = 0; j < M2; j++)
        {
            float temp = 0;
            for(m = 0; m < N1; m++)
                for(n = 0; n < M1; n++)
                {
                    int r, c;
                    r = min(max((i - N1 / 2 + m), 0), N2 - 1);
                    c = min(max((j - M1 / 2 + n), 0), M2 - 1);
                    temp += kerneldata[m * (x.step >> 2) + n] * srcdata[r * (y.step >> 2) + c];
                }
            dstdata[i * (z.step >> 2) + j] = temp;
        }
}

OCL_TEST_P(Convolve, Mat)
{
    if(mat1.type() != CV_32FC1)
    {
        cout << "\tUnsupported type\t\n";
    }
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        ocl::oclMat temp1;
        Mat kernel_cpu = mat2(Rect(0, 0, 7, 7));
        temp1 = kernel_cpu;

        conv2(kernel_cpu, mat1_roi, dst_roi);
        ocl::convolve(gmat1, temp1, gdst);

        Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        EXPECT_MAT_NEAR(dst, cpu_dst, .1);

    }
}

//////////////////////////////// ColumnSum //////////////////////////////////////

PARAM_TEST_CASE(ColumnSum, Size)
{
    Size size;
    Mat src;

    virtual void SetUp()
    {
        size = GET_PARAM(0);
    }
};

OCL_TEST_P(ColumnSum, Accuracy)
{
    Mat src = randomMat(size, CV_32FC1, 0, 255);
    ocl::oclMat d_dst;
    ocl::oclMat d_src(src);

    ocl::columnSum(d_src, d_dst);

    Mat dst(d_dst);

    for (int j = 0; j < src.cols; ++j)
    {
        float gold = src.at<float>(0, j);
        float res = dst.at<float>(0, j);
        ASSERT_NEAR(res, gold, 1e-5);
    }

    for (int i = 1; i < src.rows; ++i)
    {
        for (int j = 0; j < src.cols; ++j)
        {
            float gold = src.at<float>(i, j) += src.at<float>(i - 1, j);
            float res = dst.at<float>(i, j);
            ASSERT_NEAR(res, gold, 1e-5);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////

INSTANTIATE_TEST_CASE_P(ImgprocTestBase, EqualizeHist, Combine(
                            ONE_TYPE(CV_8UC1),
                            NULL_TYPE,
                            ONE_TYPE(CV_8UC1),
                            NULL_TYPE,
                            NULL_TYPE,
                            Values(false))); // Values(false) is the reserved parameter


INSTANTIATE_TEST_CASE_P(ImgprocTestBase, CopyMakeBorder, Combine(
                            Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32SC1, CV_32SC3, CV_32SC4, CV_32FC1, CV_32FC3, CV_32FC4),
                            NULL_TYPE,
                            Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32SC1, CV_32SC3, CV_32SC4, CV_32FC1, CV_32FC3, CV_32FC4),
                            NULL_TYPE,
                            NULL_TYPE,
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(ImgprocTestBase, CornerMinEigenVal, Combine(
                            Values(CV_8UC1, CV_32FC1),
                            NULL_TYPE,
                            ONE_TYPE(CV_32FC1),
                            NULL_TYPE,
                            NULL_TYPE,
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(ImgprocTestBase, CornerHarris, Combine(
                            Values(CV_8UC1, CV_32FC1),
                            NULL_TYPE,
                            ONE_TYPE(CV_32FC1),
                            NULL_TYPE,
                            NULL_TYPE,
                            Values(false))); // Values(false) is the reserved parameter


INSTANTIATE_TEST_CASE_P(ImgprocTestBase, Integral, Combine(
                            ONE_TYPE(CV_8UC1),
                            NULL_TYPE,
                            ONE_TYPE(CV_32SC1),
                            ONE_TYPE(CV_32FC1),
                            NULL_TYPE,
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Imgproc, WarpAffine, Combine(
                            Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1, CV_32FC3, CV_32FC4),
                            Values((MatType)INTER_NEAREST, (MatType)INTER_LINEAR,
                                   (MatType)INTER_CUBIC, (MatType)(INTER_NEAREST | WARP_INVERSE_MAP),
                                   (MatType)(INTER_LINEAR | WARP_INVERSE_MAP), (MatType)(INTER_CUBIC | WARP_INVERSE_MAP))));


INSTANTIATE_TEST_CASE_P(Imgproc, WarpPerspective, Combine
                        (Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1, CV_32FC3, CV_32FC4),
                         Values((MatType)INTER_NEAREST, (MatType)INTER_LINEAR,
                                (MatType)INTER_CUBIC, (MatType)(INTER_NEAREST | WARP_INVERSE_MAP),
                                (MatType)(INTER_LINEAR | WARP_INVERSE_MAP), (MatType)(INTER_CUBIC | WARP_INVERSE_MAP))));


INSTANTIATE_TEST_CASE_P(Imgproc, Resize, Combine(
                            Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1, CV_32FC3, CV_32FC4),  Values(Size()),
                            Values(0.5, 1.5, 2), Values(0.5, 1.5, 2), Values((MatType)INTER_NEAREST, (MatType)INTER_LINEAR)));


INSTANTIATE_TEST_CASE_P(Imgproc, Threshold, Combine(
                            Values(CV_8UC1, CV_32FC1), Values(ThreshOp(THRESH_BINARY),
                                    ThreshOp(THRESH_BINARY_INV), ThreshOp(THRESH_TRUNC),
                                    ThreshOp(THRESH_TOZERO), ThreshOp(THRESH_TOZERO_INV))));


INSTANTIATE_TEST_CASE_P(Imgproc, MeanShiftFiltering, Combine(
                            ONE_TYPE(CV_8UC4),
                            ONE_TYPE(CV_16SC2),
                            Values(5),
                            Values(6),
                            Values(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 5, 1))
                        ));


INSTANTIATE_TEST_CASE_P(Imgproc, MeanShiftProc, Combine(
                            ONE_TYPE(CV_8UC4),
                            ONE_TYPE(CV_16SC2),
                            Values(5),
                            Values(6),
                            Values(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 5, 1))
                        ));

INSTANTIATE_TEST_CASE_P(Imgproc, Remap, Combine(
                            Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1, CV_32FC3, CV_32FC4),
                            Values(CV_32FC1, CV_16SC2, CV_32FC2), Values(-1, CV_32FC1),
                            Values((int)INTER_NEAREST, (int)INTER_LINEAR),
                            Values((int)BORDER_CONSTANT)));


INSTANTIATE_TEST_CASE_P(histTestBase, CalcHist, Combine(
                            ONE_TYPE(CV_8UC1),
                            ONE_TYPE(CV_32SC1) //no use
                        ));

INSTANTIATE_TEST_CASE_P(Imgproc, CLAHE_Test, Combine(
                        Values(Size(4, 4), Size(32, 8), Size(8, 64)),
                        Values(0.0, 10.0, 62.0, 300.0)));

INSTANTIATE_TEST_CASE_P(Imgproc, ColumnSum, DIFFERENT_SIZES);

#endif // HAVE_OPENCL
