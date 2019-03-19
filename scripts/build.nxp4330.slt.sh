#!/bin/bash

BASEDIR="$(cd "$(dirname "$0")" && pwd)/../.."
RESULT="$BASEDIR/result"

# Toolchains for Bootloader and Linux
LINUX_TOOLCHAIN="$BASEDIR/characterization/crosstool/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-"
BL_TOOLCHAIN="$BASEDIR/characterization/crosstool/arm-eabi-4.8/bin/arm-eabi-"
# Build PATH
BR2_DIR=$BASEDIR/buildroot
UBOOT_DIR=$BASEDIR/u-boot-2016.01
KERNEL_DIR=$BASEDIR/kernel-4.4
BL1_DIR="$BASEDIR/2ndboot/bl1-s5p4418"
BL2_DIR="$BASEDIR/2ndboot/bl2-s5p4418"
DISP_DIR="$BASEDIR/2ndboot/armv7-dispatcher"
BIN_DIR="$BASEDIR/characterization/bin"
FILES_DIR="$BASEDIR/characterization/files"
BINARY_DIR="$BASEDIR/characterization/firmwares"
MAKE_ENV="$BASEDIR/tools/mkenvimage"
SLT_DIR="$BASEDIR/characterization/slt/"
ASB_TARGET="$RESULT/rootfs/usr/bin/"

# build option
BL1_OPT="CHIPNAME=nxp4330 BOARD=slt DEVICE_PORT=0 KERNEL_VER=y ARM_SECURE=n SYSLOG=y"
DISP_OPT="CHIPNAME=nxp4330 BOARD=slt DEVICE_PORT=0 KERNEL_VER=y ARM_SECURE=n SYSLOG=y"
# FILES
UBOOT_NSIH="$FILES_DIR/nsih_uboot.txt"
BOOT_KEY="$FILES_DIR/bootkey"
USER_KEY="$FILES_DIR/userkey"
BINGEN_EXE="$BIN_DIR/bingen"
MAKE_DISK_IMG="$BASEDIR/characterization/scripts/partmap_diskimg.sh"
PARTMAP="$BASEDIR/characterization/files/partmap_sd.txt"

# BINGEN
UBOOT_BINGEN="$BIN_DIR/SECURE_BINGEN -c nxp4330 -t 3rdboot -i $RESULT/u-boot.bin
		-l 0x74c00000 -e 0x74c00000 -o $RESULT/bootloader.img"

BL2_BINGEN="$BIN_DIR/SECURE_BINGEN -c nxp4330 -t 3rdboot -i $BL2_DIR/out/pyrope-bl2.bin
		-l 0xb0fe0000 -e 0xb0fe0400 -o $RESULT/loader-emmc.img
		-m 0x40200 -b 3 -p 0 -m 0x1E0200 -b 3 -p 0 -m 0x60200 -b 3"

DISP_BINGEN="$BIN_DIR/SECURE_BINGEN -c nxp4330 -t 3rdboot -i $DISP_DIR/out/armv7_dispatcher.bin
		-l 0xffff0200 -e 0xffff0200 -o $RESULT/bl_mon.img
		-m 0x40200 -b 3 -p 0 -m 0x1E0200 -b 3 -p 0 -m 0x60200 -b 3"


# Images BUILD
MAKE_EXT4FS_EXE="$BASEDIR/characterization/bin/make_ext4fs"

MAKE_BOOTIMG="dd if=/dev/zero of=$RESULT/boot.img bs=1M count=1000;\
		mkfs.vfat $RESULT/boot.img;\
		mkdir $RESULT/mnt;\
		sudo mount $RESULT/boot.img $RESULT/mnt;
		sudo cp $RESULT/zImage $RESULT/mnt; \
		sudo cp $RESULT/*dtb $RESULT/mnt; \
		sudo umount $RESULT/mnt;"

MAKE_ROOTIMG="mkdir -p $RESULT/root; \
		$MAKE_EXT4FS_EXE -b 4096 -L root -l 1073741824 $RESULT/root.img $RESULT/root/"

MAKE_DISK="$MAKE_DISK_IMG -f $PARTMAP -s 1 -r 0"

SLT_COPY="cp ./characterization/slt/script/S60test result/rootfs/etc/init.d/"


# Build Targets
BUILD_IMAGES=(
	"MACHINE= nxp4330",
	"ARCH	= arm",
	"TOOL	= $LINUX_TOOLCHAIN",
	"RESUL	= $RESULT",
	"bl1	=
		JOBS	: 1,
		PATH	: $BL1_DIR,
		TOOL	: $BL_TOOLCHAIN,
		OPTION	: $BL1_OPT,
		OUTPUT	: out/bl1-emmcboot.bin",
	"bl2	=
		JOBS	: 1,
		TOOL	: $BL_TOOLCHAIN,
		PATH	: $BL2_DIR,
		POSTCMD	: $BL2_BINGEN",
	"disp	=
		JOBS	: 1,
		TOOL	: $BL_TOOLCHAIN,
		OPTION	: $DISP_OPT,
		POSTCMD	: $DISP_BINGEN,
		PATH	: $DISP_DIR",
	"uboot	=
		PATH	: $UBOOT_DIR,
		CONFIG	: nxp4330_slt_defconfig,
		OUTPUT	: u-boot.bin,
		POSTCMD	: $UBOOT_BINGEN",
	"br2	=
		PATH	: $BR2_DIR,
		CONFIG	: nxp4330_slt_defconfig,
		OUTPUT	: output/target,
		COPY	: rootfs",
	"slt	=
		PATH	: $SLT_DIR/src,
		OUTPUT	: ../bin/,
		COPY	: rootfs/usr/local/,
		POSTCMD	: $SLT_COPY",
	"kernel	=
		PATH	: $KERNEL_DIR,
		CONFIG	: nxp4330_slt_defconfig,
		IMAGE	: zImage,
		OUTPUT	: arch/arm/boot/zImage",
	"dtb	=
		PATH	: $KERNEL_DIR,
		IMAGE	: nxp4330-slt.dtb,
		OUTPUT	: arch/arm/boot/dts/nxp4330-slt.dtb",
	"boot	=
		POSTCMD	: $MAKE_BOOTIMG",
	"disk	=
		POSTCMD	: $MAKE_DISK",

)
