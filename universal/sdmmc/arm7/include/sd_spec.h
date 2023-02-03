#pragma once

// SPDX-License-Identifier: MIT

// Based on SD specification version 8.00.

#include "tmio.h"


// Controller specific macros. Add controller specific bits here.
// SD_[command type]_[response type]_[transfer type]
// Command type: CMD = regular command, ACMD = Application-Specific Command.
// Transfer type: R = read, W = write.
#define SD_CMD_NONE(id)   (CMD_RESP_NONE | (id))
#define SD_CMD_R1(id)     (CMD_RESP_R1   | (id))
#define SD_CMD_R1b(id)    (CMD_RESP_R1b  | (id))
#define SD_CMD_R2(id)     (CMD_RESP_R2   | (id))
#define SD_CMD_R6(id)     (CMD_RESP_R6   | (id))
#define SD_CMD_R7(id)     (CMD_RESP_R7   | (id))
#define SD_CMD_R1_R(id)   (CMD_DIR_R | CMD_DT_EN | CMD_RESP_R1 | (id))
#define SD_CMD_R1_W(id)   (CMD_DIR_W | CMD_DT_EN | CMD_RESP_R1 | (id))
#define SD_ACMD_R1(id)    (CMD_RESP_R1 | CMD_ACMD | (id))
#define SD_ACMD_R3(id)    (CMD_RESP_R3 | CMD_ACMD | (id))
#define SD_ACMD_R1_R(id)  (CMD_DIR_R | CMD_DT_EN | CMD_RESP_R1 | CMD_ACMD | (id))


// Basic Commands (class 0).
#define SD_GO_IDLE_STATE                 SD_CMD_NONE(0u) //   -, [31:0] stuff bits.
#define SD_ALL_SEND_CID                    SD_CMD_R2(2u) //  R2, [31:0] stuff bits.
#define SD_SEND_RELATIVE_ADDR              SD_CMD_R6(3u) //  R6, [31:0] stuff bits.
#define SD_SET_DSR                       SD_CMD_NONE(4u) //   -, [31:16] DSR [15:0] stuff bits.
#define SD_SELECT_CARD                    SD_CMD_R1b(7u) // R1b, [31:16] RCA [15:0] stuff bits.
#define SD_DESELECT_CARD                 SD_CMD_NONE(7u) //   -, [31:16] RCA [15:0] stuff bits.
#define SD_SEND_IF_COND                    SD_CMD_R7(8u) //  R7, [31:12] reserved bits [11:8] supply voltage (VHS) [7:0] check pattern.
#define SD_SEND_CSD                        SD_CMD_R2(9u) //  R2, [31:16] RCA [15:0] stuff bits.
#define SD_SEND_CID                       SD_CMD_R2(10u) //  R2, [31:16] RCA [15:0] stuff bits.
#define SD_VOLTAGE_SWITCH                 SD_CMD_R1(11u) //  R1, [31:0] reserved bits (all 0).
#define SD_STOP_TRANSMISSION             SD_CMD_R1b(12u) // R1b, [31:0] stuff bits.
#define SD_SEND_STATUS                    SD_CMD_R1(13u) //  R1, [31:16] RCA [15] Send Task Status Register [14:0] stuff bits.
#define SD_SEND_TASK_STATUS               SD_CMD_R1(13u) //  R1, [31:16] RCA [15] Send Task Status Register [14:0] stuff bits.
#define SD_GO_INACTIVE_STATE            SD_CMD_NONE(15u) //   -, [31:16] RCA [15:0] reserved bits.

// Block-Oriented Read Commands (class 2).
#define SD_SET_BLOCKLEN                   SD_CMD_R1(16u) //  R1, [31:0] block length.
#define SD_READ_SINGLE_BLOCK            SD_CMD_R1_R(17u) //  R1, [31:0] data address.
#define SD_READ_MULTIPLE_BLOCK          SD_CMD_R1_R(18u) //  R1, [31:0] data address.
#define SD_SEND_TUNING_BLOCK            SD_CMD_R1_R(19u) //  R1, [31:0] reserved bits (all 0).
#define SD_SPEED_CLASS_CONTROL           SD_CMD_R1b(20u) // R1b, [31:28] Speed Class Control [27:0] See command description.
#define SD_ADDRESS_EXTENSION              SD_CMD_R1(22u) //  R1, [31:6] reserved bits (all 0) [5:0] extended address.
#define SD_SET_BLOCK_COUNT                SD_CMD_R1(23u) //  R1, [31:0] Block Count.

// Block-Oriented Write Commands (class 4).
// SET_BLOCKLEN
// SPEED_CLASS_CONTROL
// ADDRESS_EXTENSION
// SET_BLOCK_COUNT
#define SD_WRITE_BLOCK                  SD_CMD_R1_W(24u) //  R1, [31:0] data address.
#define SD_WRITE_MULTIPLE_BLOCK         SD_CMD_R1_W(25u) //  R1, [31:0] data address.
#define SD_PROGRAM_CSD                  SD_CMD_R1_W(27u) //  R1, [31:0] stuff bits.

// Block Oriented Write Protection Commands (class 6).
#define SD_SET_WRITE_PROT                SD_CMD_R1b(28u) // R1b, [31:0] data address.
#define SD_CLR_WRITE_PROT                SD_CMD_R1b(29u) // R1b, [31:0] data address.
#define SD_SEND_WRITE_PROT              SD_CMD_R1_R(30u) //  R1, [31:0] write protect data address.

// Erase Commands (class 5).
#define SD_ERASE_WR_BLK_START             SD_CMD_R1(32u) //  R1, [31:0] data address.
#define SD_ERASE_WR_BLK_END               SD_CMD_R1(33u) //  R1, [31:0] data address.
#define SD_ERASE                         SD_CMD_R1b(38u) // R1b, [31:0] Erase Function.

// Lock Card (class 7).
// SET_BLOCKLEN
// Command 40 "Defined by DPS Spec.".
#define SD_LOCK_UNLOCK                  SD_CMD_R1_W(42u) //  R1, [31:0] Reserved bits (Set all 0).

// Application-Specific Commands (class 8).
#define SD_APP_CMD                        SD_CMD_R1(55u) //  R1, [31:16] RCA [15:0] stuff bits.
#define SD_GEN_CMD_R                    SD_CMD_R1_R(56u) //  R1, [31:1] stuff bits. [0]: RD/WR = 1.
#define SD_GEN_CMD_W                    SD_CMD_R1_W(56u) //  R1, [31:1] stuff bits. [0]: RD/WR = 0.

// Application Specific Commands used/reserved by SD Memory Card.
#define SD_APP_SET_BUS_WIDTH              SD_ACMD_R1(6u) //  R1, [31:2] stuff bits [1:0] bus width.
#define SD_APP_SD_STATUS               SD_ACMD_R1_R(13u) //  R1, [31:0] stuff bits.
#define SD_APP_SEND_NUM_WR_BLOCKS      SD_ACMD_R1_R(22u) //  R1, [31:0] stuff bits.
#define SD_APP_SET_WR_BLK_ERASE_COUNT    SD_ACMD_R1(23u) //  R1, [31:23] stuff bits [22:0] Number of blocks.
#define SD_APP_SD_SEND_OP_COND           SD_ACMD_R3(41u) //  R3, [31] reserved bit [30] HCS (OCR[30]) [29] reserved for eSD [28] XPC [27:25] reserved bits [24] S18R [23:0] VDD Voltage Window (OCR[23:0]).
#define SD_APP_SET_CLR_CARD_DETECT       SD_ACMD_R1(42u) //  R1, [31:1] stuff bits [0] set_cd.
#define SD_APP_SEND_SCR                SD_ACMD_R1_R(51u) //  R1, [31:0] stuff bits.

// Switch Function Commands (class 10).
#define SD_SWITCH_FUNC                   SD_CMD_R1_R(6u) //  R1, [31] Mode 0: Check function 1: Switch function [30:24] reserved (All '0') [23:20] reserved for function group 6 (0h or Fh) [19:16] reserved for function group 5 (0h or Fh) [15:12] function group 4 for PowerLimit [11:8] function group 3 for Drive Strength [7:4] function group 2 for Command System [3:0] function group 1 for Access Mode.

// Function Extension Commands (class 11).
#define SD_READ_EXTR_SINGLE             SD_CMD_R1_R(48u) //  R1, [31] MIO0: Memory, 1: I/O [30:27] FNO[26] Reserved (=0) [25:9] ADDR [8:0] LEN.
#define SD_WRITE_EXTR_SINGLE            SD_CMD_R1_W(49u) //  R1, [31] MIO0: Memory, 1: I/O [30:27] FNO [26] MW [25:9] ADDR [8:0] LEN/MASK.
#define SD_READ_EXTR_MULTI              SD_CMD_R1_R(58u) //  R1, [31] MIO0: Memory, 1: I/O [30:27] FNO [26] BUS0: 512B, 1: 32KB [25:9] ADDR [8:0] BUC.
#define SD_WRITE_EXTR_MULTI             SD_CMD_R1_W(59u) //  R1, [31] MIO0: Memory, 1: I/O [30:27] FNO [26] BUS0: 512B, 1: 32KB [25:9] ADDR [8:0] BUC.

// Command Queue Function Commands (class 1).
#define SD_Q_MANAGEMENT                  SD_CMD_R1b(43u) // R1b, [31:21] Reserved [20:16]: Task ID [3:0]: Operation Code (Abort tasks etc.).
#define SD_Q_TASK_INFO_A                  SD_CMD_R1(44u) //  R1, [31] Reserved [30] Direction [29:24] Extended Address [23] Priority [22:21] Reserved [20:16] Task ID [15:0] Number of Blocks.
#define SD_Q_TASK_INFO_B                  SD_CMD_R1(45u) //  R1, [31:0] Start block address.
#define SD_Q_RD_TASK                    SD_CMD_R1_R(46u) //  R1, [31:21] Reserved [20:16] Task ID [15:0] Reserved.
#define SD_Q_WR_TASK                    SD_CMD_R1_W(47u) //  R1, [31:21] Reserved [20:16] Task ID [15:0] Reserved.


// 4.10.1 Card Status.
// Type:
// E: Error bit.
// S: Status bit.
// R: Detected and set for the actual command response.
// X: Detected and set during command execution. The host can get the status by issuing a command with R1 response.
//
// Clear Condition:
// A: According to the card current status.
// B: Always related to the previous command. Reception of a valid command will clear it (with a delay of one command).
// C: Clear by read.
#define SD_R1_AKE_SEQ_ERROR       (1u<<3)  //   E R C, Error in the sequence of the authentication process.
#define SD_R1_APP_CMD             (1u<<5)  //   S R C, The card will expect ACMD, or an indication that the command has been interpreted as ACMD.
#define SD_R1_FX_EVENT            (1u<<6)  //   S X A, ExtensionFunctions may set this bit to get host to deal with events.
#define SD_R1_READY_FOR_DATA      (1u<<8)  //   S X A, Corresponds to buffer empty signaling on the bus.
#define SD_R1_STATE_IDLE          (0u<<9)  //   S X B
#define SD_R1_STATE_READY         (1u<<9)  //   S X B
#define SD_R1_STATE_IDENT         (2u<<9)  //   S X B
#define SD_R1_STATE_STBY          (3u<<9)  //   S X B
#define SD_R1_STATE_TRAN          (4u<<9)  //   S X B
#define SD_R1_STATE_DATA          (5u<<9)  //   S X B
#define SD_R1_STATE_RCV           (6u<<9)  //   S X B
#define SD_R1_STATE_PRG           (7u<<9)  //   S X B
#define SD_R1_STATE_DIS           (8u<<9)  //   S X B
#define SD_R1_ERASE_RESET         (1u<<13) //   S R C, An erase sequence was cleared before executing because an out of erase sequence command was received.
#define SD_R1_CARD_ECC_DISABLED   (1u<<14) //   S X A, The command has been executed without using the internal ECC.
#define SD_R1_WP_ERASE_SKIP       (1u<<15) // E R X C, Set when only partial address space was erased due to existing write protected blocks or the temporary or permanent write protected cardwas erased.
#define SD_R1_CSD_OVERWRITE       (1u<<16) // E R X C, Can be either one of the following errors: -The read only section of the CSD does not match the card content. -An attempt to reverse the copy (set as original) or permanent WP (unprotected) bits was made.
// 17 reserved for DEFERRED_RESPONSE (Refer to eSD Addendum)
#define SD_R1_ERROR               (1u<<19) // E R X C, A general or an unknown error occurred during the operation.
#define SD_R1_CC_ERROR            (1u<<20) // E R X C, Internal card controller error:
#define SD_R1_CARD_ECC_FAILED     (1u<<21) // E R X C, Card internal ECC was applied but failed to correct the data.
#define SD_R1_ILLEGAL_COMMAND     (1u<<22) //   E R B, Command  not  legal  for  the  card state.
#define SD_R1_COM_CRC_ERROR       (1u<<23) //   E R B, The CRC check of the previous command failed.
#define SD_R1_LOCK_UNLOCK_FAILED  (1u<<24) // E R X C, Set when a sequence or password error has been detected in lock/unlock card command.
#define SD_R1_CARD_IS_LOCKED      (1u<<25) //   S X A, When set, signals that the card is locked by the host.
#define SD_R1_WP_VIOLATION        (1u<<26) // E R X C, Set when the host attempts to write to a protected block or to thetemporary or permanent write protected card.
#define SD_R1_ERASE_PARAM         (1u<<27) // E R X C, An invalid selection of write-blocks for erase occurred.
#define SD_R1_ERASE_SEQ_ERROR     (1u<<28) //   E R C, An error in the sequence of erase commands occurred.
#define SD_R1_BLOCK_LEN_ERROR     (1u<<29) // E R X C, The transferred block length is not allowed for this card, or the number of transferred bytes does not match the block length.
#define SD_R1_ADDRESS_ERROR       (1u<<30) // E R X C, A misaligned address which did not match the block length was used in the command.
#define SD_R1_OUT_OF_RANGE        (1u<<31) // E R X C, The command's argument was out of the allowed range for this card.

#define SD_R1_ERR_ALL             (SD_R1_OUT_OF_RANGE | SD_R1_ADDRESS_ERROR | SD_R1_BLOCK_LEN_ERROR | \
                                   SD_R1_ERASE_SEQ_ERROR | SD_R1_ERASE_PARAM | SD_R1_WP_VIOLATION | \
                                   SD_R1_LOCK_UNLOCK_FAILED | SD_R1_COM_CRC_ERROR | SD_R1_ILLEGAL_COMMAND | \
                                   SD_R1_CARD_ECC_FAILED | SD_R1_CC_ERROR | SD_R1_ERROR | \
                                   SD_R1_CSD_OVERWRITE | SD_R1_WP_ERASE_SKIP | SD_R1_AKE_SEQ_ERROR)

// Argument bits for SEND_IF_COND (CMD8).
#define SD_CMD8_CHK_PATT      (0xAAu)  // Check pattern.
#define SD_CMD8_VHS_2_7_3_6V  (1u<<8)  // Voltage supplied (VHS) 2.7-3.6V.
#define SD_CMD8_PCIe          (1u<<12) // PCIe Avail-ability.
#define SD_CMD8_PCIe_1_2V     (1u<<13) // PCIe 1.2V Support.

// 5.1 OCR register.
#define SD_OCR_2_7_2_8V  (1u<<15) // 2.7-2.8V.
#define SD_OCR_2_8_2_9V  (1u<<16) // 2.8-2.9V.
#define SD_OCR_2_9_3_0V  (1u<<17) // 2.9-3.0V.
#define SD_OCR_3_0_3_1V  (1u<<18) // 3.0-3.1V.
#define SD_OCR_3_1_3_2V  (1u<<19) // 3.1-3.2V.
#define SD_OCR_3_2_3_3V  (1u<<20) // 3.2-3.3V.
#define SD_OCR_3_3_3_4V  (1u<<21) // 3.3-3.4V.
#define SD_OCR_3_4_3_5V  (1u<<22) // 3.4-3.5V.
#define SD_OCR_3_5_3_6V  (1u<<23) // 3.5-3.6V.
#define SD_OCR_S18A      (1u<<24) // S18A: Switching to 1.8V Accepted. 0b: Continues current voltage signaling, 1b: Ready for switching signal voltage.
#define SD_OCR_CO2T      (1u<<27) // Over 2TB Card. CCS must also be 1 if this is 1.
#define SD_OCR_UHS_II    (1u<<29) // UHS-II Card Status. 0b: Non UHS-II Card, 1b: UHS-II Card.
#define SD_OCR_CCS       (1u<<30) // Card Capacity Status. 0b: SDSC, 1b: SDHC or SDXC.
#define SD_OCR_READY     (1u<<31) // Busy Status. 0b: On Initialization, 1b: Initialization Complete.

// Argument bits for SEND_OP_COND (ACMD41).
// For voltage bits see OCR register above.
#define SD_ACMD41_S18R   (1u<<24) // S18R: Switching to 1.8V Request. 0b: Use current signal voltage, 1b: Switch to 1.8V signal voltage.
#define SD_ACMD41_HO2T   (1u<<27) // Over 2TB Supported Host. HCS must also be 1 if this is 1.
#define SD_ACMD41_XPC    (1u<<28) // SDXC Power Control. 0b: Power Saving, 1b: Maximum Performance.
#define SD_ACMD41_HCS    (1u<<30) // Host Capacity Support. 0b: SDSC Only Host, 1b: SDHC or SDXC Supported.

// 4.3.10 Switch Function Command.
// mode:   0 = check function, 1 = set function
// pwr:    Function group 4 Power Limit.
// driver: Function group 3 Driver Strength.
// cmd:    Function group 2 Command system.
// acc:    Function group 1 Access mode.
#define SD_SWITCH_FUNC_ARG(mode, pwr, driver, cmd, acc)  ((mode)<<31 | 0xFFu<<16 | ((pwr)&0xFu)<<12 | ((driver)&0xFu)<<8 | ((cmd)&0xFu)<<4 | ((acc)&0xFu))
