/*addedd by li yang, date 2025-07-06
 */

#ifndef _SMART_COMMON_H
#define _SMART_COMMON_H

#include "sm_log.h"
#include "sm_types.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdio.h>
#include <string>
using namespace std;

#define SM_DATA_TYPE_DAT 0
#define SM_DATA_TYPE_CONV 1

#define SM_LAYOUT_TYPE_MAT_HW 0
#define SM_LAYOUT_TYPE_MAT_WHW32 1
#define SM_LAYOUT_TYPE_CONV_CHW 0
#define SM_LAYOUT_TYPE_CONV_CHWC32 1

#define SM_DATA_FMT_INT8 0
#define SM_DATA_FMT_INT16 2
#define SM_DATA_FMT_FP16 3
#define SM_DATA_FMT_BF16 4
#define SM_DATA_FMT_FP32 5

// 2d matrix
template <typename T = float>
class MATRIX_T
{
public:
    int nr, nc; // number of rows and columns
    T **data;   // 2D array to store matrix data

    // Constructor
    MATRIX_T() : nr(0), nc(0), data(nullptr) {}

    // Destructor
    ~MATRIX_T()
    {
        free_mat();
    }

    // Reset matrix to empty state
    void reset()
    {
        free_mat();
        nr = 0;
        nc = 0;
        data = nullptr;
    }

    // Free allocated memory
    void free_mat()
    {
        if (data != nullptr)
        {
            for (int i = 0; i < nr; i++)
            {
                delete[] data[i];
            }
            delete[] data;
            data = nullptr;
        }
    }

    // Initialize matrix with data from 1D array (row-major order)
    void set_data_row(T input_data[], int rows, int cols)
    {
        free_mat();
        nr = rows;
        nc = cols;
        // Allocate memory for row pointers
        data = new T *[nr];
        // Allocate memory for each row and copy data
        for (int i = 0; i < nr; i++)
        {
            data[i] = new T[nc];
            for (int j = 0; j < nc; j++)
            {
                data[i][j] = input_data[i * nc + j];
            }
        }
    }

    void set_data_col(T input_data[], int rows, int cols)
    {
        free_mat();
        nr = rows;
        nc = cols;
        data = new T *[nr];
        for (int i = 0; i < nr; i++)
        {
            data[i] = new T[nc];
        }
        for (int j = 0; j < nc; j++)
        {
            for (int i = 0; i < nr; i++)
            {
                data[i][j] = input_data[j * nr + i];
            }
        }
    }
};

struct llm_insn_info_t
{
    unsigned data_type;
    unsigned layout_type;
    unsigned idata_format;
    unsigned odata_format;
    bool fusion;
    bool quantize;
    unsigned rsv_bits;

    int get_byte_from_fmt(unsigned fmt)
    {
        int fmt_byte = 2;
        switch (fmt)
        {
        case 0:
        case 1:
            fmt_byte = 1;
            break;
        case 2:
        case 3:
        case 4:
            fmt_byte = 2;
            break;
        case 5:
            fmt_byte = 4;
            break;
        default:
            break;
        }
        return fmt_byte;
    }
    int idata_bytes() { return get_byte_from_fmt(idata_format); }
    int odata_bytes() { return get_byte_from_fmt(odata_format); }
};

struct dma_insn_info_t
{
    uint32_t base_id;
    uint32_t op;
    uint32_t fmt;
    uint32_t tensor_dim;
    bool trans_en;
    uint32_t data_type;
    uint32_t mat_type;
    uint8_t is_kv;
    uint8_t rsv_bits;
    int get_bits_from_fmt()
    {
        int fmt_bits = 16;
        switch (fmt)
        {
        case 0:
            fmt_bits = 4;
            break;
        case 1:
            fmt_bits = 8;
            break;
        case 2:
            fmt_bits = 16;
            break;
        case 3:
            fmt_bits = 32;
            break;
        default:
            break;
        }
        return fmt_bits;
    }
};

struct tmm_insn_info_t
{
    uint32_t base_id;
    bool pproc_en;
    bool is_remote_ld;
    bool fusion;
};

struct ddep_gen_insn_info_t
{
    uint32_t cmd_type;
    uint32_t time_stp;
    uint32_t uc;
    uint32_t rs_id;

    void print() const
    {
        sm_log_info("DDEP gen <cmd_type:%u,time_stp:%u, uc:%u, id:%u>", cmd_type, time_stp, uc, rs_id);
    }
};

struct ddep_use_insn_info_t
{
    uint32_t cmd_type;
    uint32_t time_stp;
    uint32_t rs_id1;
    uint32_t rs_id2;
    uint32_t rs_id3;
    uint32_t func3;

    void print() const
    {
        sm_log_info("DDEP gen <cmd_type:%u,time_stp:%u, id_vld:%u, id[1 2 3]:[%u %u %u]>", cmd_type, time_stp, func3, rs_id1, rs_id2, rs_id3);
    }
};

struct ddep_rls_insn_info_t
{
    uint32_t cmd_type;
    uint32_t time_stp;
    uint32_t rs_id;

    void print() const
    {
        sm_log_info("DDEP gen <cmd_type:%u,time_stp:%u, id:%u>", cmd_type, time_stp, rs_id);
    }
};

// fp16 to fp32 func
typedef unsigned short Half;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef union suf32
{
    int i;
    unsigned u;
    float f;
} suf32;

uint as_uint(const float x);
float as_float(const uint x);
float half_to_float(Half h);
Half float_to_half(float f);
void float_to_half_array(Half *dst, float *src, int size);
void half_to_float_array(float *dst, Half *src, int size);
Half half_add(Half a, Half b);
Half half_sub(Half a, Half b);
Half half_mul(Half a, Half b);
Half half_div(Half a, Half b);
Half half_min(Half a, Half b);
Half half_max(Half a, Half b);

#define COMPUTE_OTILE_M_SIZE 128;
#define COMPUTE_OTILE_N_SIZE 128;
#define COMPUTE_OTILE_M_SIZE 128;

// TENSOR INSTRUCTION
class TSMATMULDesc
{
public:
    TSMATMULDesc() { reset(); }

    uint64_t dst_box_addr : 40;
    uint32_t src_act_k : 24;
    uint64_t src_act_addr : 40;
    uint64_t src_wgt_addr : 24;
    uint64_t src_wgt_mask_addr : 24;

    uint32_t dst_n : 24;
    uint32_t dst_m : 16;
    uint32_t dst_b : 16;

    uint32_t dst_box_start_n : 24;
    uint32_t dst_box_start_m : 16;
    uint32_t dst_box_start_b : 16;

    uint32_t dst_box_n : 16;
    uint32_t dst_box_m : 16;
    uint32_t dst_box_b : 16;

    uint8_t dst_format : 4;
    uint8_t src_act_format : 3;
    uint8_t src_wgt_format : 3;

    bool is_out;
    bool is_acc_pre_tensor;
    uint32_t out_layout_fmt;

    uint64_t src_wgt_quant_param_addr : 24;
    uint64_t src_act_quant_param_addr : 24;
    bool quant_param_fmt;

    uint8_t act_dyna_quant_mode : 4;
    bool act_dyna_quant_is_sym;
    uint32_t act_per_group_size : 8;
    uint32_t act_per_bloack_size : 8;
    uint32_t wgt_quant_mode : 4;
    bool wgt_quant_is_sym;
    uint32_t wgt_per_group_size : 8;
    uint32_t wgt_per_bloack_size : 8;

    bool is_gather_ld_act;
    uint32_t act_gather_index_addr : 24;
    uint16_t act_gather_index_len : 8;
    uint64_t src_wgt_quant_zero_param_addr : 24;

    bool is_acc_square_sum;
    bool is_first_n_tile;
    bool is_last_n_tile;
    uint64_t square_sum_dst_addr : 24;

    void reset()
    {
        dst_box_addr = 0;
        src_act_k = 256;
        src_act_addr = 0;
        src_wgt_addr = 0;
        src_wgt_mask_addr = 0;

        dst_n = 1;
        dst_m = 1;
        dst_b = 1;

        dst_box_start_n = 0;
        dst_box_start_m = 0;
        dst_box_start_b = 0;

        dst_box_n = 128;
        dst_box_m = 128;
        dst_box_b = 1;

        dst_format = 4;
        src_act_format = 4;
        src_wgt_format = 3;

        is_out = false;
        is_acc_pre_tensor = false;
        out_layout_fmt = 0;

        src_wgt_quant_param_addr = 0;
        src_act_quant_param_addr = 0;
        quant_param_fmt = false;

        act_dyna_quant_mode = 0;
        act_dyna_quant_is_sym = false;
        act_per_group_size = 0;
        act_per_bloack_size = 0;
        wgt_quant_mode = 0;
        wgt_quant_is_sym = false;
        wgt_per_group_size = 0;
        wgt_per_bloack_size = 0;

        is_gather_ld_act = false;
        act_gather_index_addr = false;
        act_gather_index_len = 0;
        src_wgt_quant_zero_param_addr = 0;

        is_acc_square_sum = false;
        is_first_n_tile = false;
        is_last_n_tile = false;
        square_sum_dst_addr = 0;
    }

    void set_base0(uint64_t data)
    {
        dst_box_addr = data & ((1UL << 40) - 1);
        src_act_k = (data >> 40) & ((1UL << 24) - 1);
    }

    void set_base1(uint64_t data)
    {
        src_act_addr = (data >> 40) & ((1UL << 40) - 1);
    }

    void set_base2(uint64_t data)
    {
        src_wgt_addr = data & ((1UL << 24) - 1);
        src_wgt_mask_addr = (data >> 24) & ((1UL << 24) - 1);
    }

    void set_base3(uint64_t data)
    {
        dst_n = data & ((1UL << 24) - 1);
        dst_m = (data >> 24) & 0xffff;
        dst_b = (data >> 40) & 0xffff;
    }

    void set_base4(uint64_t data)
    {
        dst_box_start_n = data & ((1UL << 24) - 1);
        dst_box_start_m = (data >> 24) & 0xffff;
        dst_box_start_b = (data >> 40) & 0xffff;
    }

    void set_base5(uint64_t data)
    {
        dst_box_n = data & ((1UL << 24) - 1);
        dst_box_m = (data >> 16) & 0xffff;
        dst_box_b = (data >> 32) & 0xffff;
    }

    void set_base6(uint64_t data)
    {
        dst_format = data & 0xf;
        src_act_format = (data >> 8) & 0x7;
        src_wgt_format = (data >> 16) & 0x7;

        is_out = (data >> 24) & 0x1;
        is_acc_pre_tensor = (data >> 32) & 0x1;
        out_layout_fmt = (data >> 40) & 0x1;
    }
    void set_base7(uint64_t data)
    {
        src_wgt_quant_param_addr = data & ((1UL << 24) - 1);
        src_act_quant_param_addr = (data >> 24) & ((1UL << 24) - 1);
        quant_param_fmt = (data >> 48) & 0x1;
    }

    void set_base8(uint64_t data)
    {
        act_dyna_quant_mode = data & 0xf;
        act_dyna_quant_is_sym = (data >> 8) & 0x1;
        act_per_group_size = (data >> 16) & 0xff;
        act_per_bloack_size = (data >> 24) & 0xff;
        wgt_quant_mode = (data >> 32) & 0xf;
        wgt_quant_is_sym = (data >> 40) & 0x1;
        wgt_per_group_size = (data >> 48) & 0xff;
        wgt_per_bloack_size = (data >> 56) & 0xff;
    }

    void set_base9(uint64_t data)
    {
        is_gather_ld_act = data & 0x1;
        act_gather_index_addr = (data >> 8) & ((1UL << 24) - 1);
        act_gather_index_len = (data >> 32) & 0xff;
        src_wgt_quant_zero_param_addr = (data >> 40) & ((1UL << 24) - 1);
    }

    void set_base10(uint64_t data)
    {
        is_acc_square_sum = data & 0x1;
        is_first_n_tile = (data >> 8) & 0x1;
        is_last_n_tile = (data >> 16) & 0x1;
        square_sum_dst_addr = (data >> 24) & ((1UL << 24) - 1);
    }

    int get_bits_from_dstfmt()
    {
        int fmt_bits = 16;
        switch ((int)dst_format)
        {
        case 0:
        case 1:
            fmt_bits = 8;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            fmt_bits = 16;
            break;
        case 6:
        case 7:
        case 8:
            fmt_bits = 32;
            break;
        default:
            break;
        }
        return fmt_bits;
    }

    int get_bits_from_actfmt()
    {
        int fmt_bits = 16;
        switch ((int)src_act_format)
        {
        case 0:
        case 1:
        case 6:
        case 7:
            fmt_bits = 8;
            break;
        case 2:
        case 3:
        case 4:
            fmt_bits = 16;
            break;
        default:
            break;
        }
        return fmt_bits;
    }

    int get_bits_from_wgtfmt()
    {
        int fmt_bits = 16;
        switch ((int)src_act_format)
        {
        case 0:
        case 1:
            fmt_bits = 4;
            break;
        case 2:
        case 5:
        case 6:
            fmt_bits = 8;
            break;
        case 3:
        case 4:
            fmt_bits = 16;
            break;
        default:
            break;
        }
        return fmt_bits;
    }

    void print_info()
    {
        sm_log_info("TSMATMULDESC <dest_box_addr:0x%lx, src_act_addr:0x%lx, src_wgt_addr:0x%lx, src_act_k:%u\n dst size[m,n,b]:[%u,%u,%u], dst box start size[m,n,b]:[%u,%u,%u], dst box size[m,n,b]:[%u,%u,%u], dst_format: %d, src_act_format: %d, src_wgt_format: %d, ", dst_box_addr, src_act_addr, src_wgt_addr, src_act_k, dst_m, dst_n, dst_b, dst_box_start_m, dst_box_start_n, dst_box_start_b, dst_box_m, dst_box_n, dst_box_b, dst_format, src_act_format, src_wgt_format);
    }
};

class PPROCDesc
{
public:
    PPROCDesc() { reset(); }
    uint64_t pproc_param_base : 40;
    bool pproc_param_fmt;

    bool pproc_x_scale_en;
    bool pproc_x_scale_perchn;
    bool pproc_x_bias_en;
    bool pproc_x_bias_perchn;

    bool pproc_c_scale_en;
    bool pproc_c_scale_perchn;
    bool pproc_c_bias_en;
    bool pproc_c_bias_perchn;

    bool pproc_y_relu_scale_en;
    bool pproc_y_relu_param_perchn;
    bool pproc_y_relu_en;
    bool pproc_y_sfu_en;

    bool pproc_y_scale_en;

    bool pproc_y_relu_op : 2;
    bool pproc_y_sfu_op : 2;
    void reset()
    {
        pproc_param_base = 0;
        pproc_param_fmt = false;

        pproc_x_scale_en = false;
        pproc_x_scale_perchn = false;
        pproc_x_bias_en = false;
        pproc_x_bias_perchn = false;

        pproc_c_scale_en = false;
        pproc_c_scale_perchn = false;
        pproc_c_bias_en = false;
        pproc_c_bias_perchn = false;

        pproc_y_relu_scale_en = false;
        pproc_y_relu_param_perchn = false;
        pproc_y_relu_en = false;
        pproc_y_sfu_en = false;

        pproc_y_scale_en = false;

        pproc_y_relu_op = 0;
        pproc_y_sfu_op = 0;
    }

    void set_base0(uint64_t data)
    {
        pproc_param_base = data & ((1UL << 40) - 1);
        pproc_param_fmt = (data >> 40) & 0x1;

        pproc_x_scale_en = (data >> 41) & 0x1;
        pproc_x_scale_perchn = (data >> 42) & 0x1;
        pproc_x_bias_en = (data >> 43) & 0x1;
        pproc_x_bias_perchn = (data >> 44) & 0x1;

        pproc_c_scale_en = (data >> 45) & 0x1;
        pproc_c_scale_perchn = (data >> 46) & 0x1;
        pproc_c_bias_en = (data >> 47) & 0x1;
        pproc_c_bias_perchn = (data >> 48) & 0x1;

        pproc_y_relu_scale_en = (data >> 49) & 0x1;
        pproc_y_relu_param_perchn = (data >> 50) & 0x1;
        pproc_y_relu_en = (data >> 51) & 0x1;
        pproc_y_sfu_en = (data >> 52) & 0x1;

        pproc_y_scale_en = (data >> 53) & 0x1;

        pproc_y_relu_op = (data >> 56) & 0x3;
        pproc_y_sfu_op = (data >> 58) & 0x3;
    }

    void print_info() { sm_log_info("pprocdesc <swish_en:%d>", pproc_y_scale_en); }
};

// elementwise:(m,n)
class ELWDesc
{
public:
    ELWDesc() { reset(); }

    uint8_t elw_op : 3; // 0:nop.1:mul.2:min,3:max,4:add,5:sub
    bool elw_is_gather_ld;

    bool pproc_y_elw_scale_en;
    bool pproc_y_elw_scale_perchn;
    bool pproc_y_elw_bias_en;
    bool pproc_y_elw_bias_perchn;
    uint8_t pproc_y_elw_fmt : 3; // 0-i8,1-u8,2-i16,3-u16,4-fp6,5-bf16

    uint32_t pproc_y_elw_m : 16;
    uint32_t pproc_y_elw_n : 16;
    uint32_t pproc_y_box_start_elw_m : 16;
    uint32_t pproc_y_box_start_elw_n : 16;
    uint64_t pproc_y_elw_box_src_addr : 24; // L1 buf addr for elw_op

    void reset()
    {
        elw_op = 0; // Initialize 3-bit field to 0 (nop)
        elw_is_gather_ld = false;

        pproc_y_elw_scale_en = false;
        pproc_y_elw_scale_perchn = false;
        pproc_y_elw_bias_en = false;
        pproc_y_elw_bias_perchn = false;
        pproc_y_elw_fmt = 0; // Initialize 3-bit field to 0 (i8)

        pproc_y_elw_m = 0;
        pproc_y_elw_n = 0;
        pproc_y_box_start_elw_m = 0;
        pproc_y_box_start_elw_n = 0;
        pproc_y_elw_box_src_addr = 0;
    }

    void set_base0(uint64_t data)
    {
        elw_op = (data >>= 0) & 0x1;
        elw_is_gather_ld = (data >>= 8) & 0x1;

        pproc_y_elw_scale_en = (data >>= 8) & 0x1;
        pproc_y_elw_scale_perchn = (data >>= 8) & 0x1;
        pproc_y_elw_bias_en = (data >>= 8) & 0x1;
        pproc_y_elw_bias_perchn = (data >>= 8) & 0x1;
        pproc_y_elw_fmt = (data >>= 8) & 0x7;
    }

    void set_base1(uint64_t data)
    {
        pproc_y_elw_m = (data >>= 0) & 0xffff;
        pproc_y_elw_n = (data >>= 16) & 0xffff;
        pproc_y_box_start_elw_m = (data >>= 16) & 0xffff;
        pproc_y_box_start_elw_n = (data >>= 16) & 0xffff;
    }

    void set_base2(uint64_t data)
    {
        pproc_y_elw_box_src_addr = data & 0xffffff;
    }

    int get_bits_from_elwfmt()
    {
        int fmt_bits = 16;
        switch ((int)pproc_y_elw_fmt)
        {
        case 0:
        case 1:
        case 6:
        case 7:
            fmt_bits = 8;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            fmt_bits = 16;
            break;
        default:
            break;
        }
        return fmt_bits;
    }

    void print_info()
    {
        sm_log_info("ELWDesc <elw_op:%d, elw_box_addr:0x%lx, elw_fmt:%d, elw_size[m,n]:[%u,%u], elw_box_start:[%u,%u]>", elw_op, pproc_y_elw_box_src_addr, pproc_y_elw_fmt, pproc_y_elw_m, pproc_y_elw_n, pproc_y_box_start_elw_m, pproc_y_box_start_elw_n);
    }
};

class TCONVDesc
{
public:
    TCONVDesc() { reset(); }
    uint64_t dst_addr : 40;
    uint64_t src_act_addr : 40;
    uint64_t src_wgt_addr : 24;
    uint64_t src_wgt_mask_addr : 24;

    uint32_t dst_channel : 16;
    uint32_t dst_width : 16;
    uint32_t dst_height : 16;
    uint32_t dst_batch : 16;

    uint32_t dst_start_channel : 16;
    uint32_t dst_start_width : 16;
    uint32_t dst_start_height : 16;
    uint32_t dst_start_batch : 16;

    uint32_t dst_box_channel : 16;
    uint32_t dst_box_width : 16;
    uint32_t dst_box_height : 16;
    uint32_t dst_box_batch : 16;

    uint32_t src_act_channel : 16;
    uint32_t src_act_width : 16;
    uint32_t src_act_height : 16;
    uint32_t src_act_batch : 16;

    uint8_t dst_fmt : 3;
    uint8_t arc_act_fmt : 3;
    uint8_t arc_wgt_fmt : 3;
    bool wgt_sparse_en;
    bool wgt_sparse_mode; // 0-2:4, 1-4:8
    bool depthwise_en;

    uint8_t kernel_size_w : 8;
    uint8_t kernel_size_h : 8;
    uint8_t stride_w : 8;
    uint8_t stride_h : 8;
    uint8_t dilation_w : 8;
    uint8_t dilation_h : 8;

    uint8_t padding_mode : 2;
    uint8_t padding_size_0 : 5;
    uint8_t padding_size_1 : 5;
    uint8_t padding_size_2 : 5;
    uint8_t padding_size_3 : 5;
    uint8_t padding_value_high8 : 8;
    uint8_t padding_value_low8 : 8;

    void reset()
    {
        // Address fields
        dst_addr = 0;
        src_act_addr = 0;
        src_wgt_addr = 0;
        src_wgt_mask_addr = 0;

        // Destination dimensions
        dst_channel = 1;
        dst_width = 1;
        dst_height = 1;
        dst_batch = 1;

        // Destination start positions
        dst_start_channel = 0;
        dst_start_width = 0;
        dst_start_height = 0;
        dst_start_batch = 0;

        // Destination box dimensions
        dst_box_channel = 1;
        dst_box_width = 1;
        dst_box_height = 1;
        dst_box_batch = 1;

        // Source activation dimensions
        src_act_channel = 1;
        src_act_width = 1;
        src_act_height = 1;
        src_act_batch = 1;

        // Format and mode flags
        dst_fmt = 4;
        arc_act_fmt = 4;
        arc_wgt_fmt = 2;
        wgt_sparse_en = false;
        wgt_sparse_mode = false;
        depthwise_en = false;

        // Kernel parameters
        kernel_size_w = 1;
        kernel_size_h = 1;
        stride_w = 1;
        stride_h = 1;
        dilation_w = 0;
        dilation_h = 0;

        // Padding parameters
        padding_mode = 0;
        padding_size_0 = 0;
        padding_size_1 = 0;
        padding_size_2 = 0;
        padding_size_3 = 0;
        padding_value_high8 = 0;
        padding_value_low8 = 0;
    }

    void set_base0(uint64_t data)
    {
        dst_addr = (data >>= 0) & ((1UL << 40) - 1);
    }

    void set_base1(uint64_t data)
    {
        src_act_addr = (data >>= 0) & ((1UL << 40) - 1);
    }

    void set_base2(uint64_t data)
    {
        src_wgt_addr = (data >>= 0) & ((1UL << 24) - 1);
        src_wgt_mask_addr = (data >>= 24) & ((1UL << 24) - 1);
    }

    void set_base3(uint64_t data)
    {
        dst_channel = (data >>= 0) & ((1UL << 16) - 1);
        dst_width = (data >>= 16) & ((1UL << 16) - 1);
        dst_height = (data >>= 16) & ((1UL << 16) - 1);
        dst_batch = (data >>= 16) & ((1UL << 16) - 1);
    }

    void set_base4(uint64_t data)
    {
        dst_start_channel = (data >>= 0) & ((1UL << 16) - 1);
        dst_start_width = (data >>= 16) & ((1UL << 16) - 1);
        dst_start_height = (data >>= 16) & ((1UL << 16) - 1);
        dst_start_batch = (data >>= 16) & ((1UL << 16) - 1);
    }

    void set_base5(uint64_t data)
    {
        dst_box_channel = (data >>= 0) & ((1UL << 16) - 1);
        dst_box_width = (data >>= 16) & ((1UL << 16) - 1);
        dst_box_height = (data >>= 16) & ((1UL << 16) - 1);
        dst_box_batch = (data >>= 16) & ((1UL << 16) - 1);
    }

    void set_base6(uint64_t data)
    {
        src_act_channel = (data >>= 0) & ((1UL << 16) - 1);
        src_act_width = (data >>= 16) & ((1UL << 16) - 1);
        src_act_height = (data >>= 16) & ((1UL << 16) - 1);
        src_act_batch = (data >>= 16) & ((1UL << 16) - 1);
    }

    void set_base7(uint64_t data)
    {
        dst_fmt = (data >>= 0) & 0xf;
        arc_act_fmt = (data >>= 8) & 0x7;
        arc_wgt_fmt = (data >>= 8) & 0x7;
        wgt_sparse_en = (data >>= 8) & 0x1;
        wgt_sparse_mode = (data >>= 1) & 0x1;
    }

    void set_base8(uint64_t data)
    {
        kernel_size_w = (data >>= 0) & 0xff;
        kernel_size_h = (data >>= 8) & 0xff;
        stride_w = (data >>= 8) & 0xff;
        stride_h = (data >>= 8) & 0xff;
        dilation_w = (data >>= 8) & 0xff;
        dilation_h = (data >>= 8) & 0xff;
    }

    void set_base9(uint64_t data)
    {
        padding_mode = (data >>= 0) & 0x3;
        padding_size_0 = (data >>= 8) & 0x1f;
        padding_size_1 = (data >>= 8) & 0x1f;
        padding_size_2 = (data >>= 8) & 0x1f;
        padding_size_3 = (data >>= 8) & 0x1f;
        padding_value_high8 = (data >>= 8) & 0xff;
        padding_value_low8 = (data >>= 8) & 0xff;
    }

    int get_bits_from_dstfmt()
    {
        int fmt_bits = 16;
        switch ((int)dst_fmt)
        {
        case 0:
        case 1:
            fmt_bits = 8;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            fmt_bits = 16;
            break;
        case 6:
        case 7:
        case 8:
            fmt_bits = 32;
            break;
        default:
            break;
        }
        return fmt_bits;
    }

    int get_bits_from_actfmt()
    {
        int fmt_bits = 16;
        switch ((int)dst_fmt)
        {
        case 0:
        case 1:
            fmt_bits = 8;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            fmt_bits = 16;
            break;
        default:
            break;
        }
        return fmt_bits;
    }

    int get_bits_from_wgtfmt()
    {
        int fmt_bits = 16;
        switch ((int)dst_fmt)
        {
        case 0:
        case 1:
            fmt_bits = 4;
            break;
        case 2:
            fmt_bits = 8;
            break;
        case 3:
        case 4:
            fmt_bits = 16;
            break;
        default:
            break;
        }
        return fmt_bits;
    }

    int get_src_act_height()
    {
        int input_h, pad_size;
        pad_size = padding_size_1 + padding_size_3;
        input_h = (dst_box_height - 1) * stride_h - pad_size + dilation_h * (kernel_size_h - 1) + 1;
        return input_h;
    }

    int get_src_act_width()
    {
        int input_w, pad_size;
        pad_size = padding_size_0 + padding_size_2;
        input_w = (dst_box_width - 1) * stride_w - pad_size + dilation_w * (kernel_size_w - 1) + 1;
        return input_w;
    }

    void print_info()
    {
        sm_log_info(
            "TCONVDesc <"
            "dst:0x%lx act:0x%lx wgt:0x%lx mask:0x%lx | "
            "dst_dim[ch:%u w:%u h:%u b:%u] | "
            "src_dim[ch:%u w:%u h:%u b:%u] | "
            "fmt[dst:%u act:%u wgt:%u] | "
            "kernel[%ux%u] stride[%ux%u] dil[%ux%u] | "
            "sparse[%d mode:%d] depthwise:%d | "
            "pad[mode:%d size:[%u,%u,%u,%u] val[0x%02x%02x]>",
            dst_addr, src_act_addr, src_wgt_addr, src_wgt_mask_addr,
            dst_channel, dst_width, dst_height, dst_batch,
            src_act_channel, src_act_width, src_act_height, src_act_batch,
            dst_fmt, arc_act_fmt, arc_wgt_fmt,
            kernel_size_w, kernel_size_h, stride_w, stride_h, dilation_w, dilation_h,
            wgt_sparse_en, wgt_sparse_mode, depthwise_en,
            padding_mode, padding_size_0, padding_size_1, padding_size_2, padding_size_3,
            padding_value_high8, padding_value_low8);
    }
};

class DMALSDesc
{
public:
    DMALSDesc() { reset(); }

    uint8_t dma_op : 4;
    uint8_t space_block_size : 3;
    uint8_t d2s_size : 4;
    uint8_t ic_extend_times : 3;
    uint8_t ic_origin_size : 4;
    uint8_t cp_direct : 2;
    bool is_ddr_l1buf;
    bool is_ddr_wgt;
    uint8_t remote_code : 4;
    uint8_t dma_dtype : 3;

    uint64_t ddr_global_base_addr : 48;
    uint64_t l1buf_tile_base_addr : 48;

    uint32_t global_stride_0 : 24;
    uint32_t global_stride_1 : 24;
    uint32_t global_stride_2 : 24;
    uint32_t global_stride_3 : 24;

    uint32_t global_dim_0 : 16;
    uint32_t global_dim_1 : 24;
    uint32_t global_dim_2 : 16;
    uint32_t global_dim_3 : 8;

    uint32_t box_dim_0 : 16;
    uint32_t box_dim_1 : 24;
    uint32_t box_dim_2 : 16;
    uint32_t box_dim_3 : 8;

    uint32_t tensor_coords_0 : 16;
    uint32_t tensor_coords_1 : 24;
    uint32_t tensor_coords_2 : 16;
    uint32_t tensor_coords_3 : 8;

    uint32_t l1_tile_dim_0 : 16;
    uint32_t l1_tile_dim_1 : 24;
    uint32_t l1_tile_dim_2 : 16;
    uint32_t l1_tile_dim_3 : 8;

    uint32_t l1_box_start_dim_0 : 16;
    uint32_t l1_box_start_dim_1 : 24;
    uint32_t l1_box_start_dim_2 : 16;
    uint32_t l1_box_start_dim_3 : 8;

    uint8_t kv_intc_size : 4;
    uint64_t kv_intc_stride : 40;
    uint32_t kv_chunk_seq_dim : 10;
    uint64_t kv_chunk_stride : 32;
    uint32_t kv_sliding_wd_size : 16;
    uint32_t kv_ring_ld_start_addr_indx : 16;
    uint32_t gather_inx_l1_addr : 24;
    uint32_t gather_inx_len : 16;

    void reset()
    {
        dma_op = 0;
        space_block_size = 0;
        d2s_size = 0;
        ic_extend_times = 0;
        ic_origin_size = 0;
        cp_direct = 0;
        is_ddr_l1buf = false;
        is_ddr_wgt = false;
        remote_code = 0;
        dma_dtype = 0;

        ddr_global_base_addr = 0;
        l1buf_tile_base_addr = 0;

        global_stride_0 = 0;
        global_stride_1 = 0;
        global_stride_2 = 0;
        global_stride_3 = 0;

        global_dim_0 = 0;
        global_dim_1 = 0;
        global_dim_2 = 0;
        global_dim_3 = 0;

        box_dim_0 = 0;
        box_dim_1 = 0;
        box_dim_2 = 0;
        box_dim_3 = 0;

        tensor_coords_0 = 0;
        tensor_coords_1 = 0;
        tensor_coords_2 = 0;
        tensor_coords_3 = 0;

        l1_tile_dim_0 = 0;
        l1_tile_dim_1 = 0;
        l1_tile_dim_2 = 0;
        l1_tile_dim_3 = 0;

        l1_box_start_dim_0 = 0;
        l1_box_start_dim_1 = 0;
        l1_box_start_dim_2 = 0;
        l1_box_start_dim_3 = 0;

        kv_intc_size = 0;
        kv_intc_stride = 0;
        kv_chunk_seq_dim = 0;
        kv_chunk_stride = 0;
        kv_sliding_wd_size = 0;
        kv_ring_ld_start_addr_indx = 0;
        gather_inx_l1_addr = 0;
        gather_inx_len = 0;
    }

    void set_base0(uint64_t data)
    {
        dma_op = (data >>= 0) & 0xf;
        space_block_size = (data >>= 8) & 0x7;
        d2s_size = (data >>= 4) & 0xf;
        ic_extend_times = (data >>= 4) & 0x7;
        ic_origin_size = (data >>= 8) & 0xf;
        cp_direct = (data >>= 28) & 0x3;
        is_ddr_l1buf = (data >>= 2) & 0x1;
        is_ddr_wgt = (data >>= 1) & 0x1;
        remote_code = (data >>= 1) & 0xf;
        dma_dtype = (data >>= 4) & 0x7;
    }

    void set_base1(uint64_t data)
    {
        ddr_global_base_addr = data & ((1UL << 48) - 1);
    }

    void set_base2(uint64_t data)
    {
        l1buf_tile_base_addr = data & ((1UL << 48) - 1);
    }

    void set_base3(uint64_t data)
    {
        global_stride_0 = data & ((1UL << 24) - 1);
        global_stride_1 = (data >>= 32) & ((1UL << 24) - 1);
    }

    void set_base4(uint64_t data)
    {
        global_stride_2 = data & ((1UL << 24) - 1);
        global_stride_3 = (data >>= 32) & ((1UL << 24) - 1);
    }

    void set_base5(uint64_t data)
    {
        global_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        global_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        global_dim_1 = (data >>= 24) & ((1UL << 16) - 1);
        global_dim_1 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base6(uint64_t data)
    {
        box_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        box_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        box_dim_1 = (data >>= 24) & ((1UL << 16) - 1);
        box_dim_1 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base7(uint64_t data)
    {
        tensor_coords_0 = (data >>= 00) & ((1UL << 16) - 1);
        tensor_coords_1 = (data >>= 16) & ((1UL << 24) - 1);
        tensor_coords_2 = (data >>= 24) & ((1UL << 16) - 1);
        tensor_coords_3 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base8(uint64_t data)
    {
        l1_tile_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        l1_tile_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        l1_tile_dim_2 = (data >>= 24) & ((1UL << 16) - 1);
        l1_tile_dim_3 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base9(uint64_t data)
    {
        l1_box_start_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        l1_box_start_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        l1_box_start_dim_2 = (data >>= 24) & ((1UL << 16) - 1);
        l1_box_start_dim_3 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base10(uint64_t data)
    {
        kv_intc_size = (data >>= 0) & ((1UL << 4) - 1);
        kv_intc_stride = (data >>= 8) & ((1UL << 40) - 1);
        kv_chunk_seq_dim = (data >> 40) & ((1UL << 10) - 1);
    }

    void set_base111(uint64_t data)
    {
        kv_chunk_stride = (data >>= 0) & ((1UL << 32) - 1);
        kv_sliding_wd_size = (data >>= 32) & ((1UL << 16) - 1);
        kv_ring_ld_start_addr_indx = (data >>= 16) & ((1UL << 16) - 1);
    }

    void set_base112(uint64_t data)
    {
        gather_inx_l1_addr = (data >>= 0) & ((1UL << 24) - 1);
        gather_inx_len = (data >>= 32) & ((1UL << 16) - 1);
    }

    void print_info()
    {
        sm_log_info(
            "DMALSDesc <"
            "dma_op:%u space_blk:%u d2s:%u ic_ext:%u ic_org:%u | "
            "cp_dir:%u ddr_l1:%d ddr_wgt:%d remote:0x%x dtype:%u | "
            "ddr_addr:0x%012lx l1buf_addr:0x%012lx | "
            "g_stride[0:%u 1:%u 2:%u 3:%u] | "
            "g_dim[0:%u 1:%u 2:%u 3:%u] | "
            "box_dim[0:%u 1:%u 2:%u 3:%u] | "
            "coords[0:%u 1:%u 2:%u 3:%u] | "
            "l1_tile[0:%u 1:%u 2:%u 3:%u] | "
            "l1_box_start[0:%u 1:%u 2:%u 3:%u] | "
            "kv[intc_sz:%u intc_str:0x%010lx chunk_seq:%u chunk_str:0x%08x] | "
            "kv[sliding:%u ring_ld:%u] | "
            "gather[addr:0x%06x len:%u]>",
            // DMA Control
            dma_op, space_block_size, d2s_size, ic_extend_times, ic_origin_size,
            cp_direct, is_ddr_l1buf, is_ddr_wgt, remote_code, dma_dtype,
            // Addresses
            ddr_global_base_addr, l1buf_tile_base_addr,
            // Global Strides
            global_stride_0, global_stride_1, global_stride_2, global_stride_3,
            // Global Dimensions
            global_dim_0, global_dim_1, global_dim_2, global_dim_3,
            // Box Dimensions
            box_dim_0, box_dim_1, box_dim_2, box_dim_3,
            // Tensor Coordinates
            tensor_coords_0, tensor_coords_1, tensor_coords_2, tensor_coords_3,
            // L1 Tile Dimensions
            l1_tile_dim_0, l1_tile_dim_1, l1_tile_dim_2, l1_tile_dim_3,
            // L1 Box Start
            l1_box_start_dim_0, l1_box_start_dim_1, l1_box_start_dim_2, l1_box_start_dim_3,
            // KV Cache
            kv_intc_size, kv_intc_stride, kv_chunk_seq_dim, (uint32_t)kv_chunk_stride,
            kv_sliding_wd_size, kv_ring_ld_start_addr_indx,
            // Gather
            gather_inx_l1_addr, gather_inx_len);
    }
};

class DMACPDesc
{
public:
    DMACPDesc() { reset(); }

    uint8_t dma_op : 4;
    uint8_t space_block_size : 3;
    uint8_t d2s_size : 4;
    uint8_t ic_extend_times : 3;
    uint8_t ic_origin_size : 4;
    uint8_t reshape_dim : 2;
    uint8_t reshape_type : 2;
    uint8_t layout_convert_type : 2;
    uint8_t cp_direct : 2;
    bool is_ddr_l1buf;
    bool is_ddr_wgt;
    uint8_t remote_code : 4;
    uint8_t dma_dtype : 3;

    uint64_t l1_src_addr : 48;
    uint64_t l1_dst_addr : 48;

    uint32_t global_stride_0 : 24;
    uint32_t global_stride_1 : 24;
    uint32_t global_stride_2 : 24;
    uint32_t global_stride_3 : 24;

    uint32_t src_tile_dim_0 : 16;
    uint32_t src_tile_dim_1 : 24;
    uint32_t src_tile_dim_2 : 16;
    uint32_t src_tile_dim_3 : 8;

    uint32_t box_dim_0 : 16;
    uint32_t box_dim_1 : 24;
    uint32_t box_dim_2 : 16;
    uint32_t box_dim_3 : 8;

    uint32_t dst_tile_dim_0 : 16;
    uint32_t dst_tile_dim_1 : 24;
    uint32_t dst_tile_dim_2 : 16;
    uint32_t dst_tile_dim_3 : 8;

    uint32_t src_box_start_dim_0 : 16;
    uint32_t src_box_start_dim_1 : 24;
    uint32_t src_box_start_dim_2 : 16;
    uint32_t src_box_start_dim_3 : 8;

    uint32_t dst_box_start_dim_0 : 16;
    uint32_t dst_box_start_dim_1 : 24;
    uint32_t dst_box_start_dim_2 : 16;
    uint32_t dst_box_start_dim_3 : 8;

    void reset()
    {
        // DMA 控制字段
        dma_op = 0;
        space_block_size = 0;
        d2s_size = 0;
        ic_extend_times = 0;
        ic_origin_size = 0;
        reshape_dim = 0;
        reshape_type = 0;
        layout_convert_type = 0;
        cp_direct = 0;
        is_ddr_l1buf = false;
        is_ddr_wgt = false;
        remote_code = 0;
        dma_dtype = 0;

        // 地址字段
        l1_src_addr = 0;
        l1_dst_addr = 0;

        // 全局步长
        global_stride_0 = 0;
        global_stride_1 = 0;
        global_stride_2 = 0;
        global_stride_3 = 0;

        // 源 tile 维度
        src_tile_dim_0 = 0;
        src_tile_dim_1 = 0;
        src_tile_dim_2 = 0;
        src_tile_dim_3 = 0;

        // box 维度
        box_dim_0 = 0;
        box_dim_1 = 0;
        box_dim_2 = 0;
        box_dim_3 = 0;

        // 目标 tile 维度
        dst_tile_dim_0 = 0;
        dst_tile_dim_1 = 0;
        dst_tile_dim_2 = 0;
        dst_tile_dim_3 = 0;

        // 源 box 起始维度
        src_box_start_dim_0 = 0;
        src_box_start_dim_1 = 0;
        src_box_start_dim_2 = 0;
        src_box_start_dim_3 = 0;

        // 目标 box 起始维度
        dst_box_start_dim_0 = 0;
        dst_box_start_dim_1 = 0;
        dst_box_start_dim_2 = 0;
        dst_box_start_dim_3 = 0;
    }

    void set_base0(uint64_t data)
    {
        dma_op = (data >>= 0) & 0xf;
        space_block_size = (data >>= 8) & 0x7;
        d2s_size = (data >>= 4) & 0xf;
        ic_extend_times = (data >>= 4) & 0x7;
        ic_origin_size = (data >>= 8) & 0xf;
        reshape_dim = (data >>= 8) & 0x3;
        reshape_type = (data >>= 8) & 0x3;
        layout_convert_type = (data >>= 8) & 0x3;
        cp_direct = (data >>= 4) & 0x3;
        is_ddr_l1buf = (data >>= 2) & 0x1;
        is_ddr_wgt = (data >>= 1) & 0x1;
        remote_code = (data >>= 1) & 0xf;
        dma_dtype = (data >>= 4) & 0x7;
    }

    void set_base1(uint64_t data)
    {
        l1_src_addr = data & ((1UL << 48) - 1);
    }

    void set_base2(uint64_t data)
    {
        l1_dst_addr = data & ((1UL << 48) - 1);
    }

    void set_base3(uint64_t data)
    {
        global_stride_0 = data & ((1UL << 24) - 1);
        global_stride_1 = (data >>= 32) & ((1UL << 24) - 1);
    }

    void set_base4(uint64_t data)
    {
        global_stride_2 = data & ((1UL << 24) - 1);
        global_stride_3 = (data >>= 32) & ((1UL << 24) - 1);
    }

    void set_base5(uint64_t data)
    {
        src_tile_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        src_tile_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        src_tile_dim_1 = (data >>= 24) & ((1UL << 16) - 1);
        src_tile_dim_1 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base6(uint64_t data)
    {
        box_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        box_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        box_dim_1 = (data >>= 24) & ((1UL << 16) - 1);
        box_dim_1 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base7(uint64_t data)
    {
        dst_tile_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        dst_tile_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        dst_tile_dim_1 = (data >>= 24) & ((1UL << 16) - 1);
        dst_tile_dim_1 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base8(uint64_t data)
    {
        src_box_start_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        src_box_start_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        src_box_start_dim_1 = (data >>= 24) & ((1UL << 16) - 1);
        src_box_start_dim_1 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void set_base9(uint64_t data)
    {
        dst_box_start_dim_0 = (data >>= 00) & ((1UL << 16) - 1);
        dst_box_start_dim_1 = (data >>= 16) & ((1UL << 24) - 1);
        dst_box_start_dim_1 = (data >>= 24) & ((1UL << 16) - 1);
        dst_box_start_dim_1 = (data >>= 16) & ((1UL << 8) - 1);
    }

    void print_info() const
    {
        sm_log_info(
            "DMACPDesc <"
            // DMA控制字段
            "ctrl[dma_op:%u space_blk:%u d2s:%u ic_ext:%u ic_org:%u] | "
            "reshape[dim:%u type:%u layout:%u] | "
            "dir:%u ddr[l1buf:%d wgt:%d] remote:0x%x dtype:%u | "
            // 地址信息
            "addr[src:0x%012lx dst:0x%012lx] | "
            // 全局步长
            "g_stride[0:%u 1:%u 2:%u 3:%u] | "
            // 维度信息
            "src_tile[0:%u 1:%u 2:%u 3:%u] | "
            "box[0:%u 1:%u 2:%u 3:%u] | "
            "dst_tile[0:%u 1:%u 2:%u 3:%u] | "
            // 起始坐标
            "src_start[0:%u 1:%u 2:%u 3:%u] | "
            "dst_start[0:%u 1:%u 2:%u 3:%u]>",
            dma_op, space_block_size, d2s_size, ic_extend_times, ic_origin_size,
            reshape_dim, reshape_type, layout_convert_type,
            cp_direct, is_ddr_l1buf, is_ddr_wgt, remote_code, dma_dtype,
            l1_src_addr, l1_dst_addr,
            global_stride_0, global_stride_1, global_stride_2, global_stride_3,
            src_tile_dim_0, src_tile_dim_1, src_tile_dim_2, src_tile_dim_3,
            box_dim_0, box_dim_1, box_dim_2, box_dim_3,
            dst_tile_dim_0, dst_tile_dim_1, dst_tile_dim_2, dst_tile_dim_3,
            src_box_start_dim_0, src_box_start_dim_1,
            src_box_start_dim_2, src_box_start_dim_3,
            dst_box_start_dim_0, dst_box_start_dim_1,
            dst_box_start_dim_2, dst_box_start_dim_3);
    }
};

class LLMDesc
{
public:
    LLMDesc() { reset(); }

    uint64_t l1_src1_addr : 24;
    uint64_t l1_src2_addr : 24;
    uint64_t l1_param_src_addr : 24;
    uint32_t src_tile_dim_m : 16;
    uint32_t dst_tile_dim_m : 16;
    uint64_t l1_dst_addr : 24;
    uint64_t l1_param_dst_addr : 24;
    uint32_t box_dim_k : 24;
    uint32_t box_dim_m : 16;

    void reset()
    {
        l1_src1_addr = 0;
        l1_src2_addr = 0;
        l1_param_src_addr = 0;
        src_tile_dim_m = 0;
        dst_tile_dim_m = 0;
        l1_dst_addr = 0;
        l1_param_dst_addr = 0;
        box_dim_k = 0;
        box_dim_m = 0;
    }

    void set_base0(uint64_t data)
    {
        l1_src1_addr = data & ((1UL << 24) - 1);
        l1_src2_addr = (data >>= 32) & ((1UL << 24) - 1);
    }

    void set_base1(uint64_t data)
    {
        l1_param_src_addr = data & ((1UL << 24) - 1);
        src_tile_dim_m = (data >>= 32) & ((1UL << 16) - 1);
        dst_tile_dim_m = (data >>= 48) & ((1UL << 16) - 1);
    }

    void set_base2(uint64_t data)
    {
        l1_dst_addr = data & ((1UL << 24) - 1);
        l1_param_dst_addr = (data >>= 32) & ((1UL << 24) - 1);
    }

    void set_base3(uint64_t data)
    {
        box_dim_k = data & ((1UL << 24) - 1);
        box_dim_m = (data >>= 32) & ((1UL << 16) - 1);
    }

    void print_info() const
    {
        sm_log_info(
            "LLMDesc <"
            "addrs[src1:0x%06lx src2:0x%06lx param_src:0x%06lx] | "
            "tile[src_m:%u dst_m:%u] | "
            "addrs[dst:0x%06lx param_dst:0x%06lx] | "
            "box[k:%u m:%u]>",
            // 源地址组
            l1_src1_addr, l1_src2_addr, l1_param_src_addr,
            // Tile维度
            src_tile_dim_m, dst_tile_dim_m,
            // 目标地址组
            l1_dst_addr, l1_param_dst_addr,
            // Box维度
            box_dim_k, box_dim_m);
    }
};

typedef enum
{
    dsa_op_barrier,
    dsa_op_ddep_gen,
    dsa_op_ddep_use,
    dsa_op_ddep_rls,
    dsa_op_dma_ld,
    dsa_op_dma_st,
    dsa_op_dma_cp,
    dsa_op_gemm,
    dsa_op_conv,
    dsa_op_rope,
    dsa_op_layernorm,
    dsa_op_rmsnorm,
    dsa_op_softmax,
    dsa_op_vec_reduce,
} dsa_ops_t;

typedef enum
{
    dma_chn_read,
    dma_chn_write,
    dma_chn_copy,
    dma_chn_max,
} dma_chn_type_t;

typedef enum
{
    dsa_dev_dma,
    dsa_dev_llm,
    dsa_dev_tensor,
    dsa_dev_total,
} dsa_dev_type_t;

typedef unique_ptr<llm_insn_info_t> LLM_INFO;
typedef unique_ptr<LLMDesc> LLM_Desc;
typedef unique_ptr<dma_insn_info_t> DMA_INFO;
typedef unique_ptr<DMALSDesc> DMA_Ls_Desc;
typedef unique_ptr<DMACPDesc> DMA_Cp_Desc;

typedef unique_ptr<tmm_insn_info_t> TMM_INFO;
typedef unique_ptr<TSMATMULDesc> MUL_Desc;
typedef unique_ptr<PPROCDesc> PROC_Desc;
typedef unique_ptr<ELWDesc> ELW_Desc;
typedef unique_ptr<TCONVDesc> CONV_Desc;

typedef unique_ptr<ddep_gen_insn_info_t> DDEP_Gen_Info;
typedef unique_ptr<ddep_use_insn_info_t> DDEP_Use_Info;
typedef unique_ptr<ddep_rls_insn_info_t> DDEP_Rls_Info;

#endif