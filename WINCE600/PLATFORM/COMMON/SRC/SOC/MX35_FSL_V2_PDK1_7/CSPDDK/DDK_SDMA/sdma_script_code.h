
/*
 * Copyright 2004 Freescale Semiconductor, Inc. All Rights Reserved.  */
 
/*!
 * @file sdma_script_code.h
 * @brief This file contains functions of SDMA scripts code initialization
 * 
 * The file was generated automatically. Based on sdma scripts library.
 * 
 * @ingroup SDMA
 */
 
#ifndef SDMA_SCRIPT_CODE_H
#define SDMA_SCRIPT_CODE_H

/************************************************************************************

            SDMA RELEASE LABEL:     "SDMA_RINGO.03.00.00" (For TO2)

*************************************************************************************/

/*!
* SDMA ROM scripts start addresses and sizes
*/

#define start_ADDR  0
#define start_SIZE  24
 
#define core_ADDR   80
#define core_SIZE   233
 
#define common_ADDR 313
#define common_SIZE 416
 
#define ap_2_ap_ADDR    729
#define ap_2_ap_SIZE    41
 
#define app_2_mcu_ADDR  770
#define app_2_mcu_SIZE  64
 
#define mcu_2_app_ADDR  834
#define mcu_2_app_SIZE  70
 
#define uart_2_mcu_ADDR 904
#define uart_2_mcu_SIZE 75
 
#define shp_2_mcu_ADDR  979
#define shp_2_mcu_SIZE  69
 
#define mcu_2_shp_ADDR  1048
#define mcu_2_shp_SIZE  72
 
#define per_2_shp_ADDR  1120
#define per_2_shp_SIZE  78
 
#define shp_2_per_ADDR  1198
#define shp_2_per_SIZE  72
 
#define uartsh_2_mcu_ADDR   1270
#define uartsh_2_mcu_SIZE   69
 
#define mcu_2_ata_ADDR  1339
#define mcu_2_ata_SIZE  90
 
#define ata_2_mcu_ADDR  1429
#define ata_2_mcu_SIZE  102
 
#define app_2_per_ADDR  1531
#define app_2_per_SIZE  66
 
#define per_2_app_ADDR  1597
#define per_2_app_SIZE  74
 
#define loop_DMAs_routines_ADDR 1671
#define loop_DMAs_routines_SIZE 240
 
#define test_ADDR   1911
#define test_SIZE   63
 
#define signature_ADDR  1022
#define signature_SIZE  1
 
/*!
* SDMA RAM scripts start addresses and sizes
*/

#define asrc__mcu_ADDR  6144
#define asrc__mcu_SIZE  116
 
#define ext_mem__ipu_ram_ADDR   6260
#define ext_mem__ipu_ram_SIZE   123
 
#define mcu_2_spdif_ADDR    6383
#define mcu_2_spdif_SIZE    103
 
#define p_2_p_ADDR  6486
#define p_2_p_SIZE  260
 
#define spdif_2_mcu_ADDR    6746
#define spdif_2_mcu_SIZE    47
 
#define uart_2_per_ADDR 6793
#define uart_2_per_SIZE 73
 
#define uartsh_2_per_ADDR   6866
#define uartsh_2_per_SIZE   67
 
/*!
* SDMA RAM image start address and size
*/

#define RAM_CODE_START_ADDR     6144
#define RAM_CODE_SIZE           789

/*!
* Buffer that holds the SDMA RAM image
*/

static const short sdma_code[] =
{
0xc230, 0xc23a, 0x56f3, 0x57db, 0x047a, 0x7d07, 0x072f, 0x076e,
0x7d02, 0x6ec7, 0x9813, 0x6ed7, 0x9813, 0x074f, 0x076e, 0x7d02,
0x6e01, 0x9813, 0x6e05, 0x5ce3, 0x048f, 0x0410, 0x3c0f, 0x5c93,
0x0e03, 0x0611, 0x1eff, 0x06bf, 0x06d5, 0x7d01, 0x068d, 0x05a6,
0x5deb, 0x55fb, 0x008e, 0x076a, 0x7d02, 0x076b, 0x7c04, 0x06d4,
0x7d01, 0x008c, 0x04a0, 0x06a0, 0x076f, 0x7d0c, 0x076e, 0x7d05,
0x7802, 0x62c8, 0x5a05, 0x7c2b, 0x9847, 0x7802, 0x5205, 0x6ac8,
0x7c26, 0x9847, 0x076e, 0x7d05, 0x7802, 0x620b, 0x5a05, 0x7c21,
0x9847, 0x7802, 0x5205, 0x6a0b, 0x7c1c, 0x6a28, 0x7f1a, 0x076a,
0x7d02, 0x076b, 0x7c0a, 0x4c00, 0x7c08, 0x076a, 0x7d03, 0x5a05,
0x7f11, 0x9854, 0x5205, 0x7e0e, 0x5493, 0x4e00, 0x7ccb, 0x0000,
0x54e3, 0x55eb, 0x4d00, 0x7d0a, 0xc251, 0x57db, 0x9814, 0x68cc,
0x9862, 0x680c, 0x009e, 0x0007, 0x54e3, 0xd868, 0xc261, 0x9802,
0x55eb, 0x009d, 0x058c, 0x0aff, 0x0211, 0x1aff, 0x05ba, 0x05a0,
0x04b2, 0x04ad, 0x0454, 0x0006, 0x0e70, 0x0611, 0x5616, 0xc18a,
0x7d2a, 0x5ade, 0x008e, 0xc19c, 0x7c26, 0x5be0, 0x5ef0, 0x5ce8,
0x0688, 0x08ff, 0x0011, 0x28ff, 0x00bc, 0x53f6, 0x05df, 0x7d0b,
0x6dc5, 0x03df, 0x7d03, 0x6bd5, 0xd8c3, 0x989f, 0x6b05, 0xc6e7,
0x7e27, 0x7f29, 0x989f, 0x6d01, 0x03df, 0x7d05, 0x6bd5, 0xc711,
0x7e18, 0x7f1a, 0x989f, 0x6b05, 0xc687, 0x7e07, 0x7f06, 0x52de,
0x53e6, 0xc1a8, 0x7dd7, 0x0200, 0x9877, 0x0007, 0x6004, 0x680c,
0x53f6, 0x028e, 0x00a3, 0xc2ad, 0x048b, 0x0498, 0x0454, 0x068a,
0x989f, 0x0207, 0x680c, 0x6ddf, 0x0107, 0x68ff, 0x60d0, 0x98a8,
0x0207, 0x68ff, 0x6d28, 0x0107, 0x6004, 0x680c, 0x98a8, 0x0007,
0x68ff, 0x60d0, 0x98a8, 0x0288, 0x03a5, 0x3b03, 0x3d03, 0x4d00,
0x7d0a, 0x0804, 0x00a5, 0x00da, 0x7d1a, 0x02a0, 0x7b01, 0x65d8,
0x7eee, 0x65ff, 0x7eec, 0x0804, 0x02d0, 0x7d11, 0x4b00, 0x7c0f,
0x008a, 0x3003, 0x6dcf, 0x6bdf, 0x0015, 0x0015, 0x7b02, 0x65d8,
0x0000, 0x7edd, 0x63ff, 0x7edb, 0x3a03, 0x6dcd, 0x6bdd, 0x008a,
0x7b02, 0x65d8, 0x0000, 0x7ed3, 0x65ff, 0x7ed1, 0x0006, 0xc230,
0xc23a, 0x57db, 0x52f3, 0x047a, 0x7d06, 0x0479, 0x7c02, 0x6ac6,
0x98fc, 0x6ac7, 0x98fc, 0x6a01, 0x008f, 0x00d5, 0x7d01, 0x008d,
0x05a0, 0x5deb, 0x56fb, 0x0478, 0x7d4e, 0x0479, 0x7c1f, 0x0015,
0x0388, 0x047a, 0x7d03, 0x62c8, 0x7e39, 0x9910, 0x620a, 0x7e38,
0x0808, 0x7801, 0x0217, 0x5a06, 0x7f34, 0x2301, 0x047a, 0x7d03,
0x62c8, 0x7e2c, 0x991d, 0x620a, 0x7e2b, 0x0808, 0x7801, 0x0217,
0x5a26, 0x7f27, 0x2301, 0x4b00, 0x7ce4, 0x993c, 0x0015, 0x0015,
0x0015, 0x047a, 0x7d09, 0x7806, 0x0b00, 0x62c8, 0x5a06, 0x0b01,
0x62c8, 0x5a26, 0x7c13, 0x993c, 0x7806, 0x0b00, 0x620b, 0x5a06,
0x0b01, 0x620b, 0x5a26, 0x7c0c, 0x0b70, 0x0311, 0x5313, 0x0000,
0x55eb, 0x4d00, 0x7d11, 0xc251, 0x57db, 0x98fc, 0x68cc, 0x9949,
0x680c, 0x0007, 0x0479, 0x7c02, 0x008b, 0x9950, 0x0017, 0x00a3,
0x0b70, 0x0311, 0x5313, 0xc26a, 0xc261, 0x98f1, 0x0b70, 0x0311,
0x5313, 0x076c, 0x7c01, 0xc230, 0x5efb, 0x068a, 0x076b, 0x7c01,
0xc230, 0x5ef3, 0x59db, 0x58d3, 0x018f, 0x0110, 0x390f, 0x008b,
0xc18a, 0x7d2b, 0x5ac0, 0x5bc8, 0xc19c, 0x7c27, 0x0388, 0x0689,
0x5ce3, 0x0dff, 0x0511, 0x1dff, 0x05bc, 0x073e, 0x4d00, 0x7d18,
0x0870, 0x0011, 0x077e, 0x7d09, 0x077d, 0x7d02, 0x5228, 0x9981,
0x52f8, 0x54db, 0x02bc, 0x02cc, 0x7c09, 0x077c, 0x7d02, 0x5228,
0x998a, 0x52f8, 0x54d3, 0x02bc, 0x02cc, 0x7d09, 0x0400, 0x9978,
0x008b, 0x52c0, 0x53c8, 0xc1a8, 0x7dd6, 0x0200, 0x9968, 0x08ff,
0x00bf, 0x077f, 0x7d1b, 0x0488, 0x00d5, 0x7d01, 0x008d, 0x05a0,
0x5deb, 0x028f, 0x32ff, 0x0210, 0x32ff, 0x0210, 0x0212, 0x0217,
0x0217, 0x32ff, 0x0212, 0x05da, 0x7c02, 0x073e, 0x99b9, 0x02a4,
0x02dd, 0x7d02, 0x073e, 0x99b9, 0x075e, 0x99b9, 0x55eb, 0x0598,
0x5deb, 0x52f3, 0x54fb, 0x076a, 0x7d26, 0x076c, 0x7d01, 0x99f6,
0x076b, 0x7c57, 0x0769, 0x7d04, 0x0768, 0x7d02, 0x0e01, 0x99d0,
0x5893, 0x00d6, 0x7d01, 0x008e, 0x5593, 0x05a0, 0x5d93, 0x06a0,
0x7802, 0x5502, 0x5d04, 0x7c1d, 0x4e00, 0x7c08, 0x0769, 0x7d03,
0x5502, 0x7e17, 0x99dd, 0x5d04, 0x7f14, 0x0689, 0x5093, 0x4800,
0x7d01, 0x99c8, 0x9a41, 0x0015, 0x7806, 0x5502, 0x5d04, 0x074d,
0x5502, 0x5d24, 0x072d, 0x7c01, 0x9a41, 0x0017, 0x076d, 0x7c01,
0x2001, 0x5593, 0x009d, 0x0007, 0xda48, 0x9990, 0x6cd3, 0x0769,
0x7d04, 0x0768, 0x7d02, 0x0e01, 0x9a05, 0x5893, 0x00d6, 0x7d01,
0x008e, 0x5593, 0x05a0, 0x5d93, 0x06a0, 0x7802, 0x5502, 0x6dc8,
0x7c0f, 0x4e00, 0x7c08, 0x0769, 0x7d03, 0x5502, 0x7e09, 0x9a12,
0x6dc8, 0x7f06, 0x0689, 0x5093, 0x4800, 0x7d01, 0x99fd, 0x9a41,
0x9a3b, 0x6ac3, 0x0769, 0x7d04, 0x0768, 0x7d02, 0x0e01, 0x9a28,
0x5893, 0x00d6, 0x7d01, 0x008e, 0x5593, 0x05a0, 0x5d93, 0x06a0,
0x7802, 0x65c8, 0x5d04, 0x7c0f, 0x4e00, 0x7c08, 0x0769, 0x7d03,
0x65c8, 0x7e09, 0x9a35, 0x5d04, 0x7f06, 0x0689, 0x5093, 0x4800,
0x7d01, 0x9a20, 0x9a41, 0x5593, 0x009d, 0x0007, 0x6cff, 0xda48,
0x9990, 0x0000, 0x54e3, 0x55eb, 0x4d00, 0x7c01, 0x9990, 0x9978,
0x54e3, 0x55eb, 0x0aff, 0x0211, 0x1aff, 0x077f, 0x7c02, 0x05a0,
0x9a55, 0x009d, 0x058c, 0x05ba, 0x05a0, 0x0210, 0x04ba, 0x04ad,
0x0454, 0x0006, 0xc230, 0xc23a, 0x57db, 0x52f3, 0x047a, 0x7d02,
0x6ad7, 0x9a63, 0x6a05, 0x008f, 0x00d5, 0x7d01, 0x008d, 0x05a0,
0x56fb, 0x0015, 0x0015, 0x0015, 0x047a, 0x7d07, 0x7804, 0x5206,
0x6ac8, 0x5226, 0x6ac8, 0x7c0f, 0x9a7d, 0x7804, 0x5206, 0x6a0b,
0x5226, 0x6a0b, 0x7c0a, 0x6a28, 0x7f08, 0x0000, 0x4d00, 0x7d07,
0xc251, 0x57db, 0x9a63, 0xc2ca, 0x9a86, 0xc2ce, 0x0454, 0xc261,
0x9a5c, 0xc23a, 0x57db, 0x52f3, 0x6ad5, 0x56fb, 0x028e, 0x1a94,
0x6ac3, 0x62c8, 0x0269, 0x7d1e, 0x1e94, 0x6ee3, 0x62d0, 0x5aeb,
0x62c8, 0x0248, 0x6ed3, 0x6ac8, 0x2694, 0x52eb, 0x6ad5, 0x6ee3,
0x62c8, 0x026e, 0x7d27, 0x6ac8, 0x7f23, 0x2501, 0x4d00, 0x7d26,
0x028e, 0x1a98, 0x6ac3, 0x62c8, 0x6ec3, 0x0260, 0x7df1, 0x62d0,
0xc2d1, 0x9ace, 0x6ee3, 0x008f, 0x2001, 0x00d5, 0x7d01, 0x008d,
0x05a0, 0x62c8, 0x026e, 0x7d0e, 0x6ac8, 0x7f0a, 0x2001, 0x7cf9,
0x6add, 0x7f06, 0x0000, 0x4d00, 0x7d09, 0xc251, 0x57db, 0x9a8d,
0x0007, 0x6aff, 0x62d0, 0xc2d1, 0x0458, 0x0454, 0x6add, 0x7ff8,
0xc261, 0x9a8a, 0xc230, 0xc23a, 0x57db, 0x52f3, 0x6ad5, 0x56fb,
0x028e, 0x1a94, 0x5202, 0x0269, 0x7d17, 0x1e94, 0x5206, 0x0248,
0x5a06, 0x2694, 0x5206, 0x026e, 0x7d26, 0x6ac8, 0x7f22, 0x2501,
0x4d00, 0x7d27, 0x028e, 0x1a98, 0x5202, 0x0260, 0x7df3, 0x6add,
0x7f18, 0x62d0, 0xc2d1, 0x9b11, 0x008f, 0x2001, 0x00d5, 0x7d01,
0x008d, 0x05a0, 0x5206, 0x026e, 0x7d0e, 0x6ac8, 0x7f0a, 0x2001,
0x7cf9, 0x6add, 0x7f06, 0x0000, 0x4d00, 0x7d0b, 0xc251, 0x57db,
0x9ad7, 0x0007, 0x6aff, 0x6add, 0x7ffc, 0x62d0, 0xc2d1, 0x0458,
0x0454, 0x6add, 0x7ff6, 0xc261, 0x9ad4
};


/************************************************************************************

                        SDMA RELEASE LABEL:         "SS15_RINGO"  (For TO1)

*************************************************************************************/

/*!
* SDMA ROM scripts start addresses and sizes
*/

#define start_ADDR_TO1                  0
#define start_SIZE_TO1                  22
 
#define core_ADDR_TO1                   80
#define core_SIZE_TO1                   232
 
#define common_ADDR_TO1                 312
#define common_SIZE_TO1                 330
 
#define ap_2_ap_ADDR_TO1                642
#define ap_2_ap_SIZE_TO1                41
 
#define app_2_mcu_ADDR_TO1              683
#define app_2_mcu_SIZE_TO1              64
 
#define mcu_2_app_ADDR_TO1              747
#define mcu_2_app_SIZE_TO1              70
 
#define uart_2_mcu_ADDR_TO1             817
#define uart_2_mcu_SIZE_TO1             75
 
#define shp_2_mcu_ADDR_TO1              892
#define shp_2_mcu_SIZE_TO1              69
 
#define mcu_2_shp_ADDR_TO1              961
#define mcu_2_shp_SIZE_TO1              72
 
#define per_2_shp_ADDR_TO1              1033
#define per_2_shp_SIZE_TO1              78
 
#define shp_2_per_ADDR_TO1              1111
#define shp_2_per_SIZE_TO1              72
 
#define uartsh_2_mcu_ADDR_TO1           1183
#define uartsh_2_mcu_SIZE_TO1           69
 
#define mcu_2_ata_ADDR_TO1              1252
#define mcu_2_ata_SIZE_TO1              81
 
#define ata_2_mcu_ADDR_TO1              1333
#define ata_2_mcu_SIZE_TO1              96
 
#define loop_DMAs_routines_ADDR_TO1     1429
#define loop_DMAs_routines_SIZE_TO1     227
 
#define test_ADDR_TO1                   1656
#define test_SIZE_TO1                   63
 
#define signature_ADDR_TO1              1023
#define signature_SIZE_TO1              1
 
/*!
* SDMA RAM scripts start addresses and sizes
*/

#define app_2_per_ADDR_TO1              6144
#define app_2_per_SIZE_TO1              66
 
#define asrc__mcu_ADDR_TO1              6210
#define asrc__mcu_SIZE_TO1              114
 
#define ext_mem__ipu_ram_ADDR_TO1        6324
#define ext_mem__ipu_ram_SIZE_TO1        123
 
#define mcu_2_spdif_ADDR_TO1            6447
#define mcu_2_spdif_SIZE_TO1            103
 
#define p_2_p_ADDR_TO1                  6550
#define p_2_p_SIZE_TO1                  254
 
#define per_2_app_ADDR_TO1              6804
#define per_2_app_SIZE_TO1              74
 
#define spdif_2_mcu_ADDR_TO1            6878
#define spdif_2_mcu_SIZE_TO1            47
 
#define uart_2_per_ADDR_TO1             6925
#define uart_2_per_SIZE_TO1             73
 
#define uartsh_2_per_ADDR_TO1           6998
#define uartsh_2_per_SIZE_TO1           67
 
/*!
* SDMA RAM image start address and size
*/

#define RAM_CODE_START_ADDR_TO1         6144
#define RAM_CODE_SIZE_TO1               921

/*!
* Buffer that holds the SDMA RAM image
*/

static const short sdma_code_TO1[] =
{
0xc1e3, 0x57db, 0x52fb, 0x6ac3, 0x52f3, 0x6ad7, 0x008f, 0x00d5,
0x7d01, 0x008d, 0x05a0, 0x0478, 0x7d03, 0x0479, 0x7d1c, 0x7c21,
0x0479, 0x7c14, 0x6ddd, 0x56ee, 0x62c8, 0x7e28, 0x0660, 0x7d02,
0x0210, 0x0212, 0x6ac8, 0x7f22, 0x0212, 0x6ac8, 0x7f1f, 0x0212,
0x6ac8, 0x7f1c, 0x2003, 0x4800, 0x7cef, 0x9836, 0x6ddd, 0x7802,
0x62c8, 0x6ac8, 0x9835, 0x6dde, 0x0015, 0x7802, 0x62c8, 0x6ac8,
0x9835, 0x0015, 0x0015, 0x7801, 0x62d8, 0x7c08, 0x6ddf, 0x7f06,
0x0000, 0x4d00, 0x7d05, 0xc1fa, 0x57db, 0x9806, 0xc273, 0x0454,
0xc20a, 0x9801, 0xc1d9, 0xc1e3, 0x56f3, 0x57db, 0x047a, 0x7d07,
0x072f, 0x076e, 0x7d02, 0x6ec7, 0x9855, 0x6ed7, 0x9855, 0x074f,
0x076e, 0x7d02, 0x6e01, 0x9855, 0x6e05, 0x5ce3, 0x048f, 0x0410,
0x3c0f, 0x5c93, 0x0eff, 0x06bf, 0x06d5, 0x7d01, 0x068d, 0x05a6,
0x5deb, 0x55fb, 0x008e, 0x0768, 0x7d02, 0x0769, 0x7c04, 0x06d4,
0x7d01, 0x008c, 0x04a0, 0x06a0, 0x076f, 0x7d0c, 0x076e, 0x7d05,
0x7802, 0x62c8, 0x5a05, 0x7c2b, 0x9887, 0x7802, 0x5205, 0x6ac8,
0x7c26, 0x9887, 0x076e, 0x7d05, 0x7802, 0x620b, 0x5a05, 0x7c21,
0x9887, 0x7802, 0x5205, 0x6a0b, 0x7c1c, 0x6a28, 0x7f1a, 0x0768,
0x7d02, 0x0769, 0x7c0a, 0x4c00, 0x7c08, 0x0768, 0x7d03, 0x5a05,
0x7f11, 0x9894, 0x5205, 0x7e0e, 0x5493, 0x4e00, 0x7ccb, 0x0000,
0x54e3, 0x55eb, 0x4d00, 0x7d0a, 0xc1fa, 0x57db, 0x9856, 0x68cc,
0x98a2, 0x680c, 0x009e, 0x0007, 0x54e3, 0xd8a8, 0xc20a, 0x9844,
0x55eb, 0x009d, 0x058c, 0x0aff, 0x0211, 0x1aff, 0x05ba, 0x05a0,
0x04b2, 0x04ad, 0x0454, 0x0006, 0x0e70, 0x0611, 0x5616, 0xc13c,
0x7d2a, 0x5ade, 0x008e, 0xc14e, 0x7c26, 0x5be0, 0x5ef0, 0x5ce8,
0x0688, 0x08ff, 0x0011, 0x28ff, 0x00bc, 0x53f6, 0x05df, 0x7d0b,
0x6dc5, 0x03df, 0x7d03, 0x6bd5, 0xd903, 0x98df, 0x6b05, 0xc5f5,
0x7e27, 0x7f29, 0x98df, 0x6d01, 0x03df, 0x7d05, 0x6bd5, 0xc61f,
0x7e18, 0x7f1a, 0x98df, 0x6b05, 0xc595, 0x7e07, 0x7f06, 0x52de,
0x53e6, 0xc159, 0x7dd7, 0x0200, 0x98b7, 0x0007, 0x6004, 0x680c,
0x53f6, 0x028e, 0x00a3, 0xc256, 0x048b, 0x0498, 0x0454, 0x068a,
0x98df, 0x0207, 0x680c, 0x6ddf, 0x0107, 0x68ff, 0x60d0, 0x98e8,
0x0207, 0x68ff, 0x6d28, 0x0107, 0x6004, 0x680c, 0x98e8, 0x0007,
0x68ff, 0x60d0, 0x98e8, 0x0288, 0x03a5, 0x3b03, 0x3d03, 0x4d00,
0x7d0a, 0x0804, 0x00a5, 0x00da, 0x7d1a, 0x02a0, 0x7b01, 0x65d8,
0x7eee, 0x65ff, 0x7eec, 0x0804, 0x02d0, 0x7d11, 0x4b00, 0x7c0f,
0x008a, 0x3003, 0x6dcf, 0x6bdf, 0x0015, 0x0015, 0x7b02, 0x65d8,
0x0000, 0x7edd, 0x63ff, 0x7edb, 0x3a03, 0x6dcd, 0x6bdd, 0x008a,
0x7b02, 0x65d8, 0x0000, 0x7ed3, 0x65ff, 0x7ed1, 0x0006, 0xc1d9,
0xc1e3, 0x57db, 0x52f3, 0x047a, 0x7d06, 0x0479, 0x7c02, 0x6ac6,
0x993c, 0x6ac7, 0x993c, 0x6a01, 0x008f, 0x00d5, 0x7d01, 0x008d,
0x05a0, 0x5deb, 0x56fb, 0x0478, 0x7d4e, 0x0479, 0x7c1f, 0x0015,
0x0388, 0x047a, 0x7d03, 0x62c8, 0x7e39, 0x9950, 0x620a, 0x7e38,
0x0808, 0x7801, 0x0217, 0x5a06, 0x7f34, 0x2301, 0x047a, 0x7d03,
0x62c8, 0x7e2c, 0x995d, 0x620a, 0x7e2b, 0x0808, 0x7801, 0x0217,
0x5a26, 0x7f27, 0x2301, 0x4b00, 0x7ce4, 0x997c, 0x0015, 0x0015,
0x0015, 0x047a, 0x7d09, 0x7806, 0x0b00, 0x62c8, 0x5a06, 0x0b01,
0x62c8, 0x5a26, 0x7c13, 0x997c, 0x7806, 0x0b00, 0x620b, 0x5a06,
0x0b01, 0x620b, 0x5a26, 0x7c0c, 0x0b70, 0x0311, 0x5313, 0x0000,
0x55eb, 0x4d00, 0x7d11, 0xc1fa, 0x57db, 0x993c, 0x68cc, 0x9989,
0x680c, 0x0007, 0x0479, 0x7c02, 0x008b, 0x9990, 0x0017, 0x00a3,
0x0b70, 0x0311, 0x5313, 0xc213, 0xc20a, 0x9931, 0x0b70, 0x0311,
0x5313, 0x076c, 0x7c01, 0xc1d9, 0x5efb, 0x068a, 0x076b, 0x7c01,
0xc1d9, 0x5ef3, 0x59db, 0x58d3, 0x018f, 0x0110, 0x390f, 0x008b,
0xc13c, 0x7d2b, 0x5ac0, 0x5bc8, 0xc14e, 0x7c27, 0x0388, 0x0689,
0x5ce3, 0x0dff, 0x0511, 0x1dff, 0x05bc, 0x073e, 0x4d00, 0x7d18,
0x0870, 0x0011, 0x077e, 0x7d09, 0x077d, 0x7d02, 0x5228, 0x99c1,
0x52f8, 0x54db, 0x02bc, 0x02cc, 0x7c09, 0x077c, 0x7d02, 0x5228,
0x99ca, 0x52f8, 0x54d3, 0x02bc, 0x02cc, 0x7d09, 0x0400, 0x99b8,
0x008b, 0x52c0, 0x53c8, 0xc159, 0x7dd6, 0x0200, 0x99a8, 0x08ff,
0x00bf, 0x077f, 0x7d15, 0x0488, 0x00d5, 0x7d01, 0x008d, 0x05a0,
0x5deb, 0x028f, 0x0212, 0x0212, 0x3aff, 0x05da, 0x7c02, 0x073e,
0x99f3, 0x02a4, 0x02dd, 0x7d02, 0x073e, 0x99f3, 0x075e, 0x99f3,
0x55eb, 0x0598, 0x5deb, 0x52f3, 0x54fb, 0x076a, 0x7d26, 0x076c,
0x7d01, 0x9a30, 0x076b, 0x7c57, 0x0769, 0x7d04, 0x0768, 0x7d02,
0x0e01, 0x9a0a, 0x5893, 0x00d6, 0x7d01, 0x008e, 0x5593, 0x05a0,
0x5d93, 0x06a0, 0x7802, 0x5502, 0x5d04, 0x7c1d, 0x4e00, 0x7c08,
0x0769, 0x7d03, 0x5502, 0x7e17, 0x9a17, 0x5d04, 0x7f14, 0x0689,
0x5093, 0x4800, 0x7d01, 0x9a02, 0x9a7b, 0x0015, 0x7806, 0x5502,
0x5d04, 0x074f, 0x5502, 0x5d24, 0x072f, 0x7c01, 0x9a7b, 0x0017,
0x076f, 0x7c01, 0x2001, 0x5593, 0x009d, 0x0007, 0xda82, 0x99d0,
0x6cd3, 0x0769, 0x7d04, 0x0768, 0x7d02, 0x0e01, 0x9a3f, 0x5893,
0x00d6, 0x7d01, 0x008e, 0x5593, 0x05a0, 0x5d93, 0x06a0, 0x7802,
0x5502, 0x6dc8, 0x7c0f, 0x4e00, 0x7c08, 0x0769, 0x7d03, 0x5502,
0x7e09, 0x9a4c, 0x6dc8, 0x7f06, 0x0689, 0x5093, 0x4800, 0x7d01,
0x9a37, 0x9a7b, 0x9a75, 0x6ac3, 0x0769, 0x7d04, 0x0768, 0x7d02,
0x0e01, 0x9a62, 0x5893, 0x00d6, 0x7d01, 0x008e, 0x5593, 0x05a0,
0x5d93, 0x06a0, 0x7802, 0x65c8, 0x5d04, 0x7c0f, 0x4e00, 0x7c08,
0x0769, 0x7d03, 0x65c8, 0x7e09, 0x9a6f, 0x5d04, 0x7f06, 0x0689,
0x5093, 0x4800, 0x7d01, 0x9a5a, 0x9a7b, 0x5593, 0x009d, 0x0007,
0x6cff, 0xda82, 0x99d0, 0x0000, 0x54e3, 0x55eb, 0x4d00, 0x7c01,
0x99d0, 0x99b8, 0x54e3, 0x55eb, 0x0aff, 0x0211, 0x1aff, 0x077f,
0x7c02, 0x05a0, 0x9a8f, 0x009d, 0x058c, 0x05ba, 0x05a0, 0x0210,
0x04ba, 0x04ad, 0x0454, 0x0006, 0xc1e3, 0x57db, 0x52f3, 0x6ac5,
0x52fb, 0x6ad3, 0x008f, 0x00d5, 0x7d01, 0x008d, 0x05a0, 0x5deb,
0x0478, 0x7d03, 0x0479, 0x7d20, 0x7c25, 0x0479, 0x7c19, 0x59e3,
0x56ee, 0x61c8, 0x7e2e, 0x62c8, 0x7e2c, 0x65c8, 0x7e2a, 0x0660,
0x7d03, 0x0112, 0x0112, 0x9ab6, 0x0512, 0x0512, 0x0211, 0x02a9,
0x02ad, 0x6ac8, 0x7f1e, 0x2003, 0x4800, 0x7ceb, 0x51e3, 0x9ad0,
0x7802, 0x62c8, 0x6ac8, 0x9acf, 0x6dce, 0x0015, 0x7802, 0x62c8,
0x6ac8, 0x9acf, 0x6dcf, 0x0015, 0x0015, 0x7801, 0x62d8, 0x7c09,
0x6ddf, 0x7f07, 0x0000, 0x55eb, 0x4d00, 0x7d06, 0xc1fa, 0x57db,
0x9a9a, 0x0007, 0x68ff, 0xc213, 0xc20a, 0x9a95, 0xc1d9, 0xc1e3,
0x57db, 0x52f3, 0x047a, 0x7d02, 0x6ad7, 0x9ae7, 0x6a05, 0x008f,
0x00d5, 0x7d01, 0x008d, 0x05a0, 0x56fb, 0x0015, 0x0015, 0x0015,
0x047a, 0x7d07, 0x7804, 0x5206, 0x6ac8, 0x5226, 0x6ac8, 0x7c0f,
0x9b01, 0x7804, 0x5206, 0x6a0b, 0x5226, 0x6a0b, 0x7c0a, 0x6a28,
0x7f08, 0x0000, 0x4d00, 0x7d07, 0xc1fa, 0x57db, 0x9ae7, 0xc273,
0x9b0a, 0xc277, 0x0454, 0xc20a, 0x9ae0, 0xc1e3, 0x57db, 0x52f3,
0x6ad5, 0x56fb, 0x028e, 0x1a94, 0x6ac3, 0x62c8, 0x0269, 0x7d1e,
0x1e94, 0x6ee3, 0x62d0, 0x5aeb, 0x62c8, 0x0248, 0x6ed3, 0x6ac8,
0x2694, 0x52eb, 0x6ad5, 0x6ee3, 0x62c8, 0x026e, 0x7d27, 0x6ac8,
0x7f23, 0x2501, 0x4d00, 0x7d26, 0x028e, 0x1a98, 0x6ac3, 0x62c8,
0x6ec3, 0x0260, 0x7df1, 0x62d0, 0xc27a, 0x9b52, 0x6ee3, 0x008f,
0x2001, 0x00d5, 0x7d01, 0x008d, 0x05a0, 0x62c8, 0x026e, 0x7d0e,
0x6ac8, 0x7f0a, 0x2001, 0x7cf9, 0x6add, 0x7f06, 0x0000, 0x4d00,
0x7d09, 0xc1fa, 0x57db, 0x9b11, 0x0007, 0x6aff, 0x62d0, 0xc27a,
0x0458, 0x0454, 0x6add, 0x7ff8, 0xc20a, 0x9b0e, 0xc1d9, 0xc1e3,
0x57db, 0x52f3, 0x6ad5, 0x56fb, 0x028e, 0x1a94, 0x5202, 0x0269,
0x7d17, 0x1e94, 0x5206, 0x0248, 0x5a06, 0x2694, 0x5206, 0x026e,
0x7d26, 0x6ac8, 0x7f22, 0x2501, 0x4d00, 0x7d27, 0x028e, 0x1a98,
0x5202, 0x0260, 0x7df3, 0x6add, 0x7f18, 0x62d0, 0xc27a, 0x9b95,
0x008f, 0x2001, 0x00d5, 0x7d01, 0x008d, 0x05a0, 0x5206, 0x026e,
0x7d0e, 0x6ac8, 0x7f0a, 0x2001, 0x7cf9, 0x6add, 0x7f06, 0x0000,
0x4d00, 0x7d0b, 0xc1fa, 0x57db, 0x9b5b, 0x0007, 0x6aff, 0x6add,
0x7ffc, 0x62d0, 0xc27a, 0x0458, 0x0454, 0x6add, 0x7ff6, 0xc20a,
0x9b58
};
#endif