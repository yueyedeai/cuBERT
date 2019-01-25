//
// Created by 田露 on 2019/1/18.
//

#include "gtest/gtest.h"
#include <vector>
#include <cuda_runtime.h>
#include <cudnn.h>

#include "cuBERT/op/Softmax.h"
using namespace cuBERT;

class SoftmaxTest : public ::testing::Test {
protected:
    void SetUp() override {
        cudnnCreate(&handle);
    }

    void TearDown() override {
        cudnnDestroy(handle);
    }

    cudnnHandle_t handle;
};

TEST_F(SoftmaxTest, compute_) {
    float inout[6] = {1, 2, 3, 6, 6, 6};
    float *inout_gpu;

    cudaMalloc(&inout_gpu, sizeof(float) * 6);
    cudaMemcpy(inout_gpu, inout, sizeof(float) * 6, cudaMemcpyHostToDevice);

    Softmax softmax(handle, 3);
    softmax.compute_(2, inout_gpu);

    cudaMemcpy(inout, inout_gpu, sizeof(float) * 6, cudaMemcpyDeviceToHost);
    cudaFree(inout_gpu);

    EXPECT_FLOAT_EQ(inout[0], 0.090030573);
    EXPECT_FLOAT_EQ(inout[1], 0.244728478);
    EXPECT_FLOAT_EQ(inout[2], 0.66524094);
    EXPECT_FLOAT_EQ(inout[3], 0.33333334);
    EXPECT_FLOAT_EQ(inout[4], 0.33333334);
    EXPECT_FLOAT_EQ(inout[5], 0.33333334);
}