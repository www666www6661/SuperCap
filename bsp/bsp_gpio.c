#include "bsp_gpio.h"

#include "main.h"
#include "stm32f3xx_hal_gpio.h"

typedef struct {
  uint16_t pin;
  GPIO_TypeDef *gpio;
} bsp_gpio_map_t;

static const bsp_gpio_map_t bsp_gpio_map[BSP_GPIO_NUM] = {
    [BSP_GPIO_LED1] = {GPIO_PIN_14, GPIOB},
    [BSP_GPIO_LED2] = {GPIO_PIN_15, GPIOB}};

inline bsp_status_t bsp_gpio_write_pin(bsp_gpio_t gpio, bool value) {

  HAL_GPIO_WritePin(bsp_gpio_map[gpio].gpio, bsp_gpio_map[gpio].pin, value);
  return BSP_OK;
}

inline bsp_status_t bsp_gpio_toggle_pin(bsp_gpio_t gpio) {
  HAL_GPIO_TogglePin(bsp_gpio_map[gpio].gpio, bsp_gpio_map[gpio].pin);
  return BSP_OK;
}

inline bool bsp_gpio_read_pin(bsp_gpio_t gpio) {
  return HAL_GPIO_ReadPin(bsp_gpio_map[gpio].gpio, bsp_gpio_map[gpio].pin);
}