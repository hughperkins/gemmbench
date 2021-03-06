/*
 * Copyright (c) 2016 ARM Limited.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "mali_opencl.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#ifndef WINDOWS
#include <sys/time.h>
#endif

#if (1 == XOPENME)
 #include <cassert>
 #include "xopenme.h"

/* OpenME class to communicate with CK */
class xopenme
{
private:
    static const int max_str_len = 1000;
    static const int max_var_count = 50;
    static const int max_tmr_count = 4;
    static const int max_work_dims = 3;

public:
    char str[max_str_len];
    int  var_count;

    cl_uint  tmp_uint;
    cl_ulong tmp_ulong;
    size_t   tmp_size_t;
    size_t   tmp_size_t_dims[max_work_dims];

    xopenme() :
        var_count(0),
        tmp_uint(0),
        tmp_ulong(0),
        tmp_size_t(0),
        tmp_size_t_dims() // C++11: {0, ..., 0}
    {
        assert(3 == max_work_dims);
        xopenme_init(max_tmr_count, max_var_count);
    }

    ~xopenme()
    {
        xopenme_dump_state();
    }

    bool var_count_below_max()
    {
        return var_count < max_var_count;
    }

}; // END OF xopenme class

#endif

/* Platform and device ID */
int32_t platform_idx = 0;
int32_t device_idx = 0;

cl_platform_id   platform;
cl_device_id     device;

#if (1 == XOPENME)
    xopenme openme;
#endif

    // Get OpenCL platform specified with the "-p" command line argument.
    void get_platform()
    {
        cl_int err = CL_SUCCESS;

        cl_uint num_entries = platform_idx+1;
        cl_platform_id * platforms = new cl_platform_id[num_entries];

        cl_uint num_platforms;
        err = clGetPlatformIDs(num_entries, platforms, &num_platforms);
        assert(CL_SUCCESS == err && "clGetPlatformIDs() failed.");
        assert(platform_idx < num_platforms && "Platform not available.");

        platform = platforms[platform_idx];
        delete [] platforms;

#if (1 == XOPENME)
        err = clGetPlatformInfo(platform, CL_PLATFORM_NAME,   sizeof(openme.str), &openme.str, NULL);
        assert(CL_SUCCESS == err && "clGetPlatformInfo() failed.");
        xopenme_add_var_s(openme.var_count++, (char*) "  \"CL_PLATFORM_NAME\":\"%s\"",    openme.str);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(openme.str), &openme.str, NULL);
        assert(CL_SUCCESS == err && "clGetPlatformInfo() failed.");
        xopenme_add_var_s(openme.var_count++, (char*) "  \"CL_PLATFORM_VENDOR\":\"%s\"",  openme.str);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(openme.str), &openme.str, NULL);
        assert(CL_SUCCESS == err && "clGetPlatformInfo() failed.");
        xopenme_add_var_s(openme.var_count++, (char*) "  \"CL_PLATFORM_VERSION\":\"%s\"", openme.str);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");
#endif
    } // END OF get_platform()


    // Get OpenCL device specified with the "-d" command line argument.
    void get_device()
    {
        cl_int err = CL_SUCCESS;

        cl_uint num_entries = device_idx+1;
        cl_device_id * devices = new cl_device_id[num_entries];

        cl_uint num_devices;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_entries, devices, &num_devices);
        assert(CL_SUCCESS == err && "clGetDeviceIDs() failed.");
        assert(device_idx < num_devices && "No device.");

        device = devices[device_idx];
        delete [] devices;

#if (1 == XOPENME)
        err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(openme.str),   &openme.str, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_s(openme.var_count++, (char*) "  \"CL_DEVICE_NAME\":\"%s\"",   openme.str);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(openme.str), &openme.str, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_s(openme.var_count++, (char*) "  \"CL_DEVICE_VENDOR\":\"%s\"", openme.str);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(openme.str), &openme.str, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_s(openme.var_count++, (char*) "  \"CL_DEVICE_VERSION\":\"%s\"", openme.str);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(openme.str), &openme.str, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_s(openme.var_count++, (char*) "  \"CL_DRIVER_VERSION\":\"%s\"", openme.str);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &openme.tmp_uint, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_MAX_COMPUTE_UNITS\":%u", openme.tmp_uint);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &openme.tmp_uint, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_MAX_CLOCK_FREQUENCY\":%u", openme.tmp_uint);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &openme.tmp_ulong, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_GLOBAL_MEM_SIZE\":%u", openme.tmp_ulong);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &openme.tmp_ulong, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_LOCAL_MEM_SIZE\":%u", openme.tmp_ulong);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &openme.tmp_size_t, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_MAX_WORK_GROUP_SIZE\":%u", openme.tmp_size_t);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &openme.tmp_size_t, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS\":%u", openme.tmp_size_t);
        assert(openme.tmp_size_t == 3 && "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS != 3");
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(openme.tmp_size_t_dims), openme.tmp_size_t_dims, NULL);
        assert(CL_SUCCESS == err && "clGetDeviceInfo() failed.");
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_MAX_WORK_ITEM_SIZES_0\":%u", openme.tmp_size_t_dims[0]);
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_MAX_WORK_ITEM_SIZES_1\":%u", openme.tmp_size_t_dims[1]);
        xopenme_add_var_i(openme.var_count++, (char*) "  \"CL_DEVICE_MAX_WORK_ITEM_SIZES_2\":%u", openme.tmp_size_t_dims[2]);
#endif

    } // END OF get_device()

/* Floating point types */
typedef cl_half     fp16;
typedef cl_float    fp32;

#define READ_FLOAT_VAR( attr, env, default_value )\
    do{ \
        char * str = getenv( env ); \
        attr = ( str != NULL ) ? atof( str ) : default_value;\
        printf( "%s = %f\n", env, (float) attr );\
    }while(0)

#define READ_MATRIX_SHAPE( attr, env, default_rows, default_cols )\
    do{ \
        char * str = getenv( env ); \
        if (str) \
        { \
            attr.rows = strtol(str, &str, 10); \
            if (*str == ',') \
            { \
                str++; \
                attr.cols = strtol(str, &str, 10); \
            } \
            else \
            { \
                attr.cols = default_cols; \
            } \
        } \
        else \
        { \
            attr.rows = default_rows; \
            attr.cols = default_cols; \
        } \
        printf( "%s = %u x %u\n", env, (size_t)attr.rows, (size_t)attr.cols );\
    }while(0)

typedef enum
{
    FP32 = 0,                                               // fp32 implementation
    FP16,                                                   // fp16 implementation
    HYBRID                                                  // Hybrid implementation. Multiplication in fp16 and accumulation in fp32
} gemm_type;

struct cl_kernel_config
{
    const char *kernel_name;
    size_t      divisor_gws_x;
    size_t      divisor_gws_y;
};

struct cl_pipeline_config
{
    cl_kernel_config interleave;                            // CL kernel interleave configuration
    cl_kernel_config transpose;                             // CL kernel transpose configuration
    cl_kernel_config mm;                                    // CL kernel matrix multiplication configuration
    cl_kernel_config finalize;                              // CL kernel finalize configuration
    size_t           default_mm_lws[2];                     // Default LWS for the matrix multiplication kernel.
};

struct matrix_shape
{
    size_t rows;                                            // Rows of Matrix
    size_t cols;                                            // Columns of Matrix
    size_t internal_rows;                                   // Internal value used by OpenCL for the rows of Matrix
    size_t internal_cols;                                   // Internal value used by OpenCL for the cols of Matrix
};

cl_pipeline_config cl_gemm[] =
{
    {
        /* FP32 implementation - Pure fp32 computation */
        { "interleave_fp32", 4, 4 },
        { "transpose_fp32", 4, 1},
        { "mm_fp32", 4, 4 },
        { "finalize_fp32", 4, 1 },
        { 4, 8 }
    },
    {
        /* FP16 implementation - Pure fp16 computation */
        { "interleave_fp16", 4, 4 },
        { "transpose_fp16", 8, 1},
        { "mm_fp16", 8, 4 },
        { "finalize_fp16", 8, 1 },
        { 4, 8 }
    },
    {
        /* Hybrid implementation - Multiplication in fp16 and accumulation in fp32 */
        { "interleave_fp16", 4, 4 },
        { "transpose_hybrid", 4, 1},
        { "mm_hybrid", 4, 4 },
        { "finalize_fp16", 8, 1 },
        { 4, 8 }
    },
};

static cl_ulong
get_profiling_info(const std::string& prefix, cl::Event& event)
{
    if (event == cl::Event()) return 0;

    cl_ulong queued = event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
    cl_ulong submit = event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>();
    cl_ulong start  = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
    cl_ulong end    = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

    std::cout << prefix << " QUEUED " << queued << std::endl;
    std::cout << prefix << " SUBMIT " << submit << std::endl;
    std::cout << prefix << " START  " << start  << std::endl;
    std::cout << prefix << " END    " << end    << std::endl;

    return (end - start);
}

template<typename _type>
_type max (const _type& a, const _type& b)
{
    return (b > a)? b : a;
}

#define PADDING_BYTES(a, b) (((b) - ((a) % (b))) % (b))

template <typename _type>
void sgemm_init_type(matrix_shape &a, matrix_shape &b, _type *mtx_a, _type *mtx_b, _type *mtx_c)
{
    const unsigned seed = 11;
    srand(seed);

    /* Initiatilize Matrix A */
    for (int i = 0; i < a.rows; i++)
    {
        for (int j = 0; j < a.cols; j++)
        {
            const int ij = i * a.internal_cols + j;
            mtx_a[ij] = (_type)(((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f);
        }
    }

    /* Initiatilize Matrix B */
    for (int i = 0; i < b.rows; i++)
    {
        for (int j = 0; j < b.cols; j++)
        {
            const int ij = i * b.internal_cols + j;
            mtx_b[ij] = (_type)(((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f);
        }
    }

    /* Initiatilize Matrix C */
    for (int i = 0; i < a.rows; i++)
    {
        for (int j = 0; j < b.cols; j++)
        {
            const int ij = i * b.internal_cols + j;
            mtx_c[ij] = (_type)(((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f);
        }
    }
}

void sgemm_init(gemm_type type, matrix_shape &a, matrix_shape &b, void *mtx_a, void *mtx_b, void *mtx_c)
{
    switch(type)
    {
        case FP32:
            sgemm_init_type<fp32>( a, b, (fp32 *)mtx_a, (fp32 *)mtx_b, (fp32 *)mtx_c );
            return;
        case FP16:
        case HYBRID:
            sgemm_init_type<fp16>( a, b, (fp16 *)mtx_a, (fp16 *)mtx_b, (fp16 *)mtx_c );
            return;
    }
}

template <typename _type>
void run_reference_gemm_type( matrix_shape &a, matrix_shape &b, _type *mtx_a, _type *mtx_b,
                              _type *mtx_c, fp32 alpha, fp32 beta, _type *expected)
{
    for(int32_t r = 0; r < a.rows; ++r)
    {
        for(int32_t c = 0; c < b.cols; ++c)
        {
            _type acc = (_type)0.0;
            for (int i = 0; i < a.cols; i++)
            {
                _type A = (_type)mtx_a[i + r * a.internal_cols];
                _type B = (_type)mtx_b[c + i * b.internal_cols];
                acc += A * B;
            }

            _type C = mtx_c[c + r * b.internal_cols];
            expected[c + r * b.cols] = (_type)(alpha * acc + beta * C);
        }
    }
}

void run_reference_gemm(gemm_type type, matrix_shape &a, matrix_shape &b, void *mtx_a, void *mtx_b, void *mtx_c, fp32 alpha,
                        fp32 beta, void *expected)
{
    switch(type)
    {
        case FP32:
            run_reference_gemm_type<fp32>( a, b, (fp32 *)mtx_a, (fp32 *)mtx_b, (fp32 *)mtx_c, alpha, beta,
                                           (fp32 *)expected );
            return;
        case FP16:
        case HYBRID:
            run_reference_gemm_type<fp16>( a, b, (fp16 *)mtx_a, (fp16 *)mtx_b, (fp16 *)mtx_c, alpha, beta,
                                           (fp16 *)expected );
            return;
    }
}

template<typename _type>
void verify_result_type( int32_t row, int32_t col, _type *expected, _type *actual )
{
    bool passed = true;

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            _type ref = expected[i * col + j];
            _type given = actual[i * col + j];
            _type eps = (_type)1.0e-1;

            if (fabs((float)(ref - given)) > eps && fabs((float)(ref - given) / ref) > eps)
            {
                std::cout << "C[" << i << ", " << j << "] = " << actual[i * col + j] << " != " << expected[i * col + j]
                          << std::endl;
                passed = false;
            }
        }
    }

    std::cout << "Reference comparison " << (passed ? "passed!" : "failed!") << std::endl;
}

void verify_result(gemm_type type, int32_t row, int32_t col, void *expected, void *actual)
{
    switch(type)
    {
        case FP32:
            verify_result_type<fp32>( row, col, (fp32 *)expected, (fp32 *)actual );
            return;
        case FP16:
        case HYBRID:
            verify_result_type<fp16>( row, col, (fp16 *)expected, (fp16 *)actual );
            return;
    }
}

void sgemm(int argc, const char **argv)
{
    int32_t gemm_type_idx = 0;

    matrix_shape mtx_a;
    matrix_shape mtx_b;

    fp32 alpha = 0.0f;
    fp32 beta  = 0.0f;

    size_t mm_lws_x = 0;
    size_t mm_lws_y = 0;

    int32_t skip_padding = 0;    /* if 1, skip padding */
    int32_t skip_validation = 0; /* if 1, skip validation */

    /* Read environment: (program variable, environment variable, default value). */
    READ_CONFIG( gemm_type_idx,   "GEMM_TYPE",          static_cast<int32_t>(FP32) );

    READ_MATRIX_SHAPE( mtx_a,     "MTX_A",              1000,       1000 );
    READ_MATRIX_SHAPE( mtx_b,     "MTX_B",              mtx_a.cols, 1000 );

    READ_FLOAT_VAR( alpha,        "ALPHA",              1.0f );
    READ_FLOAT_VAR( beta ,        "BETA",               0.0f );

    READ_CONFIG( mm_lws_x,        "CK_LWS_X",           0 );
    READ_CONFIG( mm_lws_y,        "CK_LWS_Y",           0 );

    READ_CONFIG( skip_padding,    "CK_SKIP_PADDING",    0 );
    READ_CONFIG( skip_validation, "CK_SKIP_VALIDATION", 0 );

    READ_CONFIG( platform_idx,    "PLATFORM_ID",        0 );
    READ_CONFIG( device_idx,      "DEVICE_ID",          0 );

    /* Check whether matrix A can be multiplied by matrix B */
    if( mtx_a.cols != mtx_b.rows )
    {
        throw std::runtime_error( "The number of columns of matrix A must be the same as the number of rows of matrix B!" );
    }

    /* Check whether GEMM_TYPE is valid. */
    if( (gemm_type_idx < 0) || (gemm_type_idx > 2) )
    {
        throw std::runtime_error( "GEMM_TYPE must be 0, 1 or 2!" );
    }

    /* Compute matrix dimensions for the OpenCL implementation */
    size_t mm_gws_x = cl_gemm[gemm_type_idx].mm.divisor_gws_x;
    size_t mm_gws_y = cl_gemm[gemm_type_idx].mm.divisor_gws_y;
    if (mm_lws_x==0) mm_lws_x = cl_gemm[gemm_type_idx].default_mm_lws[0];
    if (mm_lws_y==0) mm_lws_y = cl_gemm[gemm_type_idx].default_mm_lws[1];

    std::cout << "LWS = [" << mm_lws_x << ", " << mm_lws_y << "]" << std::endl;

    /* Since we have a fixed LWS, we need to ensure that the dimensions of GWS[x,y] are multiples of the dimensions of LWS[x, y] */
    /* This kind of operation is done just for the rows of matrix A and columns of Matrix B */
    mtx_a.internal_rows = ( mtx_a.rows / mm_gws_y );
    if (skip_padding==0) mtx_a.internal_rows += PADDING_BYTES( ( mtx_a.rows / mm_gws_y ), mm_lws_y );
    mtx_a.internal_rows *= mm_gws_y;

    mtx_b.internal_cols = ( mtx_b.cols / mm_gws_x );
    if (skip_padding==0) mtx_b.internal_cols += PADDING_BYTES( ( mtx_b.cols / mm_gws_x ), mm_lws_x );
    mtx_b.internal_cols *= mm_gws_x;

    /* Add extra padding bytes if columns of Matrix A and/or rows of Matrix B are not multiples of GWS */
    mtx_a.internal_cols = mtx_a.cols + PADDING_BYTES( mtx_a.cols, mm_gws_x );
    mtx_b.internal_rows = mtx_b.rows + PADDING_BYTES( mtx_b.rows, mm_gws_y );

    /* Ensure internal_row_a and internal_col_b are not 0 */
    mtx_a.internal_rows = max( (size_t)mtx_a.internal_rows, mm_lws_y );
    mtx_b.internal_cols = max( (size_t)mtx_b.internal_cols, mm_lws_x);

    std::cout << "Internal Matrix A: [" << mtx_a.internal_rows << ", " << mtx_a.internal_cols << "]" << std::endl;
    std::cout << "Internal Matrix B: [" << mtx_b.internal_rows << ", " << mtx_b.internal_cols << "]" << std::endl;

    size_t mtx_a_size = ( mtx_a.internal_rows * mtx_a.internal_cols );
    size_t mtx_b_size = ( mtx_b.internal_rows * mtx_b.internal_cols );
    size_t mtx_c_size = ( mtx_a.internal_rows * mtx_b.internal_cols );

    size_t buffer_mtx_a_size = (mtx_a_size) * (static_cast<gemm_type>(gemm_type_idx) == FP32 ? sizeof(
                                                            fp32) : sizeof(fp16));
    size_t buffer_mtx_b_size = (mtx_b_size) * (static_cast<gemm_type>(gemm_type_idx) == FP32 ? sizeof(
                                                            fp32) : sizeof(fp16));
    size_t buffer_mtx_c_size = (mtx_c_size) * (static_cast<gemm_type>(gemm_type_idx) == FP32 ? sizeof(
                                                            fp32) : sizeof(fp16));

    /* Compute strides for matrix A (after interleaving) and matrix B (after transposing) */
    /* Those strides are respectively the number of elements per row in matrix A after interleaving and in matrix B after transposing */
    size_t mtx_a_inter_stride = mtx_a.internal_cols * cl_gemm[gemm_type_idx].interleave.divisor_gws_y;
    size_t mtx_b_trans_stride = mtx_b.internal_rows * cl_gemm[gemm_type_idx].transpose.divisor_gws_x;

    get_platform();
    get_device();

    cl::Program program = cl::Program( mali::read_file("../kernels/sgemm.cl") );

    /* Pre-processor CL define for the buffer origin */
    std::stringstream options;
    options << "-DROW_MTX_A=" << mtx_a.internal_rows << " ";
    options << "-DCOL_MTX_A=" << mtx_a.internal_cols << " ";
    options << "-DROW_MTX_B=" << mtx_b.internal_rows << " ";
    options << "-DCOL_MTX_B=" << mtx_b.internal_cols << " ";
    options << "-DROW_MTX_C=" << mtx_a.internal_rows << " ";
    options << "-DCOL_MTX_C=" << mtx_b.internal_cols << " ";
    options << "-DMATRIX_A_INTERLEAVED_STRIDE=" << mtx_a_inter_stride << " ";           // Number of elements per row in matrix A after interleaving
    options << "-DMATRIX_B_TRANSPOSED_STRIDE="  << mtx_b_trans_stride;                  // Number of elements per row in matrix B after transposing

    /* Build OpenCL program. */
    mali::build_cl_program( program, options.str().c_str() );

    /* Allocate OpenCL buffers for A, B, C, temp0, temp1 and output matrix. */
    cl::Buffer buffer_mtx_a    ( CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, buffer_mtx_a_size, NULL );
    cl::Buffer buffer_mtx_b    ( CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, buffer_mtx_b_size, NULL );
    cl::Buffer buffer_mtx_c    ( CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, buffer_mtx_c_size, NULL );
    cl::Buffer buffer_mtx_temp0( CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, buffer_mtx_a_size, NULL );
    cl::Buffer buffer_mtx_temp1( CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, buffer_mtx_b_size, NULL );
    cl::Buffer buffer_mtx_out  ( CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, buffer_mtx_c_size, NULL );

    /* Create OpenCL kernels. */
    cl::Kernel kernel_interleave( program, cl_gemm[gemm_type_idx].interleave.kernel_name );
    cl::Kernel kernel_transpose ( program, cl_gemm[gemm_type_idx].transpose.kernel_name );
    cl::Kernel kernel_mm        ( program, cl_gemm[gemm_type_idx].mm.kernel_name );
    cl::Kernel kernel_finalize  ( program, cl_gemm[gemm_type_idx].finalize.kernel_name );

    /* Create command queue. */
    cl::CommandQueue queue( cl::QueueProperties::Profiling, NULL );

    /* Init input buffers. */
    void *ptr_mtx_a     = NULL;
    void *ptr_mtx_b     = NULL;
    void *ptr_mtx_c     = NULL;
    void *ptr_mtx_temp0 = NULL;
    void *ptr_mtx_temp1 = NULL;
    void *ptr_mtx_out   = NULL;

    /* Map buffers. */
    ptr_mtx_a     = queue.enqueueMapBuffer( buffer_mtx_a,     CL_TRUE, CL_MAP_WRITE, 0, buffer_mtx_a_size );
    ptr_mtx_b     = queue.enqueueMapBuffer( buffer_mtx_b,     CL_TRUE, CL_MAP_WRITE, 0, buffer_mtx_b_size );
    ptr_mtx_c     = queue.enqueueMapBuffer( buffer_mtx_c,     CL_TRUE, CL_MAP_WRITE, 0, buffer_mtx_c_size );
    ptr_mtx_temp0 = queue.enqueueMapBuffer( buffer_mtx_temp0, CL_TRUE, CL_MAP_WRITE, 0, buffer_mtx_a_size );
    ptr_mtx_temp1 = queue.enqueueMapBuffer( buffer_mtx_temp1, CL_TRUE, CL_MAP_WRITE, 0, buffer_mtx_b_size );
    ptr_mtx_out   = queue.enqueueMapBuffer( buffer_mtx_out,   CL_TRUE, CL_MAP_WRITE, 0, buffer_mtx_c_size );

    /* Init to zero. */
    memset(ptr_mtx_a,     0, buffer_mtx_a_size);
    memset(ptr_mtx_b,     0, buffer_mtx_b_size);
    memset(ptr_mtx_c,     0, buffer_mtx_c_size);
    memset(ptr_mtx_temp0, 0, buffer_mtx_a_size);
    memset(ptr_mtx_temp1, 0, buffer_mtx_b_size);
    memset(ptr_mtx_out,   0, buffer_mtx_c_size);

    /* Init matrix A, B and C. */
    sgemm_init( static_cast<gemm_type>(gemm_type_idx), mtx_a, mtx_b, ptr_mtx_a, ptr_mtx_b, ptr_mtx_c );

    /* Run reference implementation. */
    size_t buffer_expected_size = 0;
    void *expected = NULL;
    if (skip_validation==0)
    {
        buffer_expected_size = ( mtx_a.rows * mtx_b.cols ) * (static_cast<gemm_type>(gemm_type_idx) == FP32 ? sizeof(
                                                                fp32) : sizeof(fp16));
        expected = malloc( buffer_expected_size );
        run_reference_gemm( static_cast<gemm_type>(gemm_type_idx), mtx_a, mtx_b, ptr_mtx_a, ptr_mtx_b, ptr_mtx_c, alpha, beta,
                            expected );
    }

    /* Unmap buffers. */
    queue.enqueueUnmapMemObject( buffer_mtx_a, ptr_mtx_a );
    queue.enqueueUnmapMemObject( buffer_mtx_b, ptr_mtx_b );
    queue.enqueueUnmapMemObject( buffer_mtx_c, ptr_mtx_c );
    queue.enqueueUnmapMemObject( buffer_mtx_temp0, ptr_mtx_temp0 );
    queue.enqueueUnmapMemObject( buffer_mtx_temp1, ptr_mtx_temp1 );
    queue.enqueueUnmapMemObject( buffer_mtx_out, ptr_mtx_out );

    /* Interleave kernel arguments. */
    kernel_interleave.setArg( 0, buffer_mtx_a );
    kernel_interleave.setArg( 1, buffer_mtx_temp0 );

    /* Transpose kernel arguments. */
    kernel_transpose.setArg( 0, buffer_mtx_b );
    kernel_transpose.setArg( 1, buffer_mtx_temp1 );

    /* Matrix Multiplication kernel arguments - A x B. */
    kernel_mm.setArg( 0, buffer_mtx_temp0 );
    kernel_mm.setArg( 1, buffer_mtx_temp1 );
    kernel_mm.setArg( 2, buffer_mtx_out );

    /* Finalize kernel arguments. */
    kernel_finalize.setArg( 0, buffer_mtx_c );
    kernel_finalize.setArg( 1, buffer_mtx_out );
    kernel_finalize.setArg( 2, alpha );
    kernel_finalize.setArg( 3, beta );

    /* Kernel events for the interleave, transpose, multiply and finalize kernels. */
    cl::Event interleave_event;
    cl::Event transpose_event;
    cl::Event multiply_event;
    cl::Event finalize_event;

    /* Vector of events associated with the previously enqueued kernels. Don't bother with for an in-order queue. */
    std::vector<cl::Event>* interleave_events = NULL;
    std::vector<cl::Event>* transpose_events  = NULL;
    std::vector<cl::Event>* multiply_events   = NULL;
    std::vector<cl::Event>* finalize_events   = NULL;

    /* Global Work Size (GWS) for the kernels. */
    cl::NDRange interleave_gws( mtx_a.internal_cols / cl_gemm[gemm_type_idx].interleave.divisor_gws_x,
                                mtx_a.internal_rows / cl_gemm[gemm_type_idx].interleave.divisor_gws_y );
    cl::NDRange transpose_gws ( mtx_b.internal_cols / cl_gemm[gemm_type_idx].transpose.divisor_gws_x,
                                mtx_b.internal_rows / cl_gemm[gemm_type_idx].transpose.divisor_gws_y );
    cl::NDRange mm_gws        ( mtx_b.internal_cols / cl_gemm[gemm_type_idx].mm.divisor_gws_x,
                                mtx_a.internal_rows / cl_gemm[gemm_type_idx].mm.divisor_gws_y );
    cl::NDRange finalize_gws  ( mtx_b.internal_cols / cl_gemm[gemm_type_idx].finalize.divisor_gws_x,
                                mtx_a.internal_rows / cl_gemm[gemm_type_idx].finalize.divisor_gws_y );

    /* Local Work Size (LWS) for the matrix multiplication kernel. */
    cl::NDRange mm_lws = { cl_gemm[gemm_type_idx].default_mm_lws[0], cl_gemm[gemm_type_idx].default_mm_lws[1] };

    /* Enqueue the commands to execute the kernels on a device. */
    queue.enqueueNDRangeKernel( kernel_interleave, cl::NullRange, interleave_gws, cl::NullRange, interleave_events, &interleave_event );
    queue.enqueueNDRangeKernel( kernel_transpose,  cl::NullRange, transpose_gws,  cl::NullRange, transpose_events,  &transpose_event  );

    std::cout << "[multiply]   Global Work Size = [" << mm_gws[0] << ", " << mm_gws[1] << "]" << std::endl;
    if( (!(mm_gws[0] % cl_gemm[gemm_type_idx].default_mm_lws[0])) && (!(mm_gws[1] % cl_gemm[gemm_type_idx].default_mm_lws[1])) )
    {
        std::cout << "[multiply]   Local Work Size = [" << mm_lws[0] << ", " << mm_lws[1] << "]" << std::endl;
        queue.enqueueNDRangeKernel( kernel_mm, cl::NullRange, mm_gws, mm_lws, multiply_events, &multiply_event );
    }
    else
    {
        std::cout << "[multiply]   Local Work Size = NULL" << std::endl;
        queue.enqueueNDRangeKernel( kernel_mm, cl::NullRange, mm_gws, cl::NullRange, multiply_events, &multiply_event );
    }

    /* Do not enqueue the finalize kernel if alpha == 1 and beta == 0 */
    if( (alpha != 1.0f) || (beta != 0.0f) )
    {
        queue.enqueueNDRangeKernel( kernel_finalize, cl::NullRange, finalize_gws, cl::NullRange, finalize_events, &finalize_event );
    }
    else
    {
        std::cout << "[finalize]   skipped as alpha = 1 and beta = 0" << std::endl;
    }

    /* Wait for completion. */
    queue.finish();

    /* Print profiling info for the kernels and get the execution time. */
    cl_ulong interleave_ns = get_profiling_info( "[interleave]", interleave_event );
    cl_ulong transpose_ns  = get_profiling_info( "[transpose] ", transpose_event  );
    cl_ulong multiply_ns   = get_profiling_info( "[multiply]  ", multiply_event   );
    cl_ulong finalize_ns   = get_profiling_info( "[finalize]  ", finalize_event   );
    /* Calculate nflops and then GFLOPS (nflops / ns). */
    {
        // C[M][K] = A[M][N] * B[N][K]
        const cl_ulong M = mtx_a.rows;
        const cl_ulong N = mtx_a.cols;
        const cl_ulong K = mtx_b.cols;
        const cl_ulong flops = 2 * M * N * K; // FIXME: Correct in the first approximation.
        const cl_double gflops_per_s = (cl_double) flops / (cl_double) multiply_ns;
        std::cout << "Calculating performance... " << gflops_per_s << " Gflops/s";
        std::cout << " (performed "  << flops << " flops in " << multiply_ns << " ns)." << std::endl;
#if (1 == XOPENME)
        xopenme_add_var_i(openme.var_count++, (char*) "  \"INTERLEAVE#ns\":%u",   interleave_ns);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"INTERLEAVE#us\":%.3f", interleave_ns * 1e-3);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"INTERLEAVE#ms\":%.3f", interleave_ns * 1e-6);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"INTERLEAVE#s\":%.3f",  interleave_ns * 1e-9);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_i(openme.var_count++, (char*) "  \"TRANSPOSE#ns\":%u",    transpose_ns);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"TRANSPOSE#us\":%.3f",  transpose_ns * 1e-3);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"TRANSPOSE#ms\":%.3f",  transpose_ns * 1e-6);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"TRANSPOSE#s\":%.3f",   transpose_ns * 1e-9);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_i(openme.var_count++, (char*) "  \"EXECUTION#ns\":%u",    multiply_ns);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"EXECUTION#us\":%.3f",  multiply_ns * 1e-3);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"EXECUTION#ms\":%.3f",  multiply_ns * 1e-6);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"EXECUTION#s\":%.3f",   multiply_ns * 1e-9);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_i(openme.var_count++, (char*) "  \"EXECUTION#flops\":%u", flops);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"EXECUTION#Gflops\":%.3f", flops * 1e-9);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_d(openme.var_count++, (char*) "  \"EXECUTION#Gflops/s\":%.3lf", gflops_per_s);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_i(openme.var_count++, (char*) "  \"FINALIZE#ns\":%u",    finalize_ns);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"FINALIZE#us\":%.3f",  finalize_ns * 1e-3);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"FINALIZE#ms\":%.3f",  finalize_ns * 1e-6);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");

        xopenme_add_var_f(openme.var_count++, (char*) "  \"FINALIZE#s\":%.3f",   finalize_ns * 1e-9);
        assert(openme.var_count_below_max() && "xOpenME max var count reached.");
#endif
    }

    /* Validate the output results. */
    if (skip_validation==0)
    {
        void *ptr_out = NULL;

        /* Map output buffer. */
        ptr_out = queue.enqueueMapBuffer( buffer_mtx_out, CL_TRUE, CL_MAP_READ, 0, buffer_mtx_c_size );

        void* actual = malloc( buffer_expected_size );

        size_t stride_src = mtx_b.internal_cols * (static_cast<gemm_type>(gemm_type_idx) == FP32 ? sizeof(
                                                                fp32) : sizeof(fp16));
        size_t stride_dst = mtx_b.cols * (static_cast<gemm_type>(gemm_type_idx) == FP32 ? sizeof(
                                                                fp32) : sizeof(fp16));

        for(int32_t y = 0; y < mtx_a.rows; ++y)
        {
            memcpy( (uint8_t*)actual + y * stride_dst, (uint8_t*)ptr_out + y * stride_src, stride_dst);
        }

        verify_result(static_cast<gemm_type>(gemm_type_idx), mtx_a.rows, mtx_b.cols, expected, actual);

        /* Unmap output buffer. */
        queue.enqueueUnmapMemObject( buffer_mtx_out, ptr_mtx_out );

        /* Free allocated memory. */
        free(expected);
        free(actual);
    }
}


int main(int argc, const char **argv)
{
    return mali::run_mali_sample(argc, argv, sgemm, "SGEMM - Single precision floating point GEneral Matrix Matrix multiplication");
}
