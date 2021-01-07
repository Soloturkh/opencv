#include "opencv2/opencv_modules.hpp"

#ifndef HAVE_OPENCV_CUDEV

#error "opencv_cudev is required"

#else

#include "opencv2/core/cuda.hpp"
#include "opencv2/cudev.hpp"

using namespace cv;
using namespace cv::cuda;

GpuMatND::DevicePtr::DevicePtr(const size_t _size)
    : data(nullptr)
{
    CV_CUDEV_SAFE_CALL(cudaMalloc(&data, _size));
}

GpuMatND::DevicePtr::~DevicePtr()
{
    CV_CUDEV_SAFE_CALL(cudaFree(data));
}

/////////////////////////////////////////////////////
/// create

void GpuMatND::create(SizeArray _size, int _type)
{
    {
        auto elements_nonzero = [](SizeArray& v)
        {
            return std::all_of(v.begin(), v.end(),
                [](unsigned u){ return u > 0; });
        };
        CV_Assert(!_size.empty());
        CV_Assert(elements_nonzero(_size));
    }

    if (size == _size && type() == _type && !external())
        return;

    release();

    setFields(std::move(_size), _type);

    data_ = std::make_shared<DevicePtr>(totalMemSize());
    data = data_->data;
}

/////////////////////////////////////////////////////
/// release

void GpuMatND::release()
{
    data = nullptr;
    data_.reset();

    flags = dims = 0;
    size.clear();
    step.clear();
}

/////////////////////////////////////////////////////
/// clone

GpuMatND GpuMatND::clone() const
{
    GpuMatND ret(size, type());

    if (isContinuous())
    {
        CV_CUDEV_SAFE_CALL(cudaMemcpy(ret.data, data, totalMemSize(), cudaMemcpyDeviceToDevice));
    }
    else
    {
        // 1D arrays are always continuous

        if (dims == 2)
        {
            CV_CUDEV_SAFE_CALL(
                cudaMemcpy2D(ret.data, ret.step[0], data, step[0],
                    size[1]*step[1], size[0], cudaMemcpyDeviceToDevice)
            );
        }
        else
        {
            std::vector<int> idx(dims-2, 0);

            bool end = false;

            uchar* d = ret.data;
            uchar* s = data;

            // iterate each 2D plane
            do
            {
                CV_CUDEV_SAFE_CALL(
                    cudaMemcpy2D(
                        d, ret.step[dims-2], s, step[dims-2],
                        size[dims-1]*step[dims-1], size[dims-2], cudaMemcpyDeviceToDevice)
                );

                int inc = dims-3;
                while (true)
                {
                    if (idx[inc] == size[inc] - 1)
                    {
                        if (inc == 0)
                        {
                            end = true;
                            break;
                        }

                        idx[inc] = 0;
                        d -= (ret.size[inc] - 1) * ret.step[inc];
                        s -= (size[inc] - 1) * step[inc];
                        inc--;
                    }
                    else
                    {
                        idx[inc]++;
                        d += ret.step[inc];
                        s += step[inc];
                        break;
                    }
                }
            }
            while (!end);
        }
    }

    return ret;
}

GpuMatND GpuMatND::clone(Stream& stream) const
{
    GpuMatND ret(size, type());

    cudaStream_t _stream = StreamAccessor::getStream(stream);

    if (isContinuous())
    {
        CV_CUDEV_SAFE_CALL(cudaMemcpyAsync(ret.data, data, totalMemSize(), cudaMemcpyDeviceToDevice, _stream));
    }
    else
    {
        // 1D arrays are always continuous

        if (dims == 2)
        {
            CV_CUDEV_SAFE_CALL(
                cudaMemcpy2DAsync(ret.data, ret.step[0], data, step[0],
                    size[1]*step[1], size[0], cudaMemcpyDeviceToDevice, _stream)
            );
        }
        else
        {
            std::vector<int> idx(dims-2, 0);

            bool end = false;

            uchar* d = ret.data;
            uchar* s = data;

            // iterate each 2D plane
            do
            {
                CV_CUDEV_SAFE_CALL(
                    cudaMemcpy2DAsync(
                        d, ret.step[dims-2], s, step[dims-2],
                        size[dims-1]*step[dims-1], size[dims-2], cudaMemcpyDeviceToDevice, _stream)
                );

                int inc = dims-3;
                while (true)
                {
                    if (idx[inc] == size[inc] - 1)
                    {
                        if (inc == 0)
                        {
                            end = true;
                            break;
                        }

                        idx[inc] = 0;
                        d -= (ret.size[inc] - 1) * ret.step[inc];
                        s -= (size[inc] - 1) * step[inc];
                        inc--;
                    }
                    else
                    {
                        idx[inc]++;
                        d += ret.step[inc];
                        s += step[inc];
                        break;
                    }
                }
            }
            while (!end);
        }
    }

    return ret;
}

/////////////////////////////////////////////////////
/// upload

void GpuMatND::upload(InputArray src)
{
    Mat mat = src.getMat();

    CV_DbgAssert(!mat.empty());

    if (!mat.isContinuous())
        mat = mat.clone();

    SizeArray _size(mat.dims);
    std::copy_n(mat.size.p, mat.dims, _size.data());

    create(std::move(_size), mat.type());

    CV_CUDEV_SAFE_CALL(cudaMemcpy(data, mat.data, totalMemSize(), cudaMemcpyHostToDevice));
}

void GpuMatND::upload(InputArray src, Stream& stream)
{
    Mat mat = src.getMat();

    CV_DbgAssert(!mat.empty());

    if (!mat.isContinuous())
        mat = mat.clone();

    SizeArray _size(mat.dims);
    std::copy_n(mat.size.p, mat.dims, _size.data());

    create(std::move(_size), mat.type());

    cudaStream_t _stream = StreamAccessor::getStream(stream);
    CV_CUDEV_SAFE_CALL(cudaMemcpyAsync(data, mat.data, totalMemSize(), cudaMemcpyHostToDevice, _stream));
}

/////////////////////////////////////////////////////
/// download

void GpuMatND::download(OutputArray dst) const
{
    CV_DbgAssert(!empty());

    dst.create(dims, size.data(), type());
    Mat mat = dst.getMat();

    GpuMatND gmat = *this;

    if (!gmat.isContinuous())
        gmat = gmat.clone();

    CV_CUDEV_SAFE_CALL(cudaMemcpy(mat.data, gmat.data, gmat.totalMemSize(), cudaMemcpyDeviceToHost));
}

void GpuMatND::download(OutputArray dst, Stream& stream) const
{
    CV_DbgAssert(!empty());

    dst.create(dims, size.data(), type());
    Mat mat = dst.getMat();

    GpuMatND gmat = *this;

    if (!gmat.isContinuous())
        gmat = gmat.clone(stream);

    cudaStream_t _stream = StreamAccessor::getStream(stream);
    CV_CUDEV_SAFE_CALL(cudaMemcpyAsync(mat.data, gmat.data, gmat.totalMemSize(), cudaMemcpyDeviceToHost, _stream));
}

#endif
