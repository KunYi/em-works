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

#define SineSize 1200

const int SineWave[SineSize] = {

0xFFF3,0x0752,0x0EAA,0x15F6,0x1D2E,0x244E,0x2B50,0x322C,0x38DE,0x3F61,0x45AD,0x4BBD,0x518D,0x5718,0x5C5A,0x614C,
0x65ED,0x6A37,0x6E26,0x71B8,0x74E9,0x77B7,0x7A20,0x7C20,0x7DB8,0x7EE5,0x7FA6,0x7FFA,0x7FE2,0x7F5E,0x7E6E,0x7D12,
0x7B4B,0x791C,0x7686,0x738D,0x7030,0x6C75,0x685C,0x63EC,0x5F27,0x5A11,0x54AF,0x4F05,0x4917,0x42EC,0x3C87,0x35EF,
0x2F29,0x283C,0x212C,0x1A01,0x12BF,0x0B6D,0x0412,0xFCB2,0xF556,0xEE04,0xE6C0,0xDF91,0xD87E,0xD18D,0xCAC2,0xC427,
0xBDBC,0xB78B,0xB196,0xABE5,0xA67B,0xA15D,0x9C8F,0x9815,0x93F5,0x9030,0x8CC9,0x89C4,0x8723,0x84EA,0x8318,0x81B2,
0x80B6,0x8026,0x8004,0x804B,0x8101,0x8224,0x83B1,0x85A6,0x8803,0x8AC7,0x8DEE,0x9177,0x955D,0x999D,0x9E35,0xA31F,
0xA858,0xADDC,0xB3A5,0xB9B0,0xBFF6,0xC673,0xCD1F,0xD3F8,0xDAF6,0xE212,0xE949,0xF093,0xF7EB,0xFF48,0x06A7,0x0DFF,
0x154D,0x1C87,0x23AA,0x2AAE,0x318E,0x3844,0x3ECB,0x451B,0x4B32,0x5109,0x569B,0x5BE2,0x60DC,0x6584,0x69D6,0x6DCE,
0x7169,0x74A4,0x777A,0x79EC,0x7BF6,0x7D97,0x7ECF,0x7F99,0x7FF8,0x7FE9,0x7F6E,0x7E88,0x7D36,0x7B79,0x7954,0x76C6,
0x73D6,0x7083,0x6CCF,0x68C0,0x6456,0x5F9A,0x5A8A,0x552E,0x4F8B,0x49A3,0x437D,0x3D1E,0x368B,0x2FC9,0x28DE,0x21D3,
0x1AA9,0x1369,0x0C19,0x04BD,0xFD5E,0xF601,0xEEAD,0xE768,0xE037,0xD922,0xD22D,0xCB60,0xC4BE,0xBE4F,0xB818,0xB21E,
0xAC66,0xA6F6,0xA1D1,0x9CFB,0x987A,0x9450,0x9083,0x8D14,0x8A06,0x875D,0x851A,0x833F,0x81CE,0x80C8,0x802F,0x8001,
0x8041,0x80ED,0x8205,0x8388,0x8574,0x87C8,0x8A83,0x8DA1,0x9121,0x94FF,0x9937,0x9DC6,0xA2AA,0xA7DC,0xAD59,0xB31B,
0xB921,0xBF62,0xC5DA,0xCC83,0xD357,0xDA52,0xE16D,0xE8A1,0xEFE9,0xF73F,0xFE9C,0x05FB,0x0D55,0x14A3,0x1BE0,0x2305,
0x2A0C,0x30F0,0x37AA,0x3E36,0x448C,0x4AA6,0x5083,0x561C,0x5B6B,0x606D,0x651B,0x6975,0x6D75,0x7119,0x745C,0x773C,
0x79B7,0x7BCB,0x7D76,0x7EB7,0x7F8B,0x7FF4,0x7FEF,0x7F7E,0x7EA1,0x7D59,0x7BA5,0x798A,0x7706,0x741F,0x70D4,0x6D29,
0x6922,0x64C1,0x600B,0x5B04,0x55AF,0x5011,0x4A2F,0x440E,0x3DB5,0x3727,0x3068,0x2981,0x2277,0x1B51,0x1412,0x0CC2,
0x0569,0xFE09,0xF6AD,0xEF57,0xE811,0xE0DD,0xD9C5,0xD2CD,0xCBFC,0xC557,0xBEE3,0xB8A7,0xB2A6,0xACE9,0xA771,0xA245,
0x9D68,0x98DF,0x94AE,0x90D7,0x8D60,0x8A49,0x8796,0x854A,0x8366,0x81EC,0x80DB,0x8038,0x8002,0x8037,0x80D9,0x81E8,
0x8360,0x8542,0x878F,0x8A40,0x8D55,0x90CC,0x94A1,0x98D0,0x9D59,0xA235,0xA75F,0xACD6,0xB293,0xB892,0xBECE,0xC541,
0xCBE5,0xD2B7,0xD9AD,0xE0C5,0xE7F8,0xEF3F,0xF694,0xFDF1,0x0550,0x0CAB,0x13F9,0x1B38,0x225F,0x296A,0x3051,0x3710,
0x3D9F,0x43F9,0x4A1B,0x4FFE,0x559C,0x5AF3,0x5FFB,0x64B2,0x6913,0x6D1C,0x70C8,0x7414,0x76FD,0x7983,0x7BA0,0x7D54,
0x7E9D,0x7F7D,0x7FEF,0x7FF4,0x7F8D,0x7EBA,0x7D7B,0x7BD2,0x79BE,0x7745,0x7465,0x7123,0x6D83,0x6983,0x652B,0x607D,
0x5B7C,0x562D,0x5097,0x4ABB,0x44A1,0x3E4A,0x37C1,0x3107,0x2A23,0x231D,0x1BF8,0x14BC,0x0D6D,0x0614,0xFEB5,0xF758,
0xF002,0xE8B9,0xE185,0xDA69,0xD36E,0xCC99,0xC5EE,0xBF77,0xB936,0xB330,0xAD6C,0xA7EE,0xA2B9,0x9DD6,0x9946,0x950C,
0x912D,0x8DAC,0x8A8D,0x87D1,0x857B,0x838D,0x8209,0x80F0,0x8043,0x8002,0x802E,0x80C6,0x81CA,0x8339,0x8513,0x8754,
0x89FD,0x8D0A,0x9077,0x9443,0x986C,0x9CEC,0xA1C0,0xA6E5,0xAC54,0xB20B,0xB804,0xBE3B,0xC4A8,0xCB48,0xD216,0xD90A,
0xE01F,0xE74F,0xEE95,0xF5E9,0xFD45,0x04A5,0x0BFF,0x1350,0x1A91,0x21BA,0x28C8,0x2FB2,0x3675,0x3D08,0x4369,0x498F,
0x4F78,0x551D,0x5A79,0x5F88,0x6448,0x68B1,0x6CC2,0x7076,0x73CB,0x76BE,0x794C,0x7B73,0x7D31,0x7E84,0x7F6D,0x7FE8,
0x7FF8,0x7F9A,0x7ED1,0x7D9C,0x7BFD,0x79F3,0x7783,0x74AD,0x7174,0x6DDA,0x69E4,0x6593,0x60EC,0x5BF3,0x56AD,0x511B,
0x4B46,0x4531,0x3EE1,0x385B,0x31A5,0x2AC5,0x23C2,0x1C9F,0x1564,0x0E19,0x06BF,0xFF60,0xF804,0xF0AB,0xE962,0xE22B,
0xDB0D,0xD40F,0xCD36,0xC688,0xC00C,0xB9C4,0xB3B9,0xADEF,0xA86B,0xA32F,0x9E45,0x99AC,0x956A,0x9184,0x8DFA,0x8AD2,
0x880D,0x85AD,0x83B5,0x8228,0x8104,0x804D,0x8003,0x8025,0x80B3,0x81AE,0x8313,0x84E4,0x871C,0x89BB,0x8CBE,0x9023,
0x93E6,0x9807,0x9C80,0xA14C,0xA669,0xABD3,0xB183,0xB776,0xBDA7,0xC411,0xCAAD,0xD176,0xD867,0xDF79,0xE6A7,0xEDEB,
0xF53E,0xFC9A,0x03F8,0x0B54,0x12A6,0x19E8,0x2115,0x2826,0x2F13,0x35D9,0x3C71,0x42D7,0x4903,0x4EF1,0x549C,0x5A00,
0x5F17,0x63DC,0x684E,0x6C68,0x7024,0x7382,0x767E,0x7914,0x7B44,0x7D0D,0x7E6A,0x7F5D,0x7FE2,0x7FFB,0x7FA7,0x7EE9,
0x7DBD,0x7C27,0x7A27,0x77C0,0x74F4,0x71C2,0x6E32,0x6A43,0x65FB,0x615D,0x5C6A,0x572B,0x51A0,0x4BD0,0x45C1,0x3F75,
0x38F4,0x3243,0x2B67,0x2466,0x1D46,0x160E,0x0EC2,0x076A,0x000C,0xF8AE,0xF156,0xEA0A,0xE2D1,0xDBB2,0xD4B1,0xCDD4,
0xC722,0xC0A0,0xBA54,0xB443,0xAE73,0xA8E8,0xA3A6,0x9EB3,0x9A14,0x95CA,0x91DA,0x8E48,0x8B17,0x8849,0x85E0,0x83DF,
0x8248,0x811B,0x8059,0x8005,0x801E,0x80A2,0x8193,0x82EE,0x84B5,0x86E3,0x8979,0x8C74,0x8FD0,0x938C,0x97A4,0x9C14,
0xA0D9,0xA5EF,0xAB52,0xB0FC,0xB6EA,0xBD16,0xC379,0xCA10,0xD0D7,0xD7C3,0xDED4,0xE5FF,0xED41,0xF492,0xFBEE,0x034E,
0x0AAA,0x11FD,0x1941,0x206F,0x2781,0x2E74,0x353E,0x3BD9,0x4244,0x4875,0x4E6A,0x541B,0x5985,0x5EA3,0x6371,0x67EA,
0x6C0C,0x6FD1,0x7337,0x763D,0x78DD,0x7B16,0x7CE7,0x7E4E,0x7F49,0x7FDA,0x7FFE,0x7FB4,0x7EFE,0x7DDD,0x7C50,0x7A5B,
0x77FD,0x7538,0x7212,0x6E8A,0x6AA2,0x6662,0x61CB,0x5CE0,0x57A8,0x5224,0x4C5A,0x4650,0x400A,0x398E,0x32E1,0x2C09,
0x250A,0x1DED,0x16B6,0x0F6D,0x0816,0x00B8,0xF95A,0xF201,0xEAB4,0xE379,0xDC56,0xD552,0xCE72,0xC7BB,0xC135,0xBAE4,
0xB4CE,0xAEF8,0xA966,0xA41E,0x9F23,0x9A7B,0x962A,0x9232,0x8E98,0x8B5D,0x8885,0x8613,0x840A,0x8268,0x8133,0x8068,
0x8008,0x8017,0x8091,0x8178,0x82CB,0x8487,0x86AD,0x8939,0x8C2A,0x8F7D,0x9331,0x9741,0x9BA9,0xA067,0xA575,0xAAD1,
0xB075,0xB65C,0xBC83,0xC2E2,0xC975,0xD038,0xD721,0xDE2E,0xE557,0xEC98,0xF3E8,0xFB43,0x02A2,0x09FE,0x1153,0x1898,
0x1FC9,0x26DF,0x2DD3,0x34A1,0x3B41,0x41B1,0x47E8,0x4DE1,0x5399,0x590A,0x5E2F,0x6304,0x6786,0x6BB0,0x6F7C,0x72ED,
0x75FA,0x78A3,0x7AE6,0x7CC1,0x7E32,0x7F37,0x7FD1,0x7FFF,0x7FBF,0x7F13,0x7DFB,0x7C78,0x7A8C,0x7837,0x757D,0x725F,
0x6EDF,0x6B01,0x66CA,0x623A,0x5D56,0x5824,0x52A7,0x4CE4,0x46DF,0x409E,0x3A27,0x337E,0x2CA9,0x25AF,0x1E94,0x1760,
0x1017,0x08C1,0x0163,0xFA05,0xF2AB,0xEB5C,0xE420,0xDCFA,0xD5F3,0xCF0F,0xC856,0xC1CC,0xBB75,0xB559,0xAF7C,0xA9E5,
0xA495,0x9F94,0x9AE5,0x968B,0x928B,0x8EE7,0x8BA4,0x88C4,0x8649,0x8435,0x828A,0x8149,0x8075,0x800C,0x8011,0x8081,
0x815E,0x82A8,0x845B,0x8677,0x88FA,0x8BE2,0x8F2C,0x92D7,0x96DE,0x9B3F,0x9FF6,0xA4FC,0xAA51,0xAFEF,0xB5D0,0xBBF1,
0xC24B,0xC8DA,0xCF99,0xD67E,0xDD89,0xE4AF,0xEBEE,0xF33D,0xFA97,0x01F7,0x0954,0x10A8,0x17F0,0x1F22,0x263A,0x2D32,
0x3404,0x3AAA,0x411D,0x4759,0x4D59,0x5317,0x588F,0x5DBB,0x6297,0x6720,0x6B52,0x6F29,0x72A0,0x75B7,0x7869,0x7AB6,
0x7C9A,0x7E15,0x7F25,0x7FC8,0x7FFF,0x7FC9,0x7F28,0x7E19,0x7CA1,0x7ABD,0x7872,0x75C1,0x72AC,0x6F35,0x6B60,0x672F,
0x62A7,0x5DCB,0x58A1,0x532A,0x4D6C,0x476E,0x4132,0x3ABF,0x341A,0x2D4A,0x2653,0x1F3A,0x1808,0x10C1,0x096C,0x0210,
0xFAB0,0xF356,0xEC06,0xE4C8,0xDDA0,0xD696,0xCFAF,0xC8F0,0xC261,0xBC06,0xB5E4,0xB002,0xAA64,0xA50F,0xA005,0x9B4E,
0x96ED,0x92E4,0x8F39,0x8BEC,0x8902,0x867E,0x8460,0x82AD,0x8163,0x8084,0x8012,0x800C,0x8073,0x8147,0x8285,0x842E,
0x8641,0x88BB,0x8B9A,0x8EDC,0x927E,0x967C,0x9AD5,0x9F84,0xA484,0xA9D2,0xAF6A,0xB545,0xBB5F,0xC1B5,0xC840,0xCEF9,
0xD5DC,0xDCE3,0xE408,0xEB44,0xF293,0xF9EC,0x014B,0x08A8,0x0FFE,0x1748,0x1E7C,0x2597,0x2C92,0x3367,0x3A11,0x4089,
0x46CA,0x4CD0,0x5294,0x5812,0x5D45,0x622A,0x66BB,0x6AF4,0x6ED3,0x7254,0x7573,0x782F,0x7A84,0x7C73,0x7DF7,0x7F10,
0x7FBE,0x7FFF,0x7FD2,0x7F3A,0x7E36,0x7CC6,0x7AED,0x78AB,0x7604,0x72F7,0x6F89,0x6BBD,0x6794,0x6315,0x5E3F,0x591C,
0x53AC,0x4DF6,0x47FC,0x41C5,0x3B58,0x34B7,0x2DEA,0x26F6,0x1FE1,0x18B1,0x116B,0x0A18,0x02BA,0xFB5B,0xF400,0xECB1,
0xE570,0xDE45,0xD738,0xD04E,0xC98C,0xC2F8,0xBC97,0xB671,0xB089,0xAAE4,0xA587,0xA076,0x9BB8,0x974F,0x933E,0x8F89,
0x8C35,0x8942,0x86B4,0x848D,0x82D0,0x817C,0x8093,0x8017,0x8008,0x8065,0x812E,0x8264,0x8403,0x860D,0x887E,0x8B53,
0x8E8C,0x9225,0x961C,0x9A6D,0x9F13,0xA40C,0xA954,0xAEE5,0xB4BA,0xBAD0,0xC120,0xC7A6,0xCE5B,0xD53A,0xDC3E,0xE361,
0xEA9B,0xF1E8,0xF941,0x00A0,0x07FD,0x0F54,0x169E,0x1DD6,0x24F3,0x2BF1,0x32CA,0x3978,0x3FF5,0x463B,0x4C47,0x5212,
0x5796,0x5CD0,0x61BB,0x6654,0x6A96,0x6E7D,0x7206,0x752F,0x77F4,0x7A53,0x7C4A,0x7DD8,0x7EFB,0x7FB3,0x7FFD,0x7FDB,
0x7F4D,0x7E52,0x7CED,0x7B1D,0x78E4,0x7645,0x7342,0x6FDD,0x6C1A,0x67F8,0x6381,0x5EB4,0x5996,0x542E,0x4E7D,0x488A,
0x425A,0x3BEF,0x3554,0x2E8B,0x279A,0x2087,0x1959,0x1215,0x0AC2,0x0366,0xFC07,0xF4AB,0xED5A,0xE617,0xDEEB,0xD7DB,
0xD0ED,0xCA27,0xC38F,0xBD2A,0xB6FE,0xB10F,0xAB64,0xA601,0xA0EA,0x9C23,0x97B2,0x9399,0x8FDC,0x8C7E,0x8983,0x86EC,
0x84BC,0x82F3,0x8196,0x80A4,0x801F,0x8004,0x8058,0x8118,0x8244,0x83D9,0x85D9,0x8841,0x8B0C,0x8E3D,0x91CE,0x95BD,
0x9A05,0x9EA4,0xA396,0xA8D5,0xAE60,0xB42F,0xBA40,0xC08B,0xC70C,0xCDBD,0xD499,0xDB9A,0xE2BA,0xE9F2,0xF13E,0xF895

};
