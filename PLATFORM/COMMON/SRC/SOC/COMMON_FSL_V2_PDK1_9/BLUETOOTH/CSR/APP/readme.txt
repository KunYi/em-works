                < BlueTooth Application Guide >
                
Because WinCE provides poor application to support Bluetooth, we modify codes under public directory and provide ceplayre.exe(for AVRCP), bthpnl.cpl and netui.dll(for pairing and unpairing) under <TARGET PLATFORM>\FILES to improve BT function. If you want to modify these files to perfect BT function, please follow the below steps to build it.

Build steps:
<Ceplayer.exe>
1. deselect 'make run-time image after building'in PB->Build->Targeted Build Settings
2. After you have modified ceplayer.cpp, please Open WINCE600\PUBLIC\DIRECTX\SDK\SAMPLES\WMP\CEPLAYER from PB->Solution Explorer and select build in rightkey menu
3. delete ceplayer.exe under <TARGET PLATFORM>\FILES
4. Sysgen

<bthpnl.cpl/netui.dll>
1. deselect 'make run-time image after building'in PB->Build->Targeted Build Settings
2. After you have modified btenum.cxx, please Open WINCE600\PUBLIC\COMMON\OAK\DRIVERS\BLUETOOTH\SAMPLE\BTENUM from PB->Solution Explorer and select build in rightkey menu; Then modify btmgmtui.cpp, and open WINCE600\PUBLIC\COMMON\OAK\DRIVERS\NETUI from PB->Solution Explorer and select build in rightkey menu; Then Open WINCE600\PUBLIC\WCESHELLFE\OAK\CTLPNL\BTHPNL from PB->Solution Explorer and select rebuild 
3. delete bthpnl.cpl/netui.dll under <TARGET PLATFORM>\FILES
4. Sysgen.
