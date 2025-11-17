#ifndef _sm_box_h_
#define _sm_box_h_
#include "sm_common.h"
#include "sm_log.h"
#include "sm_regs.h"
#include <cmath>
#include <cstdint>
#include <memory>

class simif_t; // 前向声明
class llm_insn_info_t;
class LLMDesc;

#define SM_BOX_DIM_N 3
#define SM_BOX_DIM_H 2
#define SM_BOX_DIM_W 1
#define SM_BOX_DIM_C 0

typedef enum
{
    sm_data_type_matrix = 0,
    sm_data_type_conv = 1
} sm_data_type_t;

typedef enum
{
    sm_layout_type_org,
    sm_layout_type_activate,
    sm_layout_type_weight,
    sm_layout_type_param,
    sm_wgtscale_pergroup,
    sm_wgtscale_perblock,
    sm_actscale_pergroup,
    sm_elw

} sm_layout_type_t;

struct sm_box_t
{
    sm_data_type_t Type;
    sm_layout_type_t Layout;
    int Ebits;
    int Dims;

    uint64_t Addr;
    uint32_t box_dims[4];
    uint32_t tile_dims[4];
    uint32_t box_offset[4];

    bool read_mat_k(simif_t *simif, int midx, uint8_t *bytes); // activatr: row, weight: col, [H,W]: row
    bool read_mat_m_32(simif_t *simif, int kslice, uint8_t *bytes);
    bool read_mat_ic_32(simif_t *simif, int batchid, int cslice, int hid, uint8_t *bytes);
    bool read_mat(simif_t *simif, uint8_t *bytes);
    bool read_mat_T(simif_t *simif, uint8_t *bytes);
    bool read_wgt_raw(simif_t *simif, uint8_t *bytes);
    bool read_conv(simif_t *simif, uint8_t *bytes) { return rdwr_conv(simif, bytes, false); }
    bool read_mat_for_matmul(simif_t *simif, uint8_t *bytes);
    bool mat_layout_convert(uint8_t *src, uint8_t *dst);

    bool write_mat_k(simif_t *simif, int midx, uint8_t *bytes);
    bool write_mat_m_32(simif_t *simif, int kslice, uint8_t *bytes);
    bool write_mat_ic_32(simif_t *simif, int batchid, int cslice, int hid, uint8_t *bytes);
    bool write_mat(simif_t *simif, uint8_t *bytes);
    bool write_mat_T(simif_t *simif, uint8_t *bytes);
    bool write_wgt_raw(simif_t *simif, uint8_t *bytes);
    bool write_conv(simif_t *simif, uint8_t *bytes) { return rdwr_conv(simif, bytes, true); }

    bool in_addr() { return (SM_IS_DDR0(Addr) || SM_IS_DDR1(Addr)); }
    bool is_l1buf(uint32_t id = 0) { return SM_IS_L1BUF(id, Addr); }

    bool rdwr_conv(simif_t *simif, uint8_t *bytes, bool bStore);

    int get_box_bytes()
    {
        if (Type == sm_data_type_matrix)
            return (box_dims[1] * box_dims[2] * Ebits) / 8;
        if (Layout == sm_layout_type_weight)
            return (box_dims[1] * box_dims[2] * box_dims[3] * box_dims[4] * Ebits) / 8;
        return (box_dims[1] * box_dims[2] * box_dims[0] * Ebits) / 8;
    }

    int get_box_m_kbytes(int &m, int &kbytes)
    {
        if (Layout == sm_layout_type_weight)
        {
            m = box_dims[1];
            kbytes = box_dims[2] * Ebits / 8;
        }
        else
        {
            m = box_dims[2];
            kbytes = box_dims[1] * Ebits / 8;
        }
        return 0;
    }

    int get_tile_m_kbytes(int &m, int &kbytes)
    {
        if (Layout == sm_layout_type_weight)
        {
            m = box_dims[1];
            kbytes = box_dims[2] * Ebits / 8;
        }
        else
        {
            m = box_dims[2];
            kbytes = box_dims[1] * Ebits / 8;
        }
        return 0;
    }

    bool check_box_params()
    {
        if (Type == sm_data_type_matrix)
        {
            int mbox = 0, mtile = 0;
            int kbox = 0, ktile = 0;
            get_box_m_kbytes(mbox, kbox);
            get_tile_m_kbytes(mtile, ktile);
            if (!mbox || !kbox)
            {
                sm_log_error("mat: invallid H/W(%d,%d)", box_dims[1], box_dims[2]);
                return false;
            }
            if (kbox % 32 != 0)
            {
                sm_log_error("invalid kbytes(%d)", kbox);
                return false;
            }
            if (ktile < kbox || kbox % 32 != 0)
            {
                sm_log_error("mat: invallid tile kbytes(%d)", kbox);
                return false;
            }
            // org only on ddr, can use any m
            if (Layout == sm_layout_type_org)
                return true;
            if (mbox != 1 && mbox % 16 != 0)
            {
                sm_log_error("invalid M(%d):must 1 or 16x", mbox);
                return false;
            }
            if (mbox == 1)
            {
                if (mtile != 1)
                {
                    sm_log_error("invalid Mtile(%d):must 1 ", mtile);
                    return false;
                }
            }
            else if (mtile < mbox || mtile % 16 != 0)
            {
                sm_log_error("invalid Mtile(%d)", mtile);
                return false;
            }
            return true;
        }
        // conv act: can tile c
        int cbytes = box_dims[0] * Ebits / 8;
        int ctiles = tile_dims[1] * Ebits / 8;
        if (!box_dims[0] || !tile_dims[0] || ctiles < cbytes || cbytes % 32 != 0 || ctiles % 32 != 0)
        {
            sm_log_error("Conv:invalid C,Ctile(%d,%d) --must 32 byte aligned", box_dims[0], tile_dims[0]);
            return false;
        }
        if (Layout == sm_layout_type_weight)
        {
            if (cbytes != ctiles)
            {
                sm_log_error("Conv wgt: can't tile C");
                return false;
            }
            if (!box_dims[1] || !tile_dims[2] || !box_dims[3] || !tile_dims[3] || box_dims[3] > tile_dims[3])
            {
                sm_log_error("Conv wgt: invalid [K,H,W],(%d,%d,%d) Tile_K(%d) ", box_dims[1], box_dims[2], box_dims[3]);
                return false;
            }
            if (tile_dims[3] > 1 && (box_dims[3] % 16 != 0 || tile_dims[3] % 16 != 0))
            {
                sm_log_error("Conv wgt: L1_k 1 or 16x,(%d,%d) Tile_K(%d) ", box_dims[3], tile_dims[3]);
                return false;
            }
            return true;
        }

        // Conv Act(L1): [CSlice,H,W,32], Tile:HxW 16x
        // Conv Org(ddr): [H,W,C]
        if (!box_dims[1] || !tile_dims[1] || !box_dims[2] || !tile_dims[2] || box_dims[1] > tile_dims[1] || box_dims[2] > tile_dims[2])
        {
            sm_log_error("Conv act: invalid [H,W](%d,%d),Tile,(%d,%d) ", box_dims[1], tile_dims[1], box_dims[2], tile_dims[2]);
            return false;
        }
        if (Layout == sm_layout_type_activate)
        {
            if ((tile_dims[1] * tile_dims[2] % 16 != 0))
            {
                sm_log_error("Conv act: Tile [H,W](%d,%d) must be 16x ", tile_dims[1], tile_dims[2]);
                return false;
            }
        }
        return true;
    }
};

uint16_t pad_box_m(uint16_t m);
bool sm_get_llm_box(llm_insn_info_t *info, LLMDesc *desc, sm_box_t &sboc, sm_box_t &dbox, uint32_t id = 0, uint8_t src_mode = 0, uint8_t dst_mode = 0);
bool sm_get_dma_ls_box(dma_insn_info_t *info, DMA_LS_Desc *desc, sm_box_t &sboc, sm_box_t &dbox, uint32_t id = 0, uint8_t src_mode = 0, uint8_t dst_mode = 0);
bool sm_get_dma_cp_box(dma_insn_info_t *info, DMA_CP_Desc *desc, sm_box_t &sboc, sm_box_t &dbox, uint32_t id = 0, uint8_t src_mode = 0, uint8_t dst_mode = 0);
bool sm_get_tmm_srcbox(tmm_insn_info_t *info, TSMATMULDesc *desc, sm_box_t &dbox, sm_layout_type_t type, uint32_t id = 0);
bool sm_get_tmm_dstbox(tmm_insn_info_t *info, TSMATMULDesc *desc, sm_box_t &dbox, uint32_t id = 0);
bool sm_get_elw_box(tmm_insn_info_t *info, ELWDesc *desc, sm_box_t &dbox, uint32_t id = 0);
bool sm_get_tconv_srcbox(tmm_insn_info_t *info, TCONVDesc *desc, sm_box_t &dbox, sm_layout_type_t type, uint32_t id = 0);
bool sm_get_tconv_dstbox(tmm_insn_info_t *info, TCONVDesc *desc, sm_box_t &dbox, uint32_t id = 0);
bool sm_get_tmm_scalebox(tmm_insn_info_t *info, TSMATMULDesc *desc, sm_box_t &box, sm_layout_type_t type, uint32_t id = 0);
bool sm_get_elw_srcbox(ELWDesc *desc, sm_box_t &box, uint32_t id = 0);

#endif