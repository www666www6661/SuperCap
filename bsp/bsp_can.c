#include "bsp_can.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_can.h"
#include <string.h>

#define CAN_DEV &hcan

extern CAN_HandleTypeDef hcan;

typedef struct {
  CAN_TxHeaderTypeDef header;
  uint8_t data[8];
} can_raw_tx_t;

typedef struct {
  CAN_RxHeaderTypeDef header;
  uint8_t data[8];
} can_raw_rx_t;

void bsp_can_init(void) {

  CAN_FilterTypeDef can_filter = {0};

  can_filter.FilterBank = 0;
  can_filter.FilterIdHigh = 0x010 << 5;
  can_filter.FilterIdLow = 0;
  can_filter.FilterMode = CAN_FILTERMODE_IDLIST;
  can_filter.FilterScale = CAN_FILTERSCALE_16BIT;
  can_filter.FilterMaskIdHigh = 0;
  can_filter.FilterMaskIdLow = 0;
  can_filter.FilterActivation = ENABLE;
  can_filter.SlaveStartFilterBank = 0;
  can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  HAL_CAN_ConfigFilter(CAN_DEV, &can_filter);
  __HAL_CAN_ENABLE_IT(CAN_DEV, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_Start(CAN_DEV);
}

bsp_status_t bsp_can_trans_packet(bsp_can_t can, bsp_can_format_t format,
                                  /*uint32_t id,*/ uint8_t *data) {
  CAN_TxHeaderTypeDef header;
  header.StdId = 0x051;
  header.ExtId = 0;
  header.IDE = CAN_ID_STD;
  header.RTR = CAN_RTR_DATA;
  header.DLC = 8;
  header.TransmitGlobalTime = DISABLE;

  uint32_t txMailBox = 0;
  HAL_CAN_AddTxMessage(CAN_DEV, &header, data, &txMailBox);
  //   hcan.Instance->sTxMailBox[CAN_TX_MAILBOX0].TIR  = ((txHeader.StdId <<
  //   CAN_TI0R_STID_Pos) | txHeader.RTR);
  // hcan.Instance->sTxMailBox[CAN_TX_MAILBOX0].TDTR = (txHeader.DLC);
  // WRITE_REG(hcan.Instance->sTxMailBox[CAN_TX_MAILBOX0].TDHR,
  //           ((uint32_t)(((uint8_t *)&txData)[7]) << CAN_TDH0R_DATA7_Pos) |
  //           ((uint32_t)(((uint8_t *)&txData)[6]) << CAN_TDH0R_DATA6_Pos) |
  //               ((uint32_t)(((uint8_t *)&txData)[5]) << CAN_TDH0R_DATA5_Pos)
  //               | ((uint32_t)(((uint8_t *)&txData)[4]) <<
  //               CAN_TDH0R_DATA4_Pos));
  // WRITE_REG(hcan.Instance->sTxMailBox[CAN_TX_MAILBOX0].TDLR,
  //           ((uint32_t)(((uint8_t *)&txData)[3]) << CAN_TDL0R_DATA3_Pos) |
  //           ((uint32_t)(((uint8_t *)&txData)[2]) << CAN_TDL0R_DATA2_Pos) |
  //               ((uint32_t)(((uint8_t *)&txData)[1]) << CAN_TDL0R_DATA1_Pos)
  //               | ((uint32_t)(((uint8_t *)&txData)[0]) <<
  //               CAN_TDL0R_DATA0_Pos));
  // SET_BIT(hcan.Instance->sTxMailBox[CAN_TX_MAILBOX0].TIR,
  // CAN_TI0R_TXRQ);der.StdId;

  return BSP_OK;
}

bsp_status_t bsp_can_get_msg(uint8_t *data, uint32_t *index) {
  can_raw_rx_t rx = {};

  if (HAL_CAN_GetRxMessage(CAN_DEV, CAN_RX_FIFO0, &rx.header, rx.data) ==
      HAL_OK) {
    *index = rx.header.StdId;
    memcpy(data, rx.data, sizeof(rx.data));
    return BSP_OK;
  }

  return BSP_ERR;
}

uint32_t id = 0;
uint8_t data[8] = {1, 1, 1, 1, 1, 1, 1, 1};
void CAN_RX0_IRQHandler(void) {
  if (bsp_can_get_msg(data, &id) == BSP_OK) {
    for (int i = 0; i < 5; i++)
      data[0] = i;
    bsp_can_trans_packet(BSP_CAN_2, CAN_FORMAT_STD_DATA, data);
  }
}
