// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#ifndef OPENCV_DNN_SRC_CUDA4DNN_PRIMITIVES_SPACE_DEPTH_HPP
#define OPENCV_DNN_SRC_CUDA4DNN_PRIMITIVES_SPACE_DEPTH_HPP

#include "../../op_cuda.hpp"

#include "../csl/stream.hpp"
#include "../csl/tensor.hpp"
#include "../csl/tensor_ops.hpp"
#include "../kernels/permute.hpp"

#include <utility>

namespace cv { namespace dnn { namespace cuda4dnn {

    template <class T>
    class DepthSpaceOp final : public CUDABackendNode {
    public:
        using wrapper_type = GetCUDABackendWrapperType<T>;

        DepthSpaceOp(csl::Stream stream_, const std::vector<int> &internal_shape_,
                     const std::vector<size_t> &permutation_)
            : stream(std::move(stream_)), internal_shape(internal_shape_),
              permutation(permutation_)
        {
            transposed_internal_shape = std::vector<int>(internal_shape.size());
            for (size_t i = 0; i < permutation.size(); i++) {
                transposed_internal_shape[i] = internal_shape[permutation[i]];
            }

            size_t num_elements = std::accumulate(internal_shape.begin(), internal_shape.end(), 1, std::multiplies<size_t>());
            csl::WorkspaceBuilder builder;
            builder.require<T>(num_elements);
        }

        void forward(const std::vector<cv::Ptr<BackendWrapper>> &inputs,
                     const std::vector<cv::Ptr<BackendWrapper>> &outputs,
                     csl::Workspace &workspace) override {
            CV_CheckEQ(inputs.size(), size_t(1), "DepthSpaceOp: only one input is accepted");
            CV_CheckEQ(outputs.size(), size_t(1), "DepthSpaceOp: only one output is accepted");

            auto input_wrapper = inputs.front().dynamicCast<wrapper_type>();
            auto input = input_wrapper->getView();
            auto output_wrapper = outputs.front().dynamicCast<wrapper_type>();
            auto output = output_wrapper->getSpan();
            auto ws_allocator = csl::WorkspaceAllocator(workspace);
            auto transposed_internal = ws_allocator.get_tensor_span<T>(transposed_internal_shape.begin(), transposed_internal_shape.end());

            input.reshape(internal_shape.begin(), internal_shape.end());
            kernels::permute(stream, transposed_internal, input, permutation);
            transposed_internal.reshape_as(output);
            csl::tensor_ops::copy(stream, output, csl::TensorView<T>(transposed_internal));
        }
    private:
        csl::Stream stream;
        std::vector<int> internal_shape;
        std::vector<size_t> permutation;
        std::vector<int> transposed_internal_shape;
    };

}}} // namespace cv::dnn::cuda4dnn

#endif // OPENCV_DNN_SRC_CUDA4DNN_PRIMITIVES_SPACE_DEPTH_HPP
