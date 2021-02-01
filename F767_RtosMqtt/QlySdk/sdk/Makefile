
#OS = arm_linux
#OS = arm_64_linux
#OS = arm_64_linux_aarch64
OS = x86_linux
#OS = x86_64_linux
#OS = cm3_mico
#OS = cm4_mico
#OS = cm4f_mico
#OS = mt7681
#OS = cc3200_freertos
#OS = qca4004_threadX
#OS = rnsaS7_threadX
#OS = mt7628_lede
#OS = ArmCortexA7
#OS = Arm64Android
#OS = esp8266_freertos
#OS = hf_lpb120
#OS = hf_lpb125
#OS = mips_robustos
#OS = arm_robustos
#OS = x86_windows
#OS = x64_windows
#OS = arm_linux_raspbian
#OS = esp32
#OS = m2351_freertos
#OS = Trifo_android-ndk-r17c_standalone
#OS = stm32f407_freertos

NEED_DEBUG = yes
NEED_LAN_DETECT = yes
NEED_LOCAL_SAVE = yes
IS_SUPPORT_REBOOT_OTA = yes

QLY_SDK_LIB=0
QLY_SDK_BIN=1

__SDK_TYPE__ = $(QLY_SDK_LIB)
#__SDK_TYPE__ = $(QLY_SDK_BIN)

SDK_PF_X86_LINUX = 0x00
SDK_PF_ARM_LINUX = 0x01
SDK_PF_ESP8266_LIB = 0x02
SDK_PF_ESP8266_BIN = 0x03
SDK_PF_LPB120 = 0x04
SDK_PF_CC3200 = 0x05
SDK_PF_QCA4004B = 0x06
SDK_PF_8711AM = 0x07
SDK_PF_LPB200U = 0x08
SDK_PF_MT7681 = 0x09
SDK_PF_W500 = 0x0A
SDK_PF_X64_LINUX = 0x0B
SDK_PF_CM3_STM32 = 0x0C
SDK_PF_CM3_MICO = 0x0D
SDK_PF_CM4_MICO = 0x0E
SDK_PF_CM4F_MICO = 0x0F
SDK_PF_RNSAS7 = 0x10
SDK_PF_ARM_64_LINUX = 0x11
SDK_PF_BC30 = 0x12
SDK_PF_M26 = 0x13
SDK_PF_L206 = 0x14
SDK_PF_L218 = 0x15
SDK_PF_LPT230 = 0x16
SDK_PF_MT7628 = 0x17
SDK_PF_LPXX2X = 0x18
SDK_PF_ANDROID = 0x19
SDK_PF_BK7231_RTTHREAD = 0x1A
SDK_PF_ROBUSTOS = 0x1B
SDK_PF_X86_WINDOWS = 0x1C
SDK_PF_X64_WINDOWS = 0x1D
SDK_PF_ESP32_LIB = 0x1E
SDK_PF_M2351 = 0x1F
SDK_PF_TRIFO_NDK_R17C = 0x20
SDK_PF_STM32F407_FREERTOS = 0x21

SDK_ROOT:= $(shell pwd)

ifeq ($(OS), x86_linux)
	__SDK_PLATFORM__=$(SDK_PF_X86_LINUX)
	CC=gcc
	AR=ar
	CFLAGS= -g -O2 -c
else ifeq ($(OS), x86_64_linux)
	__SDK_PLATFORM__=$(SDK_PF_X64_LINUX)
	CC=gcc
	AR=ar
	CFLAGS= -g -O2 -c
else ifeq ($(OS), mips_robustos)
	__SDK_PLATFORM__=$(SDK_PF_ROBUSTOS)
	CC=mips-ar9341-linux-gnu-gcc
	AR=mips-ar9341-linux-gnu-ar
	CFLAGS= -g -O2 -c
else ifeq ($(OS), arm_robustos)
	__SDK_PLATFORM__=$(SDK_PF_ROBUSTOS)
	CC=/opt/ROS-SDK-r3000-3.0.1/staging_dir/target-r3000/arm-sam9x25-linux-gnueabi/bin/arm-sam9x25-linux-gnueabi-gcc
	AR=/opt/ROS-SDK-r3000-3.0.1/staging_dir/target-r3000/arm-sam9x25-linux-gnueabi/bin/arm-sam9x25-linux-gnueabi-ar
	CFLAGS= -g -O2 -c
else ifeq ($(OS), ArmCortexA7)
	__SDK_PLATFORM__=$(SDK_PF_ARM_LINUX)
	CC=arm-openwrt-linux-gcc
	AR=arm-openwrt-linux-ar
	CFLAGS= -g -O2 -c
	SUBDIRS= sdk_impl_linux
else ifeq ($(OS), Arm64Android)
	__SDK_PLATFORM__=$(SDK_PF_ARM_64_LINUX)
	CC=aarch64-linux-android-gcc
	AR=aarch64-linux-android-ar
	CFLAGS= -g -O2 -c
	SUBDIRS= sdk_impl_linux
else ifeq ($(OS), arm_linux)
	__SDK_PLATFORM__=$(SDK_PF_ARM_LINUX)
	CC=gcc
	AR=ar
	CFLAGS= -g -O2 -c
else ifeq ($(OS), arm_linux_raspbian)
	__SDK_PLATFORM__=$(SDK_PF_ARM_LINUX)
	CC=arm-linux-gnueabihf-gcc
	AR=arm-linux-gnueabihf-ar
	CFLAGS= -g -O2 -c
else ifeq ($(OS), arm_64_linux)
	__SDK_PLATFORM__=$(SDK_PF_ARM_64_LINUX)
	CC=arm-linux-gnueabihf-gcc
	AR=arm-linux-gnueabihf-ar
	CFLAGS= -g -O2 -c
	INCDIR=-I../sdk_impl_linux
else ifeq ($(OS), arm_64_linux_aarch64)
	__SDK_PLATFORM__=$(SDK_PF_ARM_64_LINUX)
	CC=aarch64-linux-gnu-gcc
	AR=aarch64-linux-gnu-ar
	CFLAGS= -g -O2 -c
	INCDIR=-I../sdk_impl_linux
else ifeq ($(OS), esp8266_freertos)
	ESP8266_SDK_ROOT=$(SDK_ROOT)/../bin_demos/module/esp8266/ESP8266_RTOS_SDK-2.0.0/
	ifeq ($(__SDK_TYPE__), $(QLY_SDK_LIB))
		__SDK_PLATFORM__=$(SDK_PF_ESP8266_LIB)
	else
		__SDK_PLATFORM__=$(SDK_PF_ESP8266_BIN)
		SUBDIRS+= $(SDK_ROOT)/src/bin_adapter/src/
		SUBDIRS+= $(SDK_ROOT)/src/bin_adapter/src/protocol
		SUBDIRS+= $(SDK_ROOT)/src/bin_adapter/src/module
		INCDIR +=-I$(SDK_ROOT)/src/bin_adapter/include
		INCDIR +=-I$(SDK_ROOT)/src/bin_adapter/include/protocol
		INCDIR +=-I$(SDK_ROOT)/src/bin_adapter/include/module
		INCDIR +=-I$(SDK_ROOT)/src/bin_adapter/include/module/peripheral
		INCDIR +=-I$(SDK_ROOT)/src/bin_adapter/include/spec_define/wifi
		INCDIR +=-I$(SDK_ROOT)/../bin_demos/module/esp8266/ESP8266_RTOS_SDK-2.0.0/iot_sdk_demo/include
	endif

	CC=xtensa-lx106-elf-gcc
	AR=xtensa-lx106-elf-ar
	CFLAGS= -Os -g -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -ffunction-sections -fdata-sections
	INCDIR +=-I${ESP8266_SDK_ROOT}include
	INCDIR +=-I${ESP8266_SDK_ROOT}include/espressif/
	INCDIR +=-I${ESP8266_SDK_ROOT}include/freertos/
	INCDIR +=-I${ESP8266_SDK_ROOT}include/lwip
	INCDIR +=-I${ESP8266_SDK_ROOT}include/lwip/lwip/
	INCDIR +=-I${ESP8266_SDK_ROOT}include/lwip/ipv4/
	INCDIR +=-I${ESP8266_SDK_ROOT}include/lwip/ipv6/
	INCDIR +=-I${ESP8266_SDK_ROOT}include/openssl/
	INCDIR +=-I${ESP8266_SDK_ROOT}extra_include
else ifeq ($(OS), cm3_mico)
	CC=arm-none-eabi-gcc
	AR=arm-none-eabi-ar
	CFLAGS= -g -O2 -c -endian=little -std=c99 -mthumb -mword-relocations -enum_is_int -mcpu=cortex-m3 -fdata-sections
else ifeq ($(OS), cm4_mico)
	__SDK_PLATFORM__=$(SDK_PF_CM4_MICO)
	MICO_SDK_ROOT=/mnt/hgfs/MICO/mico-os/
	CC=arm-none-eabi-gcc
	AR=arm-none-eabi-ar
	CFLAGS= -g -O2 -c -std=c99 -mthumb -mword-relocations -enum_is_int -mcpu=cortex-m4 -ffunction-sections\
       -fdata-sections     \
	   -W                  \
	   -Wall
	SUBDIRS= sdk_impl_mico
	INCDIR=-I../sdk_impl_mico
	INCDIR +=-I${MICO_SDK_ROOT}include/
	INCDIR +=-I${MICO_SDK_ROOT}board/mk3031/
	INCDIR +=-I${MICO_SDK_ROOT}platform/
	INCDIR +=-I${MICO_SDK_ROOT}platform/cortex-m4/
	INCDIR +=-I${MICO_SDK_ROOT}platform/include/
	INCDIR +=-I${MICO_SDK_ROOT}platform/mcu/moc108
	INCDIR +=-I${MICO_SDK_ROOT}mico/security/
	INCDIR +=-I${MICO_SDK_ROOT}mico/system/
	INCDIR +=-I${MICO_SDK_ROOT}libraries/utilities/
	INCDIR +=-I${MICO_SDK_ROOT}sub_build/spi_flash_write/
else ifeq ($(OS), cm4f_mico)
	__SDK_PLATFORM__=$(SDK_PF_CM4F_MICO)
	MICO_SDK_ROOT=/mnt/hgfs/MICO/mico-os/
	CC=arm-none-eabi-gcc
	AR=arm-none-eabi-ar
	CFLAGS= -g -O2 -c -std=c99 -mthumb -mword-relocations -enum_is_int -mcpu=cortex-m4 -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=hard\
       -fdata-sections     \
	   -W                  \
	   -Wall
	SUBDIRS= sdk_impl_mico
	INCDIR +=-I${MICO_SDK_ROOT}include/
	INCDIR +=-I${MICO_SDK_ROOT}board/mk3080B/
	INCDIR +=-I${MICO_SDK_ROOT}platform/
	INCDIR +=-I${MICO_SDK_ROOT}platform/cortex-m4f/
	INCDIR +=-I${MICO_SDK_ROOT}platform/include/
	INCDIR +=-I${MICO_SDK_ROOT}platform/mcu/moc108
	INCDIR +=-I${MICO_SDK_ROOT}mico/security/
	INCDIR +=-I${MICO_SDK_ROOT}mico/system/
	INCDIR +=-I${MICO_SDK_ROOT}libraries/utilities/
	INCDIR +=-I${MICO_SDK_ROOT}sub_build/spi_flash_write/
else ifeq ($(OS), mt7681)
	CC=nds32le-elf-gcc
	AR=nds32le-elf-ar
	CFLAGS= -g -O2 -c
else ifeq ($(OS), qca4004_threadX)
	__SDK_PLATFORM__=$(SDK_PF_QCA4004B)
	QCA4004_SDK_ROOT=/mnt/hgfs/LT1218/LTP1218_360/target/
	CC=xt-xcc
	AR=xt-ar
	SUBDIRS=sdk_impl_qca4004b
	INCDIR+=-I$(FW)/include/AR6002/hw/include
	INCDIR+=-I$(FW)/include/qcom
	INCDIR+=-I$(FW)/include
	INCDIR+=-I$(FW)/include/AR6002/
	INCDIR+=-I$(FW)/include/AR6K/
	INCDIR+=-I$(THREADXDIR)/include/
	INCDIR+=-I$(INTERNALDIR)/include/
	INCDIR+=-I$(INTERNALDIR)/include/os/
	INCDIR+=-I$(INTERNALDIR)/include/whal/
	INCDIR+=-I$(FW)/include/qca
DEFINES := \
	-DAR6K \
	-DAR6002 \
	-TARG:unaligned_loads=1 \
	-DAR6002_REV74 \
	-DIOT_BUILD_FLAG \
	-DAR6002_REV7 \
	-include $(INTERNALDIR)/include/fwconfig_AR6006.h \
	-Wno-return-type \
	-DATHOS \
	-DATH_TARGET \
        -DSWAT_WMICONFIG_MISC_EXT \
        -DSWAT_SSL \
        -DSWAT_I2C \
        -DSWAT_I2S \
        -DSWAT_WMICONFIG_SNTP \
        -DENABLE_HTTP_CLIENT  \
        -DENABLE_HTTP_SERVER  \
        -DSWAT_DNS_ENABLED \
        -DHTTP_ENABLED \
        -DDNS_ENABLED \
        -DBRIDGE_ENABLED \
        -DCONFIG_HOSTLESS \
        -DSSL_ENABLED \
        -DSWAT_BENCH_RAW \
        -DSWAT_WMICONFIG_WPS \
	-DSWAT_OTP
	COPTS= -g3 -Os -Wall -Wpointer-arith -Wundef -fno-inline-functions -nostdlib -mlongcalls -ffunction-sections
	CFLAGS = $(COPTS) $(DEFINES) $(INCDIR)
else ifeq ($(OS), cc3200_freertos)
	__SDK_PLATFORM__=$(SDK_PF_CC3200)
	CC3200_SDK_ROOT=/mnt/hgfs/C/TI/CC3200SDK_1.2.0/cc3200-sdk/
	CC=arm-none-eabi-gcc
	AR=arm-none-eabi-ar
	INCDIR+=-I${CC3200_SDK_ROOT}inc
	INCDIR+=-I${CC3200_SDK_ROOT}oslib
	INCDIR+=-I${CC3200_SDK_ROOT}driverlib
	INCDIR+=-I${CC3200_SDK_ROOT}simplelink
	INCDIR+=-I${CC3200_SDK_ROOT}simplelink/source
	INCDIR+=-I${CC3200_SDK_ROOT}simplelink/include
	INCDIR+=-I${CC3200_SDK_ROOT}third_party/FreeRTOS
	INCDIR+=-I${CC3200_SDK_ROOT}third_party/FreeRTOS/source
	INCDIR+=-I${CC3200_SDK_ROOT}third_party/FreeRTOS/source/portable/GCC/ARM_CM4
	INCDIR+=-I${CC3200_SDK_ROOT}third_party/FreeRTOS/source/include
	INCDIR+=-I${CC3200_SDK_ROOT}simplelink_extlib/provisioninglib
	CFLAGS= -mthumb -mcpu=cortex-m4 -ffunction-sections -fdata-sections -MD -std=c99 -g -O0 -c
else ifeq ($(OS), rnsaS7_threadX)
	__SDK_PLATFORM__=$(SDK_PF_RNSAS7)
	S7G2SK_SDK_ROOT=/mnt/Share/device2.0/device_sdk/example_qly/qly_renesas_s5d9_demo/demo_prj
	CC=arm-none-eabi-gcc
	AR=arm-none-eabi-ar
	CFLAGS= -g -O2 -c -std=c99 -mthumb -mword-relocations -enum_is_int -mcpu=cortex-m4 -ffunction-sections -mfpu=fpv4-sp-d16 -mfloat-abi=hard\
       -fdata-sections     \
	   -W                  \
	   -Wall
	SUBDIRS= sdk_impl_rnsaS7
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy/ssp/inc/framework/el/nxd_application_layer
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy/ssp/inc/framework/el/nxd
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy/ssp/inc/framework/el
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy/ssp/inc/bsp
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy/ssp/inc/bsp/cmsis/include
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy/ssp/inc/driver/api
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy/ssp/inc/driver/instances
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy_cfg/ssp_cfg/framework/el
	INCDIR +=-I${S7G2SK_SDK_ROOT}/synergy_cfg/ssp_cfg/bsp
	INCDIR +=-I${S7G2SK_SDK_ROOT}/src
	INCDIR +=-I${S7G2SK_SDK_ROOT}/src/synergy_gen
else ifeq ($(OS), mt7628_lede)
	__SDK_PLATFORM__=$(SDK_PF_MT7628)
	export STAGING_DIR=/home/qly/
	CC=/home/alvin/K2_LEDE-STABLE-17.01/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-musl-gcc 
	AR=/home/alvin/K2_LEDE-STABLE-17.01/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-ar
	CFLAGS= -g -O2 -c
	#SUBDIRS= sdk_impl_linux
else ifeq ($(OS), hf_lpb120)
	__SDK_PLATFORM__=$(SDK_PF_LPXX2X)
	CROSS_COMPILE ?= arm-none-eabi-
	CROSS ?= $(CROSS_COMPILE)
	CC       = $(CROSS)gcc
	AR       = $(CROSS)ar
## CFLAGS ##
	DISABLE_ICOMM_DISCOVER=0
	MODULE_HIFLYING=5
	MODULE_ID=5
	ifeq ($(MODULE_ID),$(MODULE_HIFLYING))
	LINKERSCRIPT=mc300.lds
	BOOT_CODE_AND_CONFIG_SIZE=16384
	BOOT_CODE_SIZE=8192
	#CONFIG_BIN=
	HFM_LPT120=0
	HFM_LPB120=1
	HFM_LPT220=2
	HFM_LPB125=3
	HFM_SIP120=4
	HFM_LPT120G=5
	endif
	SDK_DIR   =  $(shell pwd)/../demo_hf_lpxx2x/sdk/2.10
	INCLUDE    = -I$(SDK_DIR)/include -I$(SDK_DIR)/include/net -I$(SDK_DIR)/include/bsp -I$(SDK_DIR)/include/net/ieee80211_bss -I$(SDK_DIR)/include/sys -I$(SDK_DIR)/include/net/mac -I$(SDK_DIR)/include/dev
	INCLUDE 	 += -I$(SDK_DIR)/include/hsf/include -I$(SDK_DIR)/include/matrixssl
	__HF_MODULE_ID__ = $(HFM_LPB120)
	DBGFLAGS   = -g
	OPTFLAGS   = -Os -fomit-frame-pointer
	CPPFLAGS  += $(INCLUDE) $(OPTFLAGS) $(DBGFLAGS)
	ARCH_FLAGS = -march=armv3m -mno-thumb-interwork 
	CFLAGS    += -Wall -Wno-trigraphs -fno-builtin $(CPPFLAGS) $(ARCH_FLAGS) -fdata-sections -ffunction-sections -DMODULE_ID=$(MODULE_ID) -DBOOT_SECTOR_UPDATE=$(BOOT_SECTOR_UPDATE) -DDISABLE_ICOMM_DISCOVER=$(DISABLE_ICOMM_DISCOVER) -Werror=implicit-function-declaration -Werror=maybe-uninitialized
	CFLAGS += -DAUTOSTART_ENABLE -D__HF_MODULE_ID__=$(__HF_MODULE_ID__) -D__BUILD_HSF_SDK__
## CFLAGS ##
	SUBDIRS=sdk_impl_lpt125
	OBJS += hf_tcp_impl.o \
			hf_udp_impl.o \
			ql_buffer.o
else ifeq ($(OS), hf_lpb125)
	__SDK_PLATFORM__=$(SDK_PF_LPXX2X)
	CROSS_COMPILE ?= arm-none-eabi-
	CROSS ?= $(CROSS_COMPILE)
	CC       = $(CROSS)gcc
	AR       = $(CROSS)ar
## CFLAGS ##
	DISABLE_ICOMM_DISCOVER=0
	MODULE_HIFLYING=5
	MODULE_ID=5
	ifeq ($(MODULE_ID),$(MODULE_HIFLYING))
	LINKERSCRIPT=mc300.lds
	BOOT_CODE_AND_CONFIG_SIZE=16384
	BOOT_CODE_SIZE=8192
	#CONFIG_BIN=
	HFM_LPT120=0
	HFM_LPB120=1
	HFM_LPT220=2
	HFM_LPB125=3
	HFM_SIP120=4
	HFM_LPT120G=5
	endif
	SDK_DIR   =  $(shell pwd)/example_qly/qly_lpt125_demo/sdk/2.10
	INCLUDE    = -I$(SDK_DIR)/include -I$(SDK_DIR)/include/net -I$(SDK_DIR)/include/bsp -I$(SDK_DIR)/include/net/ieee80211_bss -I$(SDK_DIR)/include/sys -I$(SDK_DIR)/include/net/mac -I$(SDK_DIR)/include/dev
	INCLUDE 	 += -I$(SDK_DIR)/include/hsf/include -I$(SDK_DIR)/include/matrixssl
	__HF_MODULE_ID__ = $(HFM_LPB125)
	DBGFLAGS   = -g
	OPTFLAGS   = -Os -fomit-frame-pointer
	CPPFLAGS  += $(INCLUDE) $(OPTFLAGS) $(DBGFLAGS)
	ARCH_FLAGS = -march=armv3m -mno-thumb-interwork 
	CFLAGS    += -Wall -Wno-trigraphs -fno-builtin $(CPPFLAGS) $(ARCH_FLAGS) -fdata-sections -ffunction-sections -DMODULE_ID=$(MODULE_ID) -DBOOT_SECTOR_UPDATE=$(BOOT_SECTOR_UPDATE) -DDISABLE_ICOMM_DISCOVER=$(DISABLE_ICOMM_DISCOVER) -Werror=implicit-function-declaration -Werror=maybe-uninitialized
	CFLAGS += -DAUTOSTART_ENABLE -D__HF_MODULE_ID__=$(__HF_MODULE_ID__) -D__BUILD_HSF_SDK__
## CFLAGS ##
	SUBDIRS=sdk_impl_lpt125
	OBJS += hf_tcp_impl.o \
			hf_udp_impl.o \
			ql_buffer.o
else ifeq ($(OS), esp32) 
	__SDK_PLATFORM__=$(SDK_PF_ESP32_LIB)

	ESP32_SDK_ROOT= $(IDF_PATH)
	CC=xtensa-esp32-elf-gcc
	AR=xtensa-esp32-elf-ar

	CFLAGS = -std=gnu99 -Og -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib -Wno-error=unused-function -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-error=deprecated-declarations -Wno-unused-parameter -Wno-sign-compare -Wno-old-style-declaration -Wno-deprecated-declarations 

	CFLAGS = -std=gnu99 -Og -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib -Wno-error=unused-function -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-error=deprecated-declarations -Wno-unused-parameter -Wno-sign-compare -Wno-old-style-declaration -Wno-deprecated-declarations
	INCDIR +=-I${ESP32_SDK_ROOT}/components/freertos
	INCDIR +=-I${ESP32_SDK_ROOT}/components/freertos/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/freertos/include/freertos
	INCDIR +=-I${ESP32_SDK_ROOT}/components/lwip/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/lwip/include/lwip
	INCDIR +=-I${ESP32_SDK_ROOT}/components/lwip/include/lwip/apps
	INCDIR +=-I${ESP32_SDK_ROOT}/components/lwip/include/lwip/netif
	INCDIR +=-I${ESP32_SDK_ROOT}/components/lwip/include/lwip/port
	INCDIR +=-I${ESP32_SDK_ROOT}/components/lwip/include/lwip/posix
	INCDIR +=-I${ESP32_SDK_ROOT}/components/lwip/include/lwip/lwip
	INCDIR +=-I${ESP32_SDK_ROOT}/components/openssl/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/openssl/include/internal
	INCDIR +=-I${ESP32_SDK_ROOT}/components/openssl/include/openssl
	INCDIR +=-I${ESP32_SDK_ROOT}/components/openssl/include/platform
	INCDIR +=-I${ESP32_SDK_ROOT}/components/spi_flash/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/esp32/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/esp32/include/xtensa/config
	INCDIR +=-I${ESP32_SDK_ROOT}/components/soc/esp32/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/soc/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/heap/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/driver/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/tcpip_adapter/include
	INCDIR +=-I${ESP32_SDK_ROOT}/components/vfs/include				
	INCDIR +=-I${ESP32_SDK_ROOT}/components/newlib/platform_include

	#下面路径为make menucofngi 生成得 sdkconfig.h路径#
	INCDIR +=-I${ESP32_SDK_ROOT}/hello_world/build/include
else ifeq ($(OS), Trifo_android-ndk-r17c_standalone)
	__SDK_PLATFORM__=$(SDK_PF_TRIFO_NDK_R17C)
	CC=aarch64-linux-android-gcc
	AR=aarch64-linux-android-ar
	CFLAGS= -g -O2 -c -fPIE -pie
else

err:
	@echo "platform error!"
endif

PF=$(__SDK_PLATFORM__)

ifeq ($(NEED_LAN_DETECT),yes)
	CFLAGS+=-DLAN_DETECT
endif

ifeq ($(NEED_LOCAL_SAVE),yes)
	CFLAGS+=-DLOCAL_SAVE
endif

ifeq ($(IS_SUPPORT_REBOOT_OTA),yes)
	CFLAGS+=-DSUPPORT_REBOOT_OTA
endif

ifeq ($(NEED_DEBUG),yes)
	CFLAGS+=-DDEBUG_VERSION
	DBG=_debug
endif

OBJS_DIR = $(SDK_ROOT)/sdk_objs
SUBDIRS = $(SDK_ROOT)/platforms $(SDK_ROOT)/thirdparty/ql_public $(SDK_ROOT)/thirdparty/mqtt $(SDK_ROOT)/thirdparty/mbedtls/library $(SDK_ROOT)/src $(SDK_ROOT)/src/ql_patch $(SDK_ROOT)/sdk_objs
#SUBDIRS+= $(SDK_ROOT)/platforms $(SDK_ROOT)/thirdparty/ql_public $(SDK_ROOT)/thirdparty/mqtt $(SDK_ROOT)/src $(SDK_ROOT)/src/ql_patch $(SDK_ROOT)/sdk_objs

CFLAGS+=-D__SDK_TYPE__=$(__SDK_TYPE__) -D__SDK_PLATFORM__=$(PF)
TARGET=libiot_$(OS)$(DBG).a
INCDIR+=-I$(SDK_ROOT)/platforms/include -I$(SDK_ROOT)/src -I$(SDK_ROOT)/src/ql_patch -I$(SDK_ROOT)/thirdparty/ql_public -I$(SDK_ROOT)/thirdparty/mqtt -I$(SDK_ROOT)/thirdparty/mbedtls/include -I$(SDK_ROOT)/../include
LIBDIR=$(SDK_ROOT)/../lib

export CC AR SDK_ROOT OS CFLAGS INCDIR TARGET OBJS_DIR LIBDIR

all : $(SUBDIRS)
$(SUBDIRS) :ECHO 
	make -C $@
ECHO:
	@echo $(SUBDIRS)

clean:
	rm -f $(OBJS_DIR)/*.o
	rm -f sdk_public/*.o
	for tmp in `echo $(SUBDIRS)`; \
	do \
		rm -f "$$tmp"/*.d; \
		rm -f "$$tmp"/*.o; \
		rm -f "$$tmp"/*.a; \
	done
	rm -f $(LIBDIR)/$(TARGET)
	rm -f $(LIBDIR)/libmbed*.a
	rm -f $(LIBDIR)/libiotpatch.a
