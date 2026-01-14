#!/usr/bin/env bash
set -euo pipefail

sudo apt update
sudo apt install -y build-essential cmake pkg-config libgpiod-dev libfreetype6-dev fonts-dejavu-core

mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

sudo install -d -m 0755 /opt/nhd12864
sudo install -m 0755 build/nhd12864_demo /opt/nhd12864/
sudo install -m 0755 build/nhd12864_gpio /opt/nhd12864/

sudo install -d -m 0755 /etc/nhd12864
sudo install -m 0644 scripts/nhd12864.conf /etc/nhd12864/nhd12864.conf

sudo install -m 0644 systemd/nhd12864-demo.service /etc/systemd/system/nhd12864-demo.service
sudo systemctl daemon-reload

echo "Edit /etc/nhd12864/nhd12864.conf to match your spidev + GPIO line offsets."
echo "Then enable:"
echo "  sudo systemctl enable --now nhd12864-demo.service"
