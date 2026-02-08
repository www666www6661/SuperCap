#!/bin/bash

# 检查是否安装了 awk (通常 Linux/Mac 默认都有)
if ! command -v awk &> /dev/null; then
    echo "错误: 未找到 awk 工具。请先安装它。"
    exit 1
fi

echo "==============================================="
echo "      STM32 ADC 线性校准计算器 (y = kx + b)"
echo "      公式: Real_Current = ADC_Val * K + B"
echo "==============================================="
echo ""

# --- 第一点输入 (通常是 0A 或者低电流) ---
echo ">>> 请输入第 1 个点的数据 (推荐：零电流点)"
read -p "   ADC 原始数值 (x1): " adc1
read -p "   真实物理数值 (y1): " real1

echo ""

# --- 第二点输入 (通常是负载电流) ---
echo ">>> 请输入第 2 个点的数据 (推荐：最大电流点)"
read -p "   ADC 原始数值 (x2): " adc2
read -p "   真实物理数值 (y2): " real2

echo ""
echo "-----------------------------------------------"
echo "正在计算..."

# 检查分母是否为 0
if [ "$adc1" == "$adc2" ]; then
    echo "错误: 两个点的 ADC 数值不能相同，无法计算斜率！"
    exit 1
fi

# 使用 awk 进行浮点运算
# K = (y2 - y1) / (x2 - x1)
# B = y1 - (K * x1)

awk -v x1="$adc1" -v y1="$real1" -v x2="$adc2" -v y2="$real2" '
BEGIN {
    # 计算斜率 K
    k = (y2 - y1) / (x2 - x1);
    
    # 计算截距 B
    b = y1 - (k * x1);
    
    # 输出结果
    print "计算结果:";
    printf "   K (Slope)     = %.8f\n", k;
    printf "   B (Intercept) = %.8f\n", b;
    print "";
    print ">>> C代码宏定义建议 (可以直接复制):";
    print "---------------------------------------";
    printf "#define ADC_IA_K  (%.8ff)\n", k;
    printf "#define ADC_IA_B  (%.8ff)\n", b;
    print "---------------------------------------";
    
    # 简单的验证
    print "";
    print "验证公式:";
    printf "   输入 ADC=%s -> 算出 %.4f (应为 %s)\n", x1, (x1 * k + b), y1;
    printf "   输入 ADC=%s -> 算出 %.4f (应为 %s)\n", x2, (x2 * k + b), y2;
}'

echo "==============================================="
