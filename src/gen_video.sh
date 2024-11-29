#!/bin/bash
scene=$1
end=$2
make
mkdir -p video/$scene
./render $scene -b 0:$end -f video/$scene/frame
ffmpeg -r 30 -i video/$scene/frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p video/$scene.mp4 -y
rm -rf video/$scene
