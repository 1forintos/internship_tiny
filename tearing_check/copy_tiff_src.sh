#!/bin/bash
if [ $# -gt 0 ]
then
mkdir -p tiffloader
rm -f tiffloader/*
cp $1/*.c $1/*.h tiffloader/
else
echo "Missing input directory!"
exit -1
fi
