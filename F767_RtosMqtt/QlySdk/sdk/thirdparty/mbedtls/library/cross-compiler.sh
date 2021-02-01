#!/bin/sh

if [ "$1" = "" ]; then
    echo "USAGE: $0 [arch]"
    echo "x86_linux                 gcc"
    echo "x86_64_linux              gcc"
    echo "arm_linux_raspbian        arm-linux-gnueabihf-gcc"
    echo "arm_64_linux_aarch64      aarch64-linux-gnu-gcc"
    echo "mt7628_lede               lede"
    exit 1;
fi

if [ "$1" = "x86_linux" ]; then
    echo "cross compiler x86_linux"
    export CC=gcc
    export AR=ar
fi

if [ "$1" = "x86_64_linux" ]; then
    echo "cross compiler x86_64_linux"
    export CC=gcc
    export AR=ar
fi

if [ "$1" = "arm_linux_raspbian" ]; then
    echo "cross compiler arm_linux_raspbian"   
    export CC=arm-linux-gnueabihf-gcc
    export AR=arm-linux-gnueabihf-ar
fi

if [ "$1" = "arm_64_linux_aarch64" ]; then
    echo "cross compiler arm_64_linux_aarch64"  
    export CC=aarch64-linux-gnu-gcc
    export AR=aarch64-linux-gnu-ar
fi

if [ "$1" = "mt7628_lede" ]; then
    export STAGING_DIR=/home/alvin/
    echo "cross compiler mt7628_lede"  
    export CC=/home/alvin/K2_LEDE-STABLE-17.01/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-musl-gcc 
    export AR=/home/alvin/K2_LEDE-STABLE-17.01/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-ar	
fi

echo "$(pwd)"

make clean

make
