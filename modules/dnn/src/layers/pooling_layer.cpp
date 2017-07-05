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
#include "opencv2/core/hal/intrin.hpp"
#include "op_halide.hpp"
#include "opencl_kernels_dnn.hpp"
#include <float.h>
#include <algorithm>
using std::max;
using std::min;

namespace cv
{
namespace dnn
{

class PoolingLayerImpl : public PoolingLayer
{
public:
    PoolingLayerImpl(const LayerParams& params)
    {
        type = PoolingLayer::MAX;
        computeMaxIdx = true;

        if (params.has("pool"))
        {
            String pool = params.get<String>("pool").toLowerCase();
            if (pool == "max")
                type = PoolingLayer::MAX;
            else if (pool == "ave")
                type = PoolingLayer::AVE;
            else if (pool == "stochastic")
                type = PoolingLayer::STOCHASTIC;
            else
                CV_Error(Error::StsBadArg, "Unknown pooling type \"" + pool + "\"");
        }

        getPoolingKernelParams(params, kernel.height, kernel.width, globalPooling,
                               pad.height, pad.width, stride.height, stride.width, padMode);
        setParamsFrom(params);
        ceilMode = params.get<bool>("ceil_mode", true);
    }

#ifdef HAVE_OPENCL
    Ptr<greentea::LibDNNPool<float>> poolOp;
#endif

    void finalize(const std::vector<Mat*> &inputs, std::vector<Mat> &outputs)
    {
        CV_Assert(inputs.size() == 1);

        cv::Size inp(inputs[0]->size[3], inputs[0]->size[2]),
                out(outputs[0].size[3], outputs[0].size[2]);

        if(globalPooling)
        {
            kernel = inp;
        }

        getConvPoolPaddings(inp, out, kernel, stride, padMode, Size(1, 1), pad);
    }

    virtual bool supportBackend(int backendId)
    {
        return backendId == DNN_BACKEND_DEFAULT ||
               backendId == DNN_BACKEND_HALIDE && haveHalide() &&
               (type == PoolingLayer::MAX ||
                type == PoolingLayer::AVE && !pad.width && !pad.height);
    }

#ifdef HAVE_OPENCL
    bool forward_ocl(std::vector<Mat*> &inputs, std::vector<Mat> &outputs, std::vector<Mat> &internals)
    {
        if (poolOp.empty())
        {
            greentea::LibDNNPoolConfig config;

            config.in_shape = {inputs[0]->size[0], inputs[0]->size[1], inputs[0]->size[2], inputs[0]->size[3]};
            config.out_shape = {outputs[0].size[0], outputs[0].size[1], outputs[0].size[2], outputs[0].size[3]};
            config.kernel = {kernel.height, kernel.width};
            config.pad = {pad.height, pad.width};
            config.stride = {stride.height, stride.width};
            config.channels = inputs[0]->size[1];
            config.pool_method = type == MAX ? greentea::LIBDNN_POOLING_METHOD_MAX :
                                (type == AVE ? greentea::LIBDNN_POOLING_METHOD_AVE :
                                               greentea::LIBDNN_POOLING_METHOD_STO);
            poolOp = Ptr<greentea::LibDNNPool<float>>(new greentea::LibDNNPool<float>(config));
        }

        for (size_t ii = 0; ii < inputs.size(); ii++)
        {
            UMat inpMat, outMat, maskMat;
            cl_mem in_mem, out_mem, mask_mem = NULL;

            inpMat = inputs[ii]->getUMat(ACCESS_READ);
            in_mem = (cl_mem)inpMat.handle(ACCESS_READ);

            if (type == MAX)
            {
                outMat = outputs[2 * ii].getUMat(ACCESS_WRITE);
                out_mem = (cl_mem)outMat.handle(ACCESS_WRITE);
                maskMat = outputs[2 * ii + 1].getUMat(ACCESS_WRITE);
                mask_mem = (cl_mem)maskMat.handle(ACCESS_WRITE);
            } else {
                outMat = outputs[ii].getUMat(ACCESS_WRITE);
                out_mem = (cl_mem)outMat.handle(ACCESS_WRITE);
            }

            CV_Assert(inpMat.offset == 0 && outMat.offset == 0);

            if (!poolOp->Forward((float *)in_mem, (float *)out_mem, (float *)mask_mem))
                return false;
        }

        return true;
    }
#endif

    void forward(std::vector<Mat*> &inputs, std::vector<Mat> &outputs, std::vector<Mat> &internals)
    {
        CV_TRACE_FUNCTION();
        CV_TRACE_ARG_VALUE(name, "name", name.c_str());

        CV_OCL_RUN(ocl::Device::getDefault().isIntel(),
                   forward_ocl(inputs, outputs, internals))

        for (size_t ii = 0; ii < inputs.size(); ii++)
        {
            switch (type)
            {
                case MAX:
                    maxPooling(*inputs[ii], outputs[2 * ii], outputs[2 * ii + 1]);
                    break;
                case AVE:
                    avePooling(*inputs[ii], outputs[ii]);
                    break;
                default:
                    CV_Error(Error::StsNotImplemented, "Not implemented");
                    break;
            }
        }
    }

    virtual Ptr<BackendNode> initHalide(const std::vector<Ptr<BackendWrapper> > &inputs)
    {
        if (type == PoolingLayer::MAX)
            return initMaxPoolingHalide(inputs);
        else if (type == PoolingLayer::AVE)
            return initAvePoolingHalide(inputs);
        else
            return Ptr<BackendNode>();
    }

    class PoolingInvoker : public ParallelLoopBody
    {
    public:
        const Mat* src;
        Mat *dst, *mask;
        Size kernel, stride, pad;
        int nstripes;
        bool computeMaxIdx;
        std::vector<int> ofsbuf;
        int poolingType;

        PoolingInvoker() : src(0), dst(0), mask(0), nstripes(0), computeMaxIdx(0), poolingType(PoolingLayer::MAX) {}

        static void run(const Mat& src, Mat& dst, Mat& mask, Size kernel,
                        Size stride, Size pad, int poolingType,
                        bool computeMaxIdx, int nstripes)
        {
            CV_Assert(src.isContinuous() && dst.isContinuous() &&
                      src.type() == CV_32F && src.type() == dst.type() &&
                      src.dims == 4 && dst.dims == 4 &&
                      src.size[0] == dst.size[0] && src.size[1] == dst.size[1] &&
                      (mask.empty() || (mask.type() == src.type() && mask.size == dst.size)));

            PoolingInvoker p;

            p.src = &src;
            p.dst = &dst;
            p.mask = &mask;
            p.kernel = kernel;
            p.stride = stride;
            p.pad = pad;
            p.nstripes = nstripes;
            p.computeMaxIdx = computeMaxIdx;
            p.poolingType = poolingType;

            if( !computeMaxIdx )
            {
                p.ofsbuf.resize(kernel.width*kernel.height);
                for( int i = 0; i < kernel.height; i++ )
                    for( int j = 0; j < kernel.width; j++ )
                        p.ofsbuf[i*kernel.width + j] = src.size[3]*i + j;
            }

            parallel_for_(Range(0, nstripes), p, nstripes);
        }

        void operator()(const Range& r) const
        {
            int channels = dst->size[1], width = dst->size[3], height = dst->size[2];
            int inp_width = src->size[3], inp_height = src->size[2];
            size_t total = dst->total();
            size_t stripeSize = (total + nstripes - 1)/nstripes;
            size_t stripeStart = r.start*stripeSize;
            size_t stripeEnd = std::min(r.end*stripeSize, total);
            int kernel_w = kernel.width, kernel_h = kernel.height;
            int pad_w = pad.width, pad_h = pad.height;
            int stride_w = stride.width, stride_h = stride.height;
            bool compMaxIdx = computeMaxIdx;

#if CV_SIMD128
            const int* ofsptr = &ofsbuf[0];
            v_float32x4 idx00(0.f, (float)stride_w, (float)(stride_w*2), (float)(stride_w*3));
            v_float32x4 ones = v_setall_f32(1.f);
            v_float32x4 idx_delta = v_setall_f32((float)(inp_width - kernel_w));
#endif

            for( size_t ofs0 = stripeStart; ofs0 < stripeEnd; )
            {
                size_t ofs = ofs0;
                int x0 = (int)(ofs % width);
                ofs /= width;
                int y0 = (int)(ofs % height);
                ofs /= height;
                int c = (int)(ofs % channels);
                int n = (int)(ofs / channels);
                int ystart = y0 * stride_h - pad_h;
                int yend = min(ystart + kernel_h, inp_height + pad_h);
                int ydelta = yend - ystart;
                ystart = max(ystart, 0);
                yend = min(yend, inp_height);
                const float *srcData = src->ptr<float>(n, c);
                float *dstData = dst->ptr<float>(n, c, y0);
                float *dstMaskData = mask->data ? mask->ptr<float>(n, c, y0) : 0;

                int delta = std::min((int)(stripeEnd - ofs0), width - x0);
                ofs0 += delta;
                int x1 = x0 + delta;

                if( poolingType == PoolingLayer::MAX )
                    for( ; x0 < x1; x0++ )
                    {
                        int xstart = x0 * stride_w - pad_w;
                        int xend = min(xstart + kernel_w, inp_width);
                        xstart = max(xstart, 0);

#if CV_SIMD128
                        if( xstart > 0 && x0 + 7 < x1 && (x0 + 7) * stride_w - pad_w + kernel_w < inp_width )
                        {
                            if( compMaxIdx )
                            {
                                v_float32x4 max_val0 = v_setall_f32(-FLT_MAX);
                                v_float32x4 max_val1 = max_val0;
                                v_float32x4 max_idx0 = v_setall_f32(-1.f);
                                v_float32x4 max_idx1 = max_idx0;
                                int index0 = ystart * inp_width + xstart;
                                v_float32x4 idx0 = idx00 + v_setall_f32((float)index0);
                                v_float32x4 idx1 = idx0 + v_setall_f32((float)(stride_w*4));

                                for (int y = ystart; y < yend; ++y)
                                {
                                    for (int x = xstart; x < xend; ++x, idx0 += ones, idx1 += ones)
                                    {
                                        const int index = y * inp_width + x;
                                        v_float32x4 v0(srcData[index], srcData[index + stride_w],
                                                       srcData[index + stride_w*2], srcData[index + stride_w*3]);
                                        v_float32x4 v1(srcData[index + stride_w*4], srcData[index + stride_w*5],
                                                       srcData[index + stride_w*6], srcData[index + stride_w*7]);
                                        max_idx0 = v_select(v0 > max_val0, idx0, max_idx0);
                                        max_idx1 = v_select(v1 > max_val1, idx1, max_idx1);
                                        max_val0 = v_max(max_val0, v0);
                                        max_val1 = v_max(max_val1, v1);
                                    }
                                    idx0 += idx_delta;
                                    idx1 += idx_delta;
                                }
                                v_store(dstData + x0, max_val0);
                                v_store(dstData + x0 + 4, max_val1);
                                if (dstMaskData)
                                {
                                    v_store(dstMaskData + x0, max_idx0);
                                    v_store(dstMaskData + x0 + 4, max_idx1);
                                }
                                x0 += 7;
                            }
                            else
                            {
                                v_float32x4 max_val0 = v_setall_f32(-FLT_MAX);
                                v_float32x4 max_val1 = max_val0;

                                if( yend - ystart == kernel_h )
                                {
                                    const float* srcData1 = srcData + ystart*inp_width + xstart;
                                    if( stride_w == 1 )
                                        for (int k = 0; k < kernel_w*kernel_h; k++)
                                        {
                                            int index = ofsptr[k];
                                            v_float32x4 v0 = v_load(srcData1 + index);
                                            v_float32x4 v1 = v_load(srcData1 + index + 4);
                                            max_val0 = v_max(max_val0, v0);
                                            max_val1 = v_max(max_val1, v1);
                                        }
#if CV_SSE2
                                    else if( stride_w == 2 )
                                        for (int k = 0; k < kernel_w*kernel_h; k++)
                                        {
                                            int index = ofsptr[k];
                                            v_float32x4 v00 = v_load(srcData1 + index), v01 = v_load(srcData1 + index + 4);
                                            v_float32x4 v0(_mm_shuffle_ps(v00.val, v01.val, _MM_SHUFFLE(2, 0, 2, 0)));
                                            v_float32x4 v10 = v_load(srcData1 + index + 8), v11 = v_load(srcData1 + index + 12);
                                            v_float32x4 v1(_mm_shuffle_ps(v10.val, v11.val, _MM_SHUFFLE(2, 0, 2, 0)));
                                            max_val0 = v_max(max_val0, v0);
                                            max_val1 = v_max(max_val1, v1);
                                        }
#endif
                                    else
                                        for (int k = 0; k < kernel_w*kernel_h; k++)
                                        {
                                            int index = ofsptr[k];
                                            v_float32x4 v0(srcData1[index], srcData1[index + stride_w],
                                                           srcData1[index + stride_w*2], srcData1[index + stride_w*3]);
                                            v_float32x4 v1(srcData1[index + stride_w*4], srcData1[index + stride_w*5],
                                                           srcData1[index + stride_w*6], srcData1[index + stride_w*7]);
                                            max_val0 = v_max(max_val0, v0);
                                            max_val1 = v_max(max_val1, v1);
                                        }
                                }
                                else
                                {
                                    for (int y = ystart; y < yend; ++y)
                                    {
                                        for (int x = xstart; x < xend; ++x)
                                        {
                                            const int index = y * inp_width + x;
                                            v_float32x4 v0(srcData[index], srcData[index + stride_w],
                                                           srcData[index + stride_w*2], srcData[index + stride_w*3]);
                                            v_float32x4 v1(srcData[index + stride_w*4], srcData[index + stride_w*5],
                                                           srcData[index + stride_w*6], srcData[index + stride_w*7]);
                                            max_val0 = v_max(max_val0, v0);
                                            max_val1 = v_max(max_val1, v1);
                                        }
                                    }
                                }
                                v_store(dstData + x0, max_val0);
                                v_store(dstData + x0 + 4, max_val1);
                                x0 += 7;
                            }
                        }
                        else
#endif
                        {
                            float max_val = -FLT_MAX;
                            if( compMaxIdx )
                            {
                                int max_index = -1;
                                for (int y = ystart; y < yend; ++y)
                                    for (int x = xstart; x < xend; ++x)
                                    {
                                        const int index = y * inp_width + x;
                                        float val = srcData[index];
                                        if (val > max_val)
                                        {
                                            max_val = val;
                                            max_index = index;
                                        }
                                    }

                                dstData[x0] = max_val;
                                if (dstMaskData)
                                    dstMaskData[x0] = max_index;
                            }
                            else
                            {
                                for (int y = ystart; y < yend; ++y)
                                    for (int x = xstart; x < xend; ++x)
                                    {
                                        const int index = y * inp_width + x;
                                        float val = srcData[index];
                                        max_val = std::max(max_val, val);
                                    }

                                dstData[x0] = max_val;
                            }
                        }
                    }
                else
                {
                    for( ; x0 < x1; x0++ )
                    {
                        int xstart = x0 * stride_w - pad_w;
                        int xend = min(xstart + kernel_w, inp_width + pad_w);
                        int xdelta = xend - xstart;
                        xstart = max(xstart, 0);
                        xend = min(xend, inp_width);
                        float inv_kernel_area = 1.f/(ydelta*xdelta);

#if CV_SIMD128
                        if( xstart > 0 && x0 + 7 < x1 && (x0 + 7) * stride_w - pad_w + kernel_w < inp_width )
                        {
                            v_float32x4 sum_val0 = v_setzero_f32(), sum_val1 = v_setzero_f32();
                            v_float32x4 ikarea = v_setall_f32(inv_kernel_area);

                            for (int y = ystart; y < yend; ++y)
                            {
                                for (int x = xstart; x < xend; ++x)
                                {
                                    const int index = y * inp_width + x;
                                    v_float32x4 v0(srcData[index], srcData[index + stride_w],
                                                   srcData[index + stride_w*2], srcData[index + stride_w*3]);
                                    v_float32x4 v1(srcData[index + stride_w*4], srcData[index + stride_w*5],
                                                   srcData[index + stride_w*6], srcData[index + stride_w*7]);
                                    sum_val0 += v0;
                                    sum_val1 += v1;
                                }
                            }
                            v_store(dstData + x0, sum_val0*ikarea);
                            v_store(dstData + x0 + 4, sum_val1*ikarea);
                            x0 += 7;
                        }
                        else
#endif
                        {
                            float sum_val = 0.f;
                            for (int y = ystart; y < yend; ++y)
                                for (int x = xstart; x < xend; ++x)
                                {
                                    const int index = y * inp_width + x;
                                    float val = srcData[index];
                                    sum_val += val;
                                }

                            dstData[x0] = sum_val*inv_kernel_area;
                        }
                    }
                }
            }
        }
    };

    void maxPooling(Mat &src, Mat &dst, Mat &mask)
    {
        const int nstripes = getNumThreads();
        PoolingInvoker::run(src, dst, mask, kernel, stride, pad, type, computeMaxIdx, nstripes);
    }

    void avePooling(Mat &src, Mat &dst)
    {
        const int nstripes = getNumThreads();
        Mat mask;
        PoolingInvoker::run(src, dst, mask, kernel, stride, pad, type, computeMaxIdx, nstripes);
    }

    virtual Ptr<BackendNode> initMaxPoolingHalide(const std::vector<Ptr<BackendWrapper> > &inputs)
    {
#ifdef HAVE_HALIDE
        Halide::Buffer<float> inputBuffer = halideBuffer(inputs[0]);
        const int inWidth = inputBuffer.width();
        const int inHeight = inputBuffer.height();

        Halide::Var x("x"), y("y"), c("c"), n("n");
        Halide::Func top = (name.empty() ? Halide::Func() : Halide::Func(name));
        Halide::RDom r(0, kernel.width, 0, kernel.height);
        Halide::Expr kx, ky;
        if (pad.width || pad.height)
        {
            kx = clamp(x * stride.width + r.x - pad.width, 0, inWidth - 1);
            ky = clamp(y * stride.height + r.y - pad.height, 0, inHeight - 1);
        }
        else
        {
            kx = min(x * stride.width + r.x, inWidth - 1);
            ky = min(y * stride.height + r.y, inHeight - 1);
        }

        // Halide::argmax returns tuple (r.x, r.y, max).
        Halide::Tuple res = argmax(inputBuffer(kx, ky, c, n));

        // Compute offset from argmax in range [0, kernel_size).
        Halide::Expr max_index;
        if (pad.width || pad.height)
        {
            max_index = clamp(y * stride.height + res[1] - pad.height,
                              0, inHeight - 1) * inWidth +
                        clamp(x * stride.width + res[0] - pad.width,
                              0, inWidth - 1);
        }
        else
        {
            max_index = min(y * stride.height + res[1], inHeight - 1) * inWidth +
                        min(x * stride.width + res[0], inWidth - 1);
        }
        top(x, y, c, n) = { res[2], Halide::cast<float>(max_index) };
        return Ptr<BackendNode>(new HalideBackendNode(top));
#endif  // HAVE_HALIDE
        return Ptr<BackendNode>();
    }

    virtual Ptr<BackendNode> initAvePoolingHalide(const std::vector<Ptr<BackendWrapper> > &inputs)
    {
#ifdef HAVE_HALIDE
        Halide::Buffer<float> inputBuffer = halideBuffer(inputs[0]);

        const int inW = inputBuffer.width(), inH = inputBuffer.height();
        if ((inW - kernel.width) % stride.width || (inH - kernel.height) % stride.height)
        {
            CV_Error(cv::Error::StsNotImplemented,
                     "Halide backend for average pooling with partial "
                     "kernels is not implemented");
        }

        const float norm = 1.0f / (kernel.width * kernel.height);

        Halide::Var x("x"), y("y"), c("c"), n("n");
        Halide::Func top = (name.empty() ? Halide::Func() : Halide::Func(name));
        Halide::RDom r(0, kernel.width, 0, kernel.height);
        top(x, y, c, n) = sum(
            inputBuffer(x * stride.width + r.x,
                        y * stride.height + r.y, c, n)) * norm;
        return Ptr<BackendNode>(new HalideBackendNode(top));
#endif  // HAVE_HALIDE
        return Ptr<BackendNode>();
    }

    virtual void applyHalideScheduler(Ptr<BackendNode>& node,
                                      const std::vector<Mat*> &inputs,
                                      const std::vector<Mat> &outputs,
                                      int targetId) const
    {
#ifdef  HAVE_HALIDE
        if (targetId != DNN_TARGET_CPU)
        {
            Layer::applyHalideScheduler(node, inputs, outputs, targetId);
            return;
        }
        Halide::Var x("x"), y("y"), c("c"), n("n"), tile("tile"),
                    xi("xi"), yi("yi"), ci("ci"), xo("xo"), yo("yo"), co("co");
        Halide::Func& top = node.dynamicCast<HalideBackendNode>()->funcs.back();

        int outW, outH, outC, outN;
        getCanonicalSize(outputs[0].size, &outW, &outH, &outC, &outN);

        if (outW < 8 || outH < 8)
        {
            if (outC > 8)
                top.split(c, co, ci, 8)
                   .fuse(x, y, tile).fuse(co, tile, tile).fuse(n, tile, tile)
                   .parallel(tile)
                   .vectorize(ci);
            else
            {
                top.fuse(y, c, tile).fuse(n, tile, tile)
                   .parallel(tile);
                if (outW > 1)
                    top.vectorize(x);
            }
        }
        else
        {
            if (outC > 8)
                top.split(x, xo, xi, 8).split(y, yo, yi, 8).split(c, co, ci, 8)
                   .fuse(xo, yo, tile).fuse(co, tile, tile).fuse(n, tile, tile)
                   .parallel(tile)
                   .vectorize(xi);
            else
                top.split(x, xo, xi, 8).split(y, yo, yi, 8)
                   .fuse(xo, yo, tile).fuse(c, tile, tile).fuse(n, tile, tile)
                   .parallel(tile)
                   .vectorize(xi);
        }
#endif  // HAVE_HALIDE
    }

    bool getMemoryShapes(const std::vector<MatShape> &inputs,
                         const int requiredOutputs,
                         std::vector<MatShape> &outputs,
                         std::vector<MatShape> &internals) const
    {
        CV_Assert(inputs.size() != 0);
        Size in(inputs[0][3], inputs[0][2]), out;

        if (globalPooling)
        {
            out.height = 1;
            out.width = 1;
        }
        else if (padMode.empty())
        {
            float height = (float)(in.height + 2 * pad.height - kernel.height) / stride.height;
            float width = (float)(in.width + 2 * pad.width - kernel.width) / stride.width;
            out.height = 1 + (ceilMode ? ceil(height) : floor(height));
            out.width = 1 + (ceilMode ? ceil(width) : floor(width));

            if (pad.height || pad.width)
            {
                // If we have padding, ensure that the last pooling starts strictly
                // inside the image (instead of at the padding); otherwise clip the last.
                if ((out.height - 1) * stride.height >= in.height + pad.height)
                    --out.height;
                if ((out.width - 1) * stride.width >= in.width + pad.width)
                    --out.width;
                CV_Assert((out.height - 1) * stride.height < in.height + pad.height);
                CV_Assert((out.width - 1) * stride.width < in.width + pad.width);
            }
        }
        else
        {
            getConvPoolOutParams(in, kernel, stride, padMode, Size(1, 1), out);
        }

        outputs.resize(type == MAX ? 2 * inputs.size() : inputs.size());
        for (size_t i = 0; i < inputs.size(); i++)
        {
            size_t index = type == MAX ? 2*i : i;
            int dims[] = {inputs[i][0], inputs[i][1], out.height, out.width};
            outputs[index] = shape(dims);

            if (type == MAX)
                outputs[index + 1] = shape(dims);
        }

        return false;
    }

    virtual int64 getFLOPS(const std::vector<MatShape> &inputs,
                           const std::vector<MatShape> &outputs) const
    {
        (void)inputs; // suppress unused variable warning
        long flops = 0;

        for(int i = 0; i < outputs.size(); i++)
        {
            if (type == MAX)
            {
                if (i%2 == 0)
                    flops += total(outputs[i])*kernel.area();
            }
            else
            {
                flops += total(outputs[i])*(kernel.area() + 1);
            }
        }
        return flops;
    }
};

Ptr<PoolingLayer> PoolingLayer::create(const LayerParams& params)
{
    return Ptr<PoolingLayer>(new PoolingLayerImpl(params));
}

}
}
