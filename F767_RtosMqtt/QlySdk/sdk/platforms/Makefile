
ifeq ($(OS), x86_linux)
	PFDIR= src/linux
else ifeq ($(OS), x86_64_linux)
	PFDIR= src/linux
else ifeq ($(OS), mips_robustos)
	PFDIR= src/linux
else ifeq ($(OS), arm_robustos)
	PFDIR= src/linux
else ifeq ($(OS), ArmCortexA7)
	PFDIR= sdk_impl_linux
else ifeq ($(OS), Arm64Android)
	PFDIR= sdk_impl_linux
else ifeq ($(OS), arm_linux)
	PFDIR= src/linux
else ifeq ($(OS), arm_linux_raspbian)
	PFDIR= src/linux
else ifeq ($(OS), arm_64_linux)
	PFDIR= sdk_impl_linux
else ifeq ($(OS), arm_64_linux_aarch64)
	PFDIR= src/linux
else ifeq ($(OS), esp8266_freertos)
	PFDIR= src/esp8266_rtos
else ifeq ($(OS), cm3_mico)
	PFDIR= sdk_impl_mico
else ifeq ($(OS), cm4_mico)
	PFDIR= sdk_impl_mico
else ifeq ($(OS), cm4f_mico)
	PFDIR= sdk_impl_mico
else ifeq ($(OS), mt7681)
	PFDIR=sdk_impl_qca4004b
else ifeq ($(OS), qca4004_threadX)
	PFDIR=sdk_impl_qca4004b
else ifeq ($(OS), cc3200_freertos)
	PFDIR=sdk_impl_cc3200
else ifeq ($(OS), rnsaS7_threadX)
	PFDIR= sdk_impl_rnsaS7
else ifeq ($(OS), mt7628_lede)
	PFDIR= src/linux
else ifeq ($(OS), hf_lpb120)
	PFDIR=src/hf_lpxx2x
else ifeq ($(OS), hf_lpb125)
	PFDIR=src/hf_lpxx2x
else ifeq ($(OS), Trifo_android-ndk-r17c_standalone)
	PFDIR=src/linux
else
err:
	@echo "platform error!"
endif

SOURCES:= $(wildcard $(PFDIR)/*.c)

all:$(SOURCES)
	$(CC) $(INCDIR) $(CFLAGS) -c $^
	mv *.o $(OBJS_DIR)/