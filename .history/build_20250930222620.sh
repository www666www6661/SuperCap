#!/bin/bash

# ==============================================================================
# RM2024 超级电容控制器固件编译脚本
#
# 该脚本会执行以下操作:
# 1. 清理上一次的编译产物，确保一个干净的编译环境。
# 2. 使用多线程（8线程）并行编译，以加快编译速度。
#
# 使用前请确保:
# - 已安装 'make' 工具。
# - 已安装 'arm-none-eabi-gcc' 工具链，并将其添加到了系统 PATH 环境变量中。
# ==============================================================================

# 当任何命令执行失败时，立即退出脚本
set -e

# --- 开始编译 ---
echo ">>> [Step 1/2] Cleaning previous build artifacts..."

# 执行清理命令，删除 build/ 目录下的所有旧文件
make clean

echo ">>> Clean complete."
echo ""
echo ">>> [Step 2/2] Starting firmware compilation with 8 parallel jobs..."

# 执行编译命令，-j8 表示使用8个核心并行编译
# 如果您的CPU核心数不同，可以调整这个数字，例如 -j16
make -j8 -DHARDWARE_ID=-1

echo ""
echo ">>> Build finished successfully!"
echo ">>> The firmware file (.bin/.hex) is located in the 'build' directory."
echo ""
