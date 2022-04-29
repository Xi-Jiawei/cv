#!/bin/sh

for i in $(seq 1 10)
do
    idx=$(printf "%02d" $i)
    mv "${idx}_bayer_debayer.tif" "${idx}_demosaic_gradient.tif"
    echo "mv ${idx}_bayer_debayer.tif ${idx}_demosaic_gradient.tif"
done

