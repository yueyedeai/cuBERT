//
// Created by 田露 on 2019/1/17.
//

#include "Transpose.h"

namespace cuBERT {
    const static float ZERO = 0;
    const static float ONE = 1;

    Transpose::Transpose(cudnnHandle_t handle,
                         const std::vector<int> &dims_in,
                         const std::vector<int> &axes)
            : dims_in(dims_in), dims_out(dims_in.size()),
              stride_in(dims_in.size()), stride_out(dims_in.size()) {
        this->handle = handle;

        const int ndim = dims_in.size();
//    assert(ndim == axes.size());
//    assert(axes[0] = 0);

        for (int i = 0; i < ndim; ++i) {
            dims_out[i] = dims_in[axes[i]];
        }

        stride_out[ndim - 1] = 1;
        for (int i = ndim - 2; i >= 0; i--) {
            stride_out[i] = stride_out[i + 1] * dims_out[i + 1];
        }

        for (int i = 0; i < ndim; ++i) {
            stride_in[i] = 1;
            for (int j = axes[i] + 1; j < ndim; ++j) {
                stride_in[i] *= dims_in[j];
            }
        }

        cudnnCreateTensorDescriptor(&desc_in);
        cudnnCreateTensorDescriptor(&desc_out);
    }

    Transpose::~Transpose() {
        cudnnDestroyTensorDescriptor(desc_out);
        cudnnDestroyTensorDescriptor(desc_in);
    }

    void Transpose::compute(size_t batch_size, float *in_gpu, float *out_gpu) {
        const int ndim = dims_in.size();

        std::vector<int> dims_out(this->dims_out);
        dims_out[0] = batch_size;

        cudnnSetTensorNdDescriptor(desc_in, CUDNN_DATA_FLOAT, ndim, dims_out.data(), stride_in.data());
        cudnnSetTensorNdDescriptor(desc_out, CUDNN_DATA_FLOAT, ndim, dims_out.data(), stride_out.data());

        cudnnTransformTensor(handle, &ONE, desc_in, in_gpu, &ZERO, desc_out, out_gpu);
    }

    void Transpose::compute_cpu(size_t batch_size, float *in, float *out) {
        cudaStream_t stream = nullptr;
        cudnnGetStream(handle, &stream);

        size_t size = batch_size;
        for (int i = 1; i < dims_in.size(); ++i) {
            size *= dims_in[i];
        }

        float *in_gpu;
        float *out_gpu;
        cudaMalloc(&in_gpu, size * sizeof(float));
        cudaMalloc(&out_gpu, size * sizeof(float));

        cudaMemcpyAsync(in_gpu, in, size * sizeof(float), cudaMemcpyHostToDevice, stream);

        compute(batch_size, in_gpu, out_gpu);

        // sync
        cudaMemcpy(out, out_gpu, size * sizeof(float), cudaMemcpyDeviceToHost);
        cudaFree(out_gpu);
        cudaFree(in_gpu);
    }
}