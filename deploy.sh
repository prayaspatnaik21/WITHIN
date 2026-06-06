#!/bin/bash

set -e

REMOTE_USER="pattu"
REMOTE_IP="192.168.55.1"
REMOTE_PATH="/home/pattu/workspace/WITHIN/"
LOCAL_PATH="/home/pattu/workspace/jetson/WITHIN/"

echo "Syncing project to Jetson..."

rsync -avh --progress \
    --delete \
    $LOCAL_PATH \
    ${REMOTE_USER}@${REMOTE_IP}:${REMOTE_PATH}

echo "Building on remote..."

ssh -t ${REMOTE_USER}@${REMOTE_IP} "
cd /home/pattu/workspace/WITHIN &&
mkdir -p build &&
cd build &&
cmake .. -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc &&
make -j4
"

echo "Done!"