This directory contains the USB device (function) MDD.

This is based on the \PUBLIC\Common\OAK\Drivers\USBFN driver, with a 
small modification to handle the USB_FEATURE_TEST_MODE Device-based
request.  This request is sent by a host to place the USB device into
one of various test modes, for physical-layer tests.  These modes place
the physical layer signals into specific states or send NAK etc.  The
test mode is entered immediately after the feature setting is handled.

See USB2.0 spec section 9.4.9.

This directory generates library usb_ufnmddbase_mx35_fsl_v2.lib instead of the
standard UFNMDDBASE.lib from the PUBLIC code.

To handle test mode, we define an IOCTL for the MDD to set the PDD into
test mode.   To compile the for test mode:

1)
define USBFN_TEST_MODE_SUPPORT in order to compile the PDD for test
mode support in PLATFORM\COMMON\SRC\SOC\MX35_FSL_V2\USBD\pdd.c

2) 
include PLATFORM\COMMON\SRC\SOC\MX35_FSL_V2\USBFN in build via
PLATFORM\COMMON\SRC\SOC\MX35_FSL_V2\dirs

3) 
use the library generated here, by changing ufnmddbase.lib in 
WINCE600\PLATFORM\iMX35-3DS\SRC\DRIVERS\USBD\sources

    $(_COMMONOAKROOT)\lib\$(_CPUINDPATH)\ufnmddbase.lib \

becomes

    $(_CSPCOMMONLIB)\$(_CPUINDPATH)\usb_ufnmddbase_mx35_fsl_v2.lib \
