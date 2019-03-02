#include "gtest/gtest.h"
#include <cmath>

#include "cuBERT/common.h"
#include "cuBERT/op_bert/BertPooler.h"
using namespace cuBERT;

class BertPoolerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cuBERT::initialize();
        handle = cuBERT::blas_create();
    }

    void TearDown() override {
        cuBERT::blas_destroy(handle);
        cuBERT::finalize();
    }

    void* handle;
};

TEST_F(BertPoolerTest, compute) {
    size_t seq_length = 3;
    size_t hidden_size = 2;

    float kernel[] = {-1, 0,
                      0, 1};
    float bias[] = {2, 3};

    BertPooler pooler(handle, seq_length, hidden_size, kernel, bias, 32);

    float in[12] = {
            0, 1,
            1, 1,
            2, 1,
            -2, -1,
            3, 2,
            0, 5,
    };
    float out[4];

    float* in_gpu = (float*) cuBERT::malloc(sizeof(float) * 12);
    float* out_gpu = (float*) cuBERT::malloc(sizeof(float) * 4);

    cuBERT::memcpy(in_gpu, in, sizeof(float) * 12, 1);

    pooler.compute(2, in_gpu, out_gpu);

    cuBERT::memcpy(out, out_gpu, sizeof(float) * 4, 2);
    cuBERT::free(in_gpu);
    cuBERT::free(out_gpu);

    EXPECT_FLOAT_EQ(out[0], tanhf(2));
    EXPECT_FLOAT_EQ(out[1], tanhf(4));
    EXPECT_FLOAT_EQ(out[2], tanhf(4));
    EXPECT_FLOAT_EQ(out[3], tanhf(2));
}


class BertPoolerCPUTest : public ::testing::Test {
protected:
    void SetUp() override {
        cuBERT::initialize(true);
    }

    void TearDown() override {
        cuBERT::finalize();
    }
};

TEST_F(BertPoolerCPUTest, compute_cpu) {
    size_t seq_length = 3;
    size_t hidden_size = 2;

    float kernel[] = {-1, 0,
                      0, 1};
    float bias[] = {2, 3};

    BertPooler pooler(nullptr, seq_length, hidden_size, kernel, bias, 32);

    float in[12] = {
            0, 1,
            1, 1,
            2, 1,
            -2, -1,
            3, 2,
            0, 5,
    };
    float out[4];

    pooler.compute(2, in, out);

    EXPECT_FLOAT_EQ(out[0], tanhf(2));
    EXPECT_FLOAT_EQ(out[1], tanhf(4));
    EXPECT_FLOAT_EQ(out[2], tanhf(4));
    EXPECT_FLOAT_EQ(out[3], tanhf(2));
}