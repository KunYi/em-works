;------------------------------------------------------------------------------
;
;   Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------


    AREA   |.data|, DATA,align=12;  page alignment for kitl buffer. 
    EXPORT g_rndis_ep0_buf[DATA]
    EXPORT g_rndis_ep2_buf[DATA]
    EXPORT g_rndis_ep3_buf[DATA]
    EXPORT g_serial_send_buf[DATA]
    EXPORT g_serial_rev_buf[DATA]

ALIGN   

g_rndis_ep2_buf
g_serial_rev_buf
    SPACE 8192
    
g_rndis_ep3_buf
g_serial_send_buf
    SPACE 8192  

g_rndis_ep0_buf
    SPACE 1024
     END
