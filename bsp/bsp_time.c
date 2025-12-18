#include "bsp_time.h"
#include "tim.h"

#define BSP_CLOCK_FREQ_MHZ 72

// 记录 TIM16 溢出次数 (扩展为64位)
static volatile uint64_t tim16_overflow_cnt = 0;

/**
 * @brief 初始化时间基准
 */
void bsp_time_init(void) {
  // 启动 TIM16 并开启溢出中断
  HAL_TIM_Base_Start_IT(&htim16);
}

/**
 * @brief TIM16 溢出中断回调
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM16) {
    tim16_overflow_cnt++;
  }
}

/**
 * @brief 获取微秒数 (核心实现)
 * @return uint64_t
 */
uint64_t bsp_time_get_us(void) {
  uint32_t cnt;
  uint64_t ovf_first, ovf_second;

  /*
   * 原子读取保护：
   * 循环读取直到前后两次溢出次数一致，确保读取期间没有发生中断。
   * 这是一个极低概率但致命的 Bug 修复。
   */
  do {
    ovf_first = tim16_overflow_cnt;
    cnt = __HAL_TIM_GET_COUNTER(&htim16);
    ovf_second = tim16_overflow_cnt;
  } while (ovf_first != ovf_second);

  // 拼接成总 Ticks (TIM16 是 16 位的，所以左移 16)
  uint64_t total_ticks = (ovf_first << 16) | cnt;

  // 转换为微秒
  // 因为 BSP_CLOCK_FREQ_MHZ 是常量 72，这里会被编译器极度优化
  return total_ticks / BSP_CLOCK_FREQ_MHZ;
}

/**
 * @brief 获取毫秒数
 */
uint32_t bsp_time_get_ms(void) { return (uint32_t)(bsp_time_get_us() / 1000); }

// 别名定义
uint64_t bsp_time_get() __attribute__((alias("bsp_time_get_us")));