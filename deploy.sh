#!/bin/bash

set -e

REMOTE_USER="pattu"
REMOTE_IP="192.168.55.1"
REMOTE_PATH="/home/pattu/workspace/WITHIN/"
LOCAL_PATH="/Users/prayaspatnaik/opt/anaconda3/ProjectMe/WITHIN/"

echo "Syncing project to Jetson..."

rsync -avh --progress \
    --delete \
    $LOCAL_PATH \
    ${REMOTE_USER}@${REMOTE_IP}:${REMOTE_PATH}

echo "Building and running on remote..."

ssh -t ${REMOTE_USER}@${REMOTE_IP} << 'EOF'
set -e

cd /home/pattu/workspace/WITHIN

mkdir -p build
cd build

echo "Running CMake..."
cmake .. -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc

echo "Building..."
make -j4

echo "Build successful!"

# Set display for GUI apps
export DISPLAY=:0

echo "Launching application..."
./cam

EOF

echo "Done!"