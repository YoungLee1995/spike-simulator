/*addedd by li yang, date 2025-07-17
 */
#include "sm_box.h"
#include "simif.h"
#include "sm_common.h"
#include "sm_log.h"

#define DIM_N SM_BOX_DIM_N
#define DIM_H SM_BOX_DIM_H
#define DIM_W SM_BOX_DIM_W
#define DIM_C SM_BOX_DIM_C

// read/write conv act/wgt : [K(N),H,W,C]
bool sm_box_t::rdwr_conv(simif_t *simif, uint8_t *bytes, bool bStore)
{
    assert(Type == sm_data_type_conv);
    if (!check_box_params())
        return false;
    //[Cslice,H,W,K,32]
    if (Layout == sm_layout_type_weight)
    {
        int cslice = box_dims[DIM_C] * Ebits / 8 / 32;
        int cbytes = cslice * 32;
        int wbytes = box_dims[DIM_W] * cbytes;
        int hbytes = box_dims[DIM_H] * wbytes;
        int kstride = tile_dims[DIM_N] * 32;
        U64 wAddr = Addr;
        for (int c = 0; c < cslice; c++)
        {
            for (uint32_t h = 0; h < box_dims[DIM_H]; h++)
            {
                for (uint32_t w = 0; w < box_dims[DIM_W]; w++)
                {
                    U64 kAddr = wAddr;
                    for (uint32_t n = 0; n < box_dims[DIM_N]; n++)
                    {
                        uint8_t *dptr = bytes + n * hbytes + h * wbytes + w * cbytes + c * 32;
                        if (bStore)
                            simif->mmio_store(kAddr, 32, dptr);
                        else
                            simif->mmio_load(kAddr, 32, dptr);
                        kAddr += 32;
                    }
                    wAddr += kstride;
                }
            }
        }
        return true;
    }

    // ON DDR:[H,W,C]
    if (Layout == sm_layout_type_org)
    {
        int cbytes = box_dims[DIM_C] * Ebits / 8;
        int cstrides = tile_dims[DIM_C] * Ebits / 8;
        int wstride = tile_dims[DIM_W] * cstrides;
        U64 wAddr = Addr;

        for (uint32_t h = 0; h < box_dims[DIM_H]; h++)
        {
            U64 hAddr = wAddr;
            for (uint32_t w = 0; w < box_dims[DIM_W]; w++)
            {
                if (bStore)
                    simif->mmio_store(wAddr, 32, bytes);
                else
                    simif->mmio_load(wAddr, 32, bytes);
                wAddr += cstrides;
                bytes += cbytes;
            }
            hAddr += wstride;
        }
        return true;
    }

    // ACT on L1: [Cslice,H,W,32]
    int cslice = box_dims[DIM_C] * Ebits / 8 / 32;
    int cbytes = cslice * 32;
    int wbytes = box_dims[DIM_W] * cbytes;
    int wstride = tile_dims[DIM_W] * 32;
    int hstride = tile_dims[DIM_H] * wstride;

    U64 cAddr = Addr;
    for (int c = 0; c < cslice; c++)
    {
        U64 hAddr = cAddr;
        for (uint32_t h = 0; h < box_dims[DIM_H]; h++)
        {
            U64 wAddr = hAddr;
            for (uint32_t w = 0; w < box_dims[DIM_W]; w++)
            {
                uint8_t *dptr = bytes + h * wbytes + w * cbytes + c * 32;
                if (bStore)
                    simif->mmio_store(wAddr, 32, dptr);
                else
                    simif->mmio_load(wAddr, 32, dptr);
                wAddr += 32;
            }
            hAddr += wstride;
        }
        cAddr += hstride;
    }
    return true;
}

bool sm_box_t::read_mat_k(simif_t *simif, int midx, uint8_t *bytes)
{
    assert(Type == sm_data_type_matrix);

    uint64_t addr;
    if (Layout == sm_layout_type_weight)
    {
        addr = (Addr + midx * tile_dims[SM_BOX_DIM_W] * Ebits) / 8;
        return simif->mmio_load(addr, box_dims[SM_BOX_DIM_W] * Ebits / 8, bytes);
    }

    uint8_t *pbuf = bytes;
    int aoff = midx * 32;
    if (Layout == sm_layout_type_activate)
    {
        //[midx.:]
        int kbytes = box_dims[SM_BOX_DIM_W] * Ebits / 8;
        for (int i = 0; i < kbytes; i += 32)
        {
            addr = Addr + i * tile_dims[SM_BOX_DIM_H] + aoff;
            simif->mmio_load(addr, 32, pbuf);
            pbuf += 32;
        }
        return true;
    }

    if (Layout == sm_layout_type_weight)
    {
        //[midx.:]
        int kbytes = box_dims[SM_BOX_DIM_H] * Ebits / 8;
        for (int i = 0; i < kbytes; i += 32)
        {
            addr = Addr + i * tile_dims[SM_BOX_DIM_W] + aoff;
            simif->mmio_load(addr, 32, pbuf);
            pbuf += 32;
        }
        return true;
    }
    return false;
}

bool sm_box_t::read_mat_m_32(simif_t *simif, int kslice, uint8_t *bytes)
{
    assert(Type == sm_data_type_matrix);

    uint64_t addr;
    uint8_t *pbuf = bytes;
    if (Layout == sm_layout_type_org)
    {
        addr = Addr + kslice;
        for (int i = 0; i < box_dims[SM_BOX_DIM_H]; i++)
        {
            simif->mmio_load(addr, 32, pbuf);
            addr += tile_dims[SM_BOX_DIM_W] * Ebits / 8;
            pbuf += 32;
        }
    }

    if (Layout == sm_layout_type_activate)
    {
        //[:,kslice:kslice+32]
        addr = Addr + kslice * tile_dims[SM_BOX_DIM_H];
        return simif->mmio_load(addr, box_dims[SM_BOX_DIM_H] * 32, pbuf);
    }

    if (Layout == sm_layout_type_weight)
    {
        //[:,kslice:kslice+32]
        addr = Addr + kslice * tile_dims[SM_BOX_DIM_W];
        return simif->mmio_load(addr, box_dims[SM_BOX_DIM_W] * 32, pbuf);
    }
    return false;
}

bool sm_box_t::read_mat_ic_32(simif_t *simif, int batchid, int cslice, int hid, uint8_t *bytes)
{
    assert(Type == sm_data_type_conv);

    uint64_t addr;
    uint8_t *pbuf = bytes;
    if (Layout == sm_layout_type_activate)
    {
        //[cslice,hid,:]
        addr = Addr + batchid * tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_C] * tile_dims[SM_BOX_DIM_W] + 32 * tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_W] * cslice + hid * 32 * tile_dims[SM_BOX_DIM_W];
        return simif->mmio_load(addr, box_dims[SM_BOX_DIM_W] * 32, pbuf);
    }

    if (Layout == sm_layout_type_weight)
    {
        //[cslice,hid:hid+32,:]
        addr = Addr + batchid * tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_C] * tile_dims[SM_BOX_DIM_W] + tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_W] * cslice + hid * tile_dims[SM_BOX_DIM_W];
        return simif->mmio_load(addr, box_dims[SM_BOX_DIM_W] * 32, pbuf);
    }
    return false;
}

bool sm_box_t::read_mat(simif_t *simif, uint8_t *bytes)
{
    assert(Type == sm_data_type_matrix);
    check_box_params();
    int M = 0, kbytes = 0;
    get_box_m_kbytes(M, kbytes);
    if (Layout == sm_layout_type_weight)
    {
        for (int i = 0; i < kbytes; i += 32)
        {
            read_mat_m_32(simif, i, bytes);
            bytes += M * 32;
        }
    }
    else
    {
        for (int i = 0; i < M; i++)
        {
            read_mat_k(simif, i, bytes);
            bytes += kbytes;
        }
    }
    return true;
}

bool sm_box_t::read_mat_T(simif_t *simif, uint8_t *bytes)
{
    assert(Type == sm_data_type_matrix);
    check_box_params();
    int M = 0, kbytes = 0;
    get_box_m_kbytes(M, kbytes);
    if (Layout == sm_layout_type_weight)
    {
        for (int i = 0; i < M; i++)
        {
            read_mat_m_32(simif, i, bytes);
            bytes += kbytes;
        }
    }
    else
    {
        sm_log_error("can't read_matT activation");
    }
    return true;
}
bool sm_box_t::read_wgt_raw(simif_t *simif, uint8_t *bytes)
{
    assert(Type == sm_data_type_conv && Layout == sm_layout_type_weight);
    if (!check_box_params())
        return false;

    int kbytes = box_dims[DIM_N] * 32;
    int kstride = tile_dims[DIM_N] * 32;
    int Steps = box_dims[DIM_N] * box_dims[DIM_W] * box_dims[DIM_C] * Ebits / 8 / 32;
    U64 kAddr = Addr;
    for (int i = 0; i < Steps; i++)
    {
        simif->mmio_load(kAddr, kbytes, bytes);
        kAddr += kstride;
        bytes += kbytes;
    }
    return true;
}

bool sm_box_t::write_mat_k(simif_t *simif, int midx, uint8_t *bytes)
{
    assert(Type == sm_data_type_matrix);

    uint64_t addr;
    if (Layout == sm_layout_type_org)
    {
        addr = (Addr + midx * tile_dims[SM_BOX_DIM_W] * Ebits) / 8;
        return simif->mmio_load(addr, box_dims[SM_BOX_DIM_W] * Ebits / 8, bytes);
    }

    uint8_t *pbuf = bytes;
    int aoff = midx * 32;
    if (Layout == sm_layout_type_activate)
    {
        //[midx.:]
        int kbytes = box_dims[SM_BOX_DIM_W] * Ebits / 8;
        for (int i = 0; i < kbytes; i += 32)
        {
            addr = Addr + i * tile_dims[SM_BOX_DIM_H] + aoff;
            simif->mmio_store(addr, 32, pbuf);
            pbuf += 32;
        }
        return true;
    }

    if (Layout == sm_layout_type_weight)
    {
        //[midx.:]
        int kbytes = box_dims[SM_BOX_DIM_H] * Ebits / 8;
        for (int i = 0; i < kbytes; i += 32)
        {
            addr = Addr + i * tile_dims[SM_BOX_DIM_W] + aoff;
            simif->mmio_store(addr, 32, pbuf);
            pbuf += 32;
        }
        return true;
    }
    return false;
}

bool sm_box_t::write_mat_m_32(simif_t *simif, int kslice, uint8_t *bytes)
{
    assert(Type == sm_data_type_matrix);

    uint64_t addr;
    uint8_t *pbuf = bytes;
    if (Layout == sm_layout_type_org)
    {
        addr = Addr + kslice;
        for (int i = 0; i < box_dims[SM_BOX_DIM_H]; i++)
        {
            simif->mmio_store(addr, 32, pbuf);
            addr += tile_dims[SM_BOX_DIM_W] * Ebits / 8;
            pbuf += 32;
        }
    }

    if (Layout == sm_layout_type_activate)
    {
        //[:,kslice:kslice+32]
        addr = Addr + kslice * tile_dims[SM_BOX_DIM_H];
        return simif->mmio_store(addr, box_dims[SM_BOX_DIM_H] * 32, pbuf);
    }

    if (Layout == sm_layout_type_weight)
    {
        //[:,kslice:kslice+32]
        addr = Addr + kslice * tile_dims[SM_BOX_DIM_W];
        return simif->mmio_store(addr, box_dims[SM_BOX_DIM_W] * 32, pbuf);
    }
    return false;
}

bool sm_box_t::write_mat_ic_32(simif_t *simif, int batchid, int cslice, int hid, uint8_t *bytes)
{
    assert(Type == sm_data_type_conv);

    uint64_t addr;
    uint8_t *pbuf = bytes;
    if (Layout == sm_layout_type_activate)
    {
        //[cslice,hid,:]
        addr = Addr + batchid * tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_C] * tile_dims[SM_BOX_DIM_W] + 32 * tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_W] * cslice + hid * 32 * tile_dims[SM_BOX_DIM_W];
        return simif->mmio_store(addr, box_dims[SM_BOX_DIM_W] * 32, pbuf);
    }

    if (Layout == sm_layout_type_weight)
    {
        //[cslice,hid:hid+32,:]
        addr = Addr + batchid * tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_C] * tile_dims[SM_BOX_DIM_W] + tile_dims[SM_BOX_DIM_H] * tile_dims[SM_BOX_DIM_W] * cslice + hid * tile_dims[SM_BOX_DIM_W];
        return simif->mmio_store(addr, box_dims[SM_BOX_DIM_W] * 32, pbuf);
    }
    return false;
}

bool sm_box_t::write_mat(simif_t *simif, uint8_t *bytes)
{
    assert(Type == sm_data_type_matrix);
    check_box_params();
    int M = 0, kbytes = 0;
    get_box_m_kbytes(M, kbytes);
    if (Layout == sm_layout_type_weight)
    {
        for (int i = 0; i < kbytes; i += 32)
        {
            write_mat_m_32(simif, i, bytes);
            bytes += M * 32;
        }
    }
    else
    {
        for (int i = 0; i < M; i++)
        {
            write_mat_k(simif, i, bytes);
            bytes += kbytes;
        }
    }
    return true;
}
bool sm_box_t::write_wgt_raw(simif_t *simif, uint8_t *bytes)
{
    assert(Type == sm_data_type_conv && Layout == sm_layout_type_weight);
    if (!check_box_params())
        return false;

    int kbytes = box_dims[DIM_N] * 32;
    int kstride = tile_dims[DIM_N] * 32;
    int Steps = box_dims[DIM_N] * box_dims[DIM_W] * box_dims[DIM_C] * Ebits / 8 / 32;
    U64 kAddr = Addr;
    for (int i = 0; i < Steps; i++)
    {
        simif->mmio_store(kAddr, kbytes, bytes);
        kAddr += kstride;
        bytes += kbytes;
    }
    return true;
}

uint16_t pad_box_m(uint16_t m);
bool sm_get_llm_box(llm_insn_info_t *info, LLMDesc *desc, sm_box_t &sbox, sm_box_t &dbox, uint32_t id, uint8_t src_mode, uint8_t dst_mode)
{
    sbox.Type = info->data_type == sm_data_type_matrix ? sm_data_type_matrix : sm_data_type_conv;
    sbox.Layout = info->layout_type == SM_LAYOUT_TYPE_MAT_HW ? sm_layout_type_org : sm_layout_type_activate;
    sbox.Ebits = info->idata_bytes() * 8;
    if (src_mode == 0)
    {
        sbox.Addr = SM_L1_NPU_BASE(id) + desc->l1_src1_addr;
    }
    else if (src_mode == 1)
    {
        sbox.Addr = SM_L1_NPU_BASE(id) + desc->l1_src2_addr;
    }
    else
    {
        sbox.Addr = SM_L1_NPU_BASE(id) + desc->l1_param_src_addr;
    }
    sbox.box_dims[0] = 1;
    sbox.box_dims[1] = desc->box_dim_k;
    sbox.box_dims[2] = desc->box_dim_m;
    sbox.box_dims[3] = 1;
    sbox.tile_dims[0] = 1;
    sbox.tile_dims[1] = desc->box_dim_k;
    sbox.tile_dims[2] = desc->dst_tile_dim_m;
    sbox.tile_dims[3] = 1;
    sbox.box_offset[0] = 0;
    sbox.box_offset[1] = 0;
    sbox.box_offset[2] = 0;
    sbox.box_offset[3] = 0;

    dbox.Type = sbox.Type;
    dbox.Layout = sbox.Layout;
    dbox.Ebits = info->odata_bytes() * 8;
    if (dst_mode == 0)
    {
        dbox.Addr = SM_L1_NPU_BASE(id) + desc->l1_dst_addr;
    }
    else if (dst_mode == 1)
    {
        dbox.Addr = SM_L1_NPU_BASE(id) + desc->l1_param_dst_addr;
    }
    else
    {
        dbox.Addr = SM_L1_NPU_BASE(id) + desc->l1_param_dst_addr;
    }
    dbox.box_dims[0] = 1;
    dbox.box_dims[1] = desc->box_dim_k;
    dbox.box_dims[2] = desc->box_dim_m;
    dbox.box_dims[3] = 1;
    dbox.tile_dims[0] = 1;
    dbox.tile_dims[1] = desc->box_dim_k;
    dbox.tile_dims[2] = desc->dst_tile_dim_m;
    dbox.tile_dims[3] = 1;
    dbox.box_offset[0] = 0;
    dbox.box_offset[1] = 0;
    dbox.box_offset[2] = 0;
    dbox.box_offset[3] = 0;

    return true;
}

bool sm_get_dma_ls_box(dma_insn_info_t *info, DMA_LS_Desc *desc, sm_box_t &sbox, sm_box_t &dbox, uint32_t id, uint8_t src_mode, uint8_t dst_mode)
{
    sbox.Type = info->mat_type == 0 ? sm_data_type_matrix : sm_data_type_conv;
    sbox.Layout = info->data_type == 0 ? sm_layout_type_weight : (info->data_type == 1 && info->op == 1 ? sm_layout_type_activate : sm_layout_type_org);
    sbox.Ebits = info->get_bits_from_fmt();
    if (info->op == 0)
    {
        sbox.Addr = SM_DDR1_BASE + desc->ddr_global_base_addr;
    }
    else
    {
        sbox.Addr = SM_L1_NPU_BASE(id) + desc->l1buf_tile_base_addr;
    }
    sbox.box_dims[0] = desc->box_dim_0;
    sbox.box_dims[1] = desc->box_dim_1;
    sbox.box_dims[2] = desc->box_dim_2;
    sbox.box_dims[3] = desc->box_dim_3;
    int elem_bytes = info->get_bits_from_fmt() / 8;
    uint32_t stride_elem0 = ceil(static_cast<double>(desc->global_stride_0) / elem_bytes);
    uint32_t stride_elem1 = ceil(static_cast<double>(desc->global_stride_1) / elem_bytes);
    uint32_t stride_elem2 = ceil(static_cast<double>(desc->global_stride_2) / elem_bytes);
    uint32_t stride_elem3 = ceil(static_cast<double>(desc->global_stride_3) / elem_bytes);
    sbox.tile_dims[0] = info->op == 0 ? stride_elem0 : desc->l1_tile_dim_0;
    sbox.tile_dims[1] = info->op == 0 ? stride_elem1 : desc->l1_tile_dim_1;
    sbox.tile_dims[2] = info->op == 0 ? stride_elem2 : desc->l1_tile_dim_2;
    sbox.tile_dims[3] = info->op == 0 ? stride_elem3 : desc->l1_tile_dim_3;
    sbox.box_offset[0] = info->op == 0 ? desc->tensor_coords_0 : desc->l1_box_start_dim_0;
    sbox.box_offset[1] = info->op == 0 ? desc->tensor_coords_1 : desc->l1_box_start_dim_1;
    sbox.box_offset[2] = info->op == 0 ? desc->tensor_coords_2 : desc->l1_box_start_dim_2;
    sbox.box_offset[3] = info->op == 0 ? desc->tensor_coords_3 : desc->l1_box_start_dim_3;

    dbox.Type = sbox.Type;
    dbox.Addr = info->op == 0 ? (SM_L1_NPU_BASE(id) + desc->l1buf_tile_base_addr) : (SM_DDR1_BASE + desc->ddr_global_base_addr);
    dbox.box_dims[0] = desc->box_dim_0;
    dbox.box_dims[1] = desc->box_dim_1;
    dbox.box_dims[2] = desc->box_dim_2;
    dbox.box_dims[3] = desc->box_dim_3;
    sbox.tile_dims[0] = info->op == 0 ? desc->l1_tile_dim_0 : stride_elem0;
    dbox.tile_dims[1] = info->op == 0 ? desc->l1_tile_dim_1 : stride_elem1;
    dbox.tile_dims[2] = info->op == 0 ? desc->l1_tile_dim_2 : stride_elem2;
    dbox.tile_dims[3] = info->op == 0 ? desc->l1_tile_dim_3 : stride_elem3;
    dbox.box_offset[0] = info->op == 0 ? desc->l1_box_start_dim_0 : desc->tensor_coords_0;
    dbox.box_offset[1] = info->op == 0 ? desc->l1_box_start_dim_1 : desc->tensor_coords_1;
    dbox.box_offset[2] = info->op == 0 ? desc->l1_box_start_dim_2 : desc->tensor_coords_2;
    dbox.box_offset[3] = info->op == 0 ? desc->l1_box_start_dim_3 : desc->tensor_coords_3;

    return true;
}

bool sm_get_dma_cp_box(dma_insn_info_t *info, DMA_CP_Desc *desc, sm_box_t &sboc, sm_box_t &dbox, uint32_t id, uint8_t src_mode, uint8_t dst_mode)
{
    sboc.Type = info->mat_type == 0 ? sm_data_type_matrix : sm_data_type_conv;
    sboc.Layout = info->data_type == 0 ? sm_layout_type_weight : (info->data_type == 1 ? sm_layout_type_activate : sm_layout_type_param);
    sboc.Ebits = info->get_bits_from_fmt();
    if (info->op == 0)
    bool  remote_l1=(desc->cp_direct==0|| desc->cp_direct==1)&&desc->is_ddr_l1buf==0&&(desc->remote_code_id!=id);
    if(desc->is_ddr_l1buf){
        sboc.Addr = SM_DDR1_BASE + desc->cp_te;
    }
    sboc.box_dims[0] = desc->box_dim_0;
    sboc.box_dims[1] = desc->box_dim_1;
    sboc.box_dims[2] = desc->box_dim_2;
    sboc.box_dims[3] = desc->box_dim_3;
    int elem_bytes = info->get_bits_from_fmt() / 8;
    uint32_t stride_elem0 = ceil(static_cast<double>(desc->global_stride_0) / elem_bytes);
    uint32_t stride_elem1 = ceil(static_cast<double>(desc->global_stride_1) / elem_bytes);
    uint32_t stride_elem2 = ceil(static_cast<double>(desc->global_stride_2) / elem_bytes);
    uint32_t stride_elem3 = ceil(static_cast<double>(desc->global_stride_3) / elem_bytes);
    sboc.tile_dims[0] = info->op == 0 ? stride_elem0 : desc->l1_tile_dim_0;
    sboc.tile_dims[1] = info->op == 0 ? stride_elem1 : desc->l1_tile_dim_1;
    sboc.tile_dims[2] = info->op == 0 ? stride_elem2 : desc->l1_tile_dim_2;
    sboc.tile_dims[3] = info->op == 0 ? stride_elem3 : desc->l1_tile_dim_3;
    sboc.box_offset[0] = info->op == 0 ? desc->tensor_coords_0 : desc->l1_box_start_dim_0;
    sboc.box_offset[1] = info->op == 0 ? desc->tensor_coords_1 : desc->l1_box_start_dim_1;
}

bool sm_get_tmm_srcbox(tmm_insn_info_t *info, TSMATMULDesc *desc, sm_box_t &dbox, sm_layout_type_t type, uint32_t id);
bool sm_get_tmm_dstbox(tmm_insn_info_t *info, TSMATMULDesc *desc, sm_box_t &dbox, uint32_t id);
bool sm_get_elw_box(tmm_insn_info_t *info, ELWDesc *desc, sm_box_t &dbox, uint32_t id);
bool sm_get_tconv_srcbox(tmm_insn_info_t *info, TCONVDesc *desc, sm_box_t &dbox, sm_layout_type_t type, uint32_t id);
bool sm_get_tconv_dstbox(tmm_insn_info_t *info, TCONVDesc *desc, sm_box_t &dbox, uint32_t id);