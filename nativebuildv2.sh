
export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
mkdir build
rm -rf build/cping..o
rm -rf build/main.o
rm -rf build/dd.o
rm -rf build/freearp.o
rm -rf build/args.o

CC=gcc
CP=g++
YH="-Wall   -fexceptions -O2 -DOPENSSL=1 -DOPENSSLDL=1"
$CP $YH -c  cping.cpp -o build/cping.o
$CP $YH -c  main.cpp -o build/main.o
$CP $YH -c  freearp.cpp -o build/freearp.o
$CC $YH -c  args.c -o build/args.o
$CP  -s build/main.o build/cping.o build/dd.o build/freearp.o build/args.o -o build/smarthome 








