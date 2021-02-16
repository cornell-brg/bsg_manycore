#!/usr/bin/sh
#=========================================================================
# test_runner.sh
#=========================================================================
# This script takes one test name argument, one BSG_MACHINE_PATH argument,
# and runs that test case in directory ../$test_name/. We have to do this
# to be compliant to the spmd build system.
#
# Author : Peitian Pan
# Date   : Feb 16, 2021

# Helper variables
main_cgra_test_dir=$(pwd)
sub_cgra_test_dir=$(pwd)/../cgra_test_$1

# Make sure the directory has been created
cd ..
mkdir -p $sub_cgra_test_dir
mkdir -p $sub_cgra_test_dir/cgra_test_vectors

# Copy over source files
cp -f $main_cgra_test_dir/Makefile $sub_cgra_test_dir/Makefile
cp -f $main_cgra_test_dir/main.c $sub_cgra_test_dir/main.c
cp -f $main_cgra_test_dir/cgra_xcel_hb_tapeout.h $sub_cgra_test_dir/cgra_xcel_hb_tapeout.h
cp -f $main_cgra_test_dir/register_tests.py $sub_cgra_test_dir/register_tests.py

# Copy over the desired test vector data file
cp -f $main_cgra_test_dir/cgra_test_vectors/cgra_tv_$1.dat $sub_cgra_test_dir/cgra_test_vectors/cgra_tv_$1.dat

# Generate test vector header
cd $sub_cgra_test_dir
./register_tests.py

# Run the test case
# We don't make change to RTL or testbenches -- hopefully VCS doesn't have
# to recompile the simulator.
make CGRA_TEST=$1 BSG_MACHINE_PATH=$2
