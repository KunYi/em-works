//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  omap5912_irq.h
//
//  This file defines names for IRQ. This names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
#ifndef __OMAP5912_IRQ_H
#define __OMAP5912_IRQ_H

//------------------------------------------------------------------------------

#define OMAP5912_IRQ_MAXIMUM         240
#define OMAP5912_IRQ_PER_SYSINTR     4

//------------------------------------------------------------------------------

#define IRQ_L2IRQ                   0
#define IRQ_CAMERA_IF               1
#define IRQ_L2FIQ                   2
#define IRQ_EXT_NFIQ                3
#define IRQ_MCBSP2_TX               4
#define IRQ_MCBSP2_RX               5
#define IRQ_RTDX                    6
#define IRQ_DSP_MMU_ABORT           7
#define IRQ_HOST_INIT               8
#define IRQ_ABORT                   9
#define IRQ_DSP_MAILBOX1            10
#define IRQ_DSP_MAILBOX2            11
#define IRQ_LCD_LINE                12
#define IRQ_PRIVATE_TIPB_ABORT      13
#define IRQ_GPIO1                   14
#define IRQ_UART3                   15
#define IRQ_TIMER3                  16
#define IRQ_GPTIMER1                17
#define IRQ_GPTIMER2                18
#define IRQ_DMA_CH0                 19	/*DMA must be in OMAP3.2 mode*/
#define IRQ_DMA_CH1                 20  /*DMA must be in OMAP3.2 mode*/
#define IRQ_DMA_CH2                 21  /*DMA must be in OMAP3.2 mode*/
#define IRQ_DMA_CH3                 22  
#define IRQ_DMA_CH4                 23  
#define IRQ_DMA_CH5                 24  
#define IRQ_DMA_LCD                 25
#define IRQ_TIMER1                  26
#define IRQ_WD_TIMER                27
#define IRQ_PUBLIC_TIPB_ABORT       28 
#define IRQ_SSR_FIFO_FULL_CH0       29
#define IRQ_TIMER2                  30
#define IRQ_LCD_CTRL                31

// Note: IRQ's 32 and higher are handled by the Level 2 interrupt controller

#define IRQ_FAC                     32
#define IRQ_KEYBOARD                33
#define IRQ_MWIRE_TX                34
#define IRQ_MWIRE_RX                35
#define IRQ_I2C                     36
#define IRQ_MPUIO                   37
#define IRQ_USB_HHC_1               38
#define IRQ_USB_HHC_2               39
#define IRQ_USB_OTG                 40
#define IRQ_SOSSI_ATTN              41
#define IRQ_MCBSP3_TX               42
#define IRQ_MCBSP3_RX               43
#define IRQ_MCBSP1_TX               44
#define IRQ_MCBSP1_RX               45
#define IRQ_UART1                   46
#define IRQ_UART2                   47
#define IRQ_MCSI1                   48
#define IRQ_MCSI2                   49
#define IRQ_50                      50
#define IRQ_SOSSI_MATCH             51
#define IRQ_USB_GENI                52
#define IRQ_1WIRE                   53
#define IRQ_TIMER32K                54
#define IRQ_MMC_SDIO                55
#define IRQ_32K_USB_WAKEUP          56
#define IRQ_RTC                     57
#define IRQ_RTC_ALARM               58
#define IRQ_MEMORY_STICK            59
#define IRQ_DSP_MMU                 60
#define IRQ_USBISO					61  //Isochronous mode interrupt
#define IRQ_USB                     62  //(NON-ISO)
#define IRQ_MCBSP2_RX_OVERFLOW      63

#define IRQ_STI_GLOBAL              64
#define IRQ_STI_WAKEUP              65
#define IRQ_GPTIMER3                66
#define IRQ_GPTIMER4                67
#define IRQ_GPTIMER5                68
#define IRQ_GPTIMER6                69
#define IRQ_GPTIMER7                70
#define IRQ_GPTIMER8                71
#define IRQ_GPIO2                   72
#define IRQ_GPIO3                   73
#define IRQ_MMC_SDIO2               74
#define IRQ_COMPACT_FLASH           75
#define IRQ_COMM_RX                 76
#define IRQ_COMM_TX                 77
#define IRQ_WAKEUP_REQ              78
#define IRQ_79                      79
#define IRQ_GPIO4                   80
#define IRQ_SPI                     81
#define IRQ_CCP_STATUS              82
#define IRQ_CCP_FIFO_NOTEMPTY       83
#define IRQ_CCP_ATTN                84
#define IRQ_DMA_CH6                 85
#define IRQ_DMA_CH7                 86
#define IRQ_DMA_CH8                 87
#define IRQ_DMA_CH9                 88
#define IRQ_DMA_CH10                89
#define IRQ_DMA_CH11                90
#define IRQ_DMA_CH12                91
#define IRQ_DMA_CH13                92
#define IRQ_DMA_CH14                93
#define IRQ_DMA_CH15                94
#define IRQ_NAND                    95

#define IRQ_USB_HHC2_SUSPEND        96
#define IRQ_SST_FIFO_EMPTY_CH0      97
#define IRQ_98                      98
#define IRQ_SSR_OVERRUN_CH0         99
#define IRQ_SST_FIFO_EMPTY_CH1     100
#define IRQ_SSR_FIFO_FULL_CH1      101
#define IRQ_SSR_OVERRUN_CH1        102
#define IRQ_SST_FIFO_EMPTY_CH2     103
#define IRQ_SSR_FIFO_FULL_CH2      104
#define IRQ_SSR_OVERRUN_CH2        105
#define IRQ_SST_FIFO_EMPTY_CH3     106
#define IRQ_SSR_FIFO_FULL_CH3      107
#define IRQ_SSR_OVERRUN_CH3        108
#define IRQ_SST_FIFO_EMPTY_CH4     109
#define IRQ_SSR_FIFO_FULL_CH4      110
#define IRQ_SSR_OVERRUN_CH4        111
#define IRQ_SST_FIFO_EMPTY_CH5     112
#define IRQ_SSR_FIFO_FULL_CH5      113
#define IRQ_SSR_OVERRUN_CH5        114
#define IRQ_SST_FIFO_EMPTY_CH6     115
#define IRQ_SSR_FIFO_FULL_CH6      116
#define IRQ_SSR_OVERRUN_CH6        117
#define IRQ_SST_FIFO_EMPTY_CH7     118
#define IRQ_SSR_FIFO_FULL_CH7      119
#define IRQ_SSR_OVERRUN_CH7        120
#define IRQ_SSR_SIGNALING_ERROR    121
#define IRQ_SSR_CONTROLLER         122
#define IRQ_SHA1_MD5               123
#define IRQ_RNG                    124
#define IRQ_RNG_IDLE               125
#define IRQ_VLYNQ                  126
#define IRQ_GDD_LCH0               127

#define IRQ_GDD_LCH1               128
#define IRQ_GDD_LCH2               129
#define IRQ_GDD_LCH3               130
#define IRQ_GDD_LCH4               131
#define IRQ_GDD_LCH5               132
#define IRQ_GDD_LCH6               133
#define IRQ_GDD_LCH7               134
#define IRQ_135                    135
#define IRQ_136                    136
#define IRQ_137                    137
#define IRQ_138                    138
#define IRQ_139                    139
#define IRQ_140                    140
#define IRQ_141                    141
#define IRQ_142                    142
#define IRQ_143                    143
#define IRQ_144                    144
#define IRQ_145                    145
#define IRQ_146                    146
#define IRQ_147                    147
#define IRQ_148                    148
#define IRQ_149                    149
#define IRQ_150                    150
#define IRQ_151                    151
#define IRQ_152                    152
#define IRQ_153                    153
#define IRQ_154                    154
#define IRQ_155                    155
#define IRQ_156                    156
#define IRQ_157                    157
#define IRQ_158                    158
#define IRQ_159                    159

#define IRQ_GPIO_0                 160         // GPIO1 bit 0
#define IRQ_GPIO_16                176         // GPIO2 bit 0
#define IRQ_GPIO_32                192         // GPIO3 bit 0
#define IRQ_GPIO_48                208         // GPIO4 bit 0

#define IRQ_MPUIO_0                224         // MPUIO bit 0

//------------------------------------------------------------------------------

#endif // __OMAP5912_IRQ_H
