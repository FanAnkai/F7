::请根据相关文档，安装rda5981的工具链
::请根据实际情况，将 iot_sdk_root 设置为qly sdk 根目录，将rda5981_sdk_mbed_root设置为RDA sdk根目录
::完成修改后，请将本文件复制至RDA sdk根目录下，然后双击运行

set iot_sdk_root=E:\gitcode\sdk3.0
set rda5981_sdk_mbed_root=D:\work\RDA5981_SDK_MbedOS515_V1.3.5

::yes or no
set NEED_DEBUG=no
::yes or no
set NEED_LAN_DETECT=yes
::yes or no
set NEED_LOCAL_SAVE=yes
::company number(0,1,2,3...)
set COMPANY=0

if %NEED_DEBUG% == yes (
    set DEBUG_FLAG=-DDEBUG_VERSION
)else (
    set DEBUG_FLAG= 
)

if %NEED_LAN_DETECT% == yes (
    set LAN_DETECT_FLAG=-DLAN_DETECT
)else (
    set LAN_DETECT_FLAG= 
)

if %NEED_LOCAL_SAVE% == yes (
    set LOCAL_SAVE_FLAG=-DLOCAL_SAVE
)else (
    set LOCAL_SAVE_FLAG= 
)

mbed compile --library --source %rda5981_sdk_mbed_root%   --source %iot_sdk_root%\include  --source %iot_sdk_root%\sdk\src  --source %iot_sdk_root%\sdk\thirdparty  --source %iot_sdk_root%\sdk\platforms\include   --source %iot_sdk_root%\sdk\platforms\src\bc30_mbed -t ARM -m UNO_81A -c %LAN_DETECT_FLAG% %DEBUG_FLAG% %LOCAL_SAVE_FLAG% -D__COMPANY__=%COMPANY% -DSUPPORT_REBOOT_OTA  -DSUPPORT_SUBDEV -DFIRMWARE_GW -D__SDK_TYPE__=0 -D__SDK_PLATFORM__=0x12
if %NEED_DEBUG% == yes (
	copy D:\work\RDA5981_SDK_MbedOS515_V1.3.5\BUILD\libraries\RDA5981_SDK_MbedOS515_V1.3.5\UNO_81A\ARM\RDA5981_SDK_MbedOS515_V1.3.5.ar E:\gitcode\sdk3.0\lib\libiot_bc30_mbed_debug.ar
)else (
	copy D:\work\RDA5981_SDK_MbedOS515_V1.3.5\BUILD\libraries\RDA5981_SDK_MbedOS515_V1.3.5\UNO_81A\ARM\RDA5981_SDK_MbedOS515_V1.3.5.ar E:\gitcode\sdk3.0\lib\libiot_bc30_mbed.ar
)


pause