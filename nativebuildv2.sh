
export PATH=$PATH:'/home/dosgo/OpenWrt-SDK/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin'
export STAGING_DIR="/home/dosgo/OpenWrt-SDK/staging_dir"
mkdir build
rm -rf build/cping..o
rm -rf build/main.o
rm -rf build/dd.o

CC=g++
YH="-Wall   -fexceptions -O2 -DOPENSSL=1 -DOPENSSLDL=1"
$CC $YH -c  cping.cpp -o build/cping.o
$CC $YH -c  main.cpp -o build/main.o
$CC $YH -c  dd.cpp -o build/dd.o

$CC  -s build/main.o build/cping.o build/dd.o  -o build/smarthome 








