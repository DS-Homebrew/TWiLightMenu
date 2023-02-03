#pragma once

// SPDX-License-Identifier: MIT

// Based on JEDEC eMMC Card Product Standard V4.41.

#include "tmio.h"


// Controller specific macros. Add controller specific bits here.
// MMC_CMD_[response type]_[transfer type]
// Transfer type: R = read, W = write.
#define MMC_CMD_NONE(id)   (CMD_RESP_NONE | (id))
#define MMC_CMD_R1(id)     (CMD_RESP_R1   | (id))
#define MMC_CMD_R1b(id)    (CMD_RESP_R1b  | (id))
#define MMC_CMD_R2(id)     (CMD_RESP_R2   | (id))
#define MMC_CMD_R3(id)     (CMD_RESP_R3   | (id))
#define MMC_CMD_R4(id)     (CMD_RESP_R4   | (id))
#define MMC_CMD_R5(id)     (CMD_RESP_R5   | (id))
#define MMC_CMD_R1_R(id)   (CMD_DIR_R | CMD_DT_EN | CMD_RESP_R1 | (id))
#define MMC_CMD_R1_W(id)   (CMD_DIR_W | CMD_DT_EN | CMD_RESP_R1 | (id))


// Basic commands and read-stream command (class 0 and class 1).
#define MMC_GO_IDLE_STATE          MMC_CMD_NONE(0u) //      -, [31:0] 0x00000000 GO_IDLE_STATE, 0xF0F0F0F0 GO_PRE_IDLE_STATE, 0xFFFFFFFA BOOT_INITIATION.
#define MMC_SEND_OP_COND             MMC_CMD_R3(1u) //     R3, [31:0] OCR with-out busy.
#define MMC_ALL_SEND_CID             MMC_CMD_R2(2u) //     R2, [31:0] stuff bits.
#define MMC_SET_RELATIVE_ADDR        MMC_CMD_R1(3u) //     R1, [31:16] RCA [15:0] stuff bits.
#define MMC_SET_DSR                MMC_CMD_NONE(4u) //      -, [31:16] DSR [15:0] stuff bits.
#define MMC_SLEEP_AWAKE             MMC_CMD_R1b(5u) //    R1b, [31:16] RCA [15] Sleep/Awake [14:0] stuff bits.
#define MMC_SWITCH                  MMC_CMD_R1b(6u) //    R1b, [31:26] Set to 0 [25:24] Access [23:16] Index [15:8] Value [7:3] Set to 0 [2:0] Cmd Set.
#define MMC_SELECT_CARD             MMC_CMD_R1b(7u) // R1/R1b, [31:16] RCA [15:0] stuff bits. Note: "R1b while selecting from Disconnected State to Programming State."
#define MMC_DESELECT_CARD          MMC_CMD_NONE(7u) //      -, [31:16] RCA [15:0] stuff bits.
#define MMC_SEND_EXT_CSD           MMC_CMD_R1_R(8u) //     R1, [31:0] stuff bits.
#define MMC_SEND_CSD                 MMC_CMD_R2(9u) //     R2, [31:16] RCA [15:0] stuff bits.
#define MMC_SEND_CID                MMC_CMD_R2(10u) //     R2, [31:16] RCA [15:0] stuff bits.
#define MMC_READ_DAT_UNTIL_STOP   MMC_CMD_R1_R(11u) //     R1, [31:0] data address.
#define MMC_STOP_TRANSMISSION      MMC_CMD_R1b(12u) // R1/R1b, [31:16] RCA [15:1] stuff bits [0] HPI. Note: "RCA in CMD12 is used only if HPI bit is set." Note 2: "R1 for read cases and R1b for write cases."
#define MMC_SEND_STATUS             MMC_CMD_R1(13u) //     R1, [31:16] RCA [15:1] stuff bits [0] HPI.
#define MMC_BUSTEST_R             MMC_CMD_R1_R(14u) //     R1, [31:0] stuff bits.
#define MMC_GO_INACTIVE_STATE     MMC_CMD_NONE(15u) //      -, [31:16] RCA [15:0] stuff bits.
#define MMC_BUSTEST_W             MMC_CMD_R1_W(19u) //     R1, [31:0] stuff bits.

// Block-oriented read commands (class 2).
#define MMC_SET_BLOCKLEN            MMC_CMD_R1(16u) //     R1, [31:0] block length.
#define MMC_READ_SINGLE_BLOCK     MMC_CMD_R1_R(17u) //     R1, [31:0] data address.
#define MMC_READ_MULTIPLE_BLOCK   MMC_CMD_R1_R(18u) //     R1, [31:0] data address.

// Stream write commands (class 3).
#define MMC_WRITE_DAT_UNTIL_STOP  MMC_CMD_R1_W(20u) //     R1, [31:0] data address.

// Block-oriented write commands (class 4).
#define MMC_SET_BLOCK_COUNT         MMC_CMD_R1(23u) //     R1, [31] Reliable Write Request [30:16] set to 0 [15:0] number of blocks.
#define MMC_WRITE_BLOCK           MMC_CMD_R1_W(24u) //     R1, [31:0] data address.
#define MMC_WRITE_MULTIPLE_BLOCK  MMC_CMD_R1_W(25u) //     R1, [31:0] data address.
#define MMC_PROGRAM_CID           MMC_CMD_R1_W(26u) //     R1, [31:0] stuff bits.
#define MMC_PROGRAM_CSD           MMC_CMD_R1_W(27u) //     R1, [31:0] stuff bits.

// Block-oriented write protection commands (class 6).
#define MMC_SET_WRITE_PROT         MMC_CMD_R1b(28u) //    R1b, [31:0] data address.
#define MMC_CLR_WRITE_PROT         MMC_CMD_R1b(29u) //    R1b, [31:0] data address.
#define MMC_SEND_WRITE_PROT       MMC_CMD_R1_R(30u) //     R1, [31:0] write protect data address.
#define MMC_SEND_WRITE_PROT_TYPE  MMC_CMD_R1_R(31u) //     R1, [31:0] write protect data address.

// Erase commands (class 5).
#define MMC_ERASE_GROUP_START       MMC_CMD_R1(35u) //     R1, [31:0] data address.
#define MMC_ERASE_GROUP_END         MMC_CMD_R1(36u) //     R1, [31:0] data address.
#define MMC_ERASE                  MMC_CMD_R1b(38u) //    R1b, [31] Secure request [30:16] set to 0 [15] Force Garbage Collect request [14:1] set to 0 [0] Identify Write block for Erase.

// I/O mode commands (class 9).
#define MMC_FAST_IO                 MMC_CMD_R4(39u) //     R4, [31:16] RCA [15:15] register write flag [14:8] register address [7:0] register data.
#define MMC_GO_IRQ_STATE            MMC_CMD_R5(40u) //     R5, [31:0] stuff bits.

// Lock card commands (class 7).
#define MMC_LOCK_UNLOCK           MMC_CMD_R1_W(42u) //     R1, [31:0] stuff bits.

// Application-specific commands (class 8).
#define MMC_APP_CMD                 MMC_CMD_R1(55u) //     R1, [31:16] RCA [15:0] stuff bits.
#define MMC_GEN_CMD_R             MMC_CMD_R1_R(56u) //     R1, [31:1] stuff bits [0] RD/WR = 1.
#define MMC_GEN_CMD_W             MMC_CMD_R1_W(56u) //     R1, [31:1] stuff bits [0] RD/WR = 0.


// 7.13 Card status.
// Type:
// E: Error bit.
// S: Status bit.
// R: Detected and set for the actual command response.
// X: Detected and set during command execution. The host can get the status by issuing a command with R1 response.
//
// Clear Condition:
// A: These bits are persistent, they are set and cleared in accordance with the card status.
// B: These bits are cleared as soon as the response (reporting the error) is sent out.
#define MMC_R1_APP_CMD               (1u<<5)  // S R   A, The card will expect ACMD, or indication that the command has been interpreted as ACMD.
#define MMC_R1_URGENT_BKOPS          (1u<<6)  // S R   A, If set, device needs to perform backgroundoperations urgently. Host can check EXT_CSD field BKOPS_STATUS for the detailed level.
#define MMC_R1_SWITCH_ERROR          (1u<<7)  // E X   B, If set, the card did not switch to the expected mode as requested by the SWITCH command.
#define MMC_R1_READY_FOR_DATA        (1u<<8)  // S R   A, Corresponds to buffer empty signalling on the bus.
#define MMC_R1_STATE_IDLE            (0u<<9)  // S R   A
#define MMC_R1_STATE_READY           (1u<<9)  // S R   A
#define MMC_R1_STATE_IDENT           (2u<<9)  // S R   A
#define MMC_R1_STATE_STBY            (3u<<9)  // S R   A
#define MMC_R1_STATE_TRAN            (4u<<9)  // S R   A
#define MMC_R1_STATE_DATA            (5u<<9)  // S R   A
#define MMC_R1_STATE_RCV             (6u<<9)  // S R   A
#define MMC_R1_STATE_PRG             (7u<<9)  // S R   A
#define MMC_R1_STATE_DIS             (8u<<9)  // S R   A
#define MMC_R1_STATE_BTST            (9u<<9)  // S R   A
#define MMC_R1_STATE_SLP             (10u<<9) // S R   A
#define MMC_R1_ERASE_RESET           (1u<<13) // E R   B, An erase sequence was cleared before executing because an out of erase sequence command was received (commands other than CMD35, CMD36, CMD38 or CMD13.
#define MMC_R1_WP_ERASE_SKIP         (1u<<15) // E X   B, Only partial address space was erased due to existing write protected blocks.
#define MMC_R1_CXD_OVERWRITE         (1u<<16) // E X   B, Can be either one of the following errors: - The CID register has been already written and can not be overwritten - The read only section of the CSD does not match the card content. - An attempt to reverse the copy (set as original) or permanent WP (unprotected) bits was made.
#define MMC_R1_OVERRUN               (1u<<17) // E X   B, The card could not sustain data programming in stream write mode.
#define MMC_R1_UNDERRUN              (1u<<18) // E X   B, The card could not sustain data transfer in stream read mode.
#define MMC_R1_ERROR                 (1u<<19) // E X   B, (Undefined by the standard) A generic card error related to the (and detected during) execution of the last host command (e.g. read or write failures).
#define MMC_R1_CC_ERROR              (1u<<20) // E R   B, (Undefined by the standard) A card error occurred, which is not related to the host command.
#define MMC_R1_CARD_ECC_FAILED       (1u<<21) // E X   B, Card internal ECC was applied but failed to correct the data.
#define MMC_R1_ILLEGAL_COMMAND       (1u<<22) // E R   B, Command not legal for the card state.
#define MMC_R1_COM_CRC_ERROR         (1u<<23) // E R   B, The CRC check of the previous command failed.
#define MMC_R1_LOCK_UNLOCK_FAILED    (1u<<24) // E X   B, Set when a sequence or password error has been detected in lock/unlock card command.
#define MMC_R1_CARD_IS_LOCKED        (1u<<25) // S R   A, When set, signals that the card is locked by the host.
#define MMC_R1_WP_VIOLATION          (1u<<26) // E X   B, Attempt to program a write protected block.
#define MMC_R1_ERASE_PARAM           (1u<<27) // E X   B, An invalid selection of erase groups for erase occurred.
#define MMC_R1_ERASE_SEQ_ERROR       (1u<<28) // E R   B, An error in the sequence of erase commands occurred.
#define MMC_R1_BLOCK_LEN_ERROR       (1u<<29) // E R   B, Either the argument of a SET_BLOCKLEN command exceeds the maximum value allowed for the card, or the previously defined block length is illegal for the current command (e.g. the host issues a write command, the current block length is smaller than the card’s maximum and write partial blocks is not allowed).
#define MMC_R1_ADDRESS_MISALIGN      (1u<<30) // E R/X B, The command’ s address argument (in accordance with the currently set block length) positions the first data block misaligned to the card physical blocks. A multiple block read/write operation (although started with a valid address/blocklength combination) is attempting to read or write a data block which does not align with the physical blocks of the card.
#define MMC_R1_ADDRESS_OUT_OF_RANGE  (1u<<31) // E R/X B, The command’s address argument was out of the allowed range for this card. A multiple block or stream read/write operation is (although started in a valid address) attempting to read or write beyond the card capacity.

#define MMC_R1_ERR_ALL               (MMC_R1_ADDRESS_OUT_OF_RANGE | MMC_R1_ADDRESS_MISALIGN | \
                                      MMC_R1_BLOCK_LEN_ERROR | MMC_R1_ERASE_SEQ_ERROR | \
                                      MMC_R1_ERASE_PARAM | MMC_R1_WP_VIOLATION | MMC_R1_LOCK_UNLOCK_FAILED | \
                                      MMC_R1_COM_CRC_ERROR | MMC_R1_ILLEGAL_COMMAND | MMC_R1_CARD_ECC_FAILED | \
                                      MMC_R1_CC_ERROR | MMC_R1_ERROR | MMC_R1_UNDERRUN | MMC_R1_OVERRUN | \
                                      MMC_R1_CXD_OVERWRITE | MMC_R1_WP_ERASE_SKIP | MMC_R1_ERASE_RESET | \
                                      MMC_R1_SWITCH_ERROR)

// 8.1 OCR register.
// Same bits for CMD1 argument.
#define MMC_OCR_1_7_1_95V  (1u<<7)  // 1.70–1.95V.
#define MMC_OCR_2_0_2_1V   (1u<<8)  // 2.0-2.1V.
#define MMC_OCR_2_1_2_2V   (1u<<9)  // 2.1-2.2V.
#define MMC_OCR_2_2_2_3V   (1u<<10) // 2.2-2.3V.
#define MMC_OCR_2_3_2_4V   (1u<<11) // 2.3-2.4V.
#define MMC_OCR_2_4_2_5V   (1u<<12) // 2.4-2.5V.
#define MMC_OCR_2_5_2_6V   (1u<<13) // 2.5-2.6V.
#define MMC_OCR_2_6_2_7V   (1u<<14) // 2.6-2.7V.
#define MMC_OCR_2_7_2_8V   (1u<<15) // 2.7-2.8V.
#define MMC_OCR_2_8_2_9V   (1u<<16) // 2.8-2.9V.
#define MMC_OCR_2_9_3_0V   (1u<<17) // 2.9-3.0V.
#define MMC_OCR_3_0_3_1V   (1u<<18) // 3.0-3.1V.
#define MMC_OCR_3_1_3_2V   (1u<<19) // 3.1-3.2V.
#define MMC_OCR_3_2_3_3V   (1u<<20) // 3.2-3.3V.
#define MMC_OCR_3_3_3_4V   (1u<<21) // 3.3-3.4V.
#define MMC_OCR_3_4_3_5V   (1u<<22) // 3.4-3.5V.
#define MMC_OCR_3_5_3_6V   (1u<<23) // 3.5-3.6V.
#define MMC_OCR_BYTE_MODE  (0u<<29) // Access mode = byte mode.
#define MMC_OCR_SECT_MODE  (2u<<29) // Access mode = sector mode.
#define MMC_OCR_READY      (1u<<31) // Card power up status bit (busy). 0 = busy.

// 7.6.1 Command sets and extended settings.
#define MMC_SWITCH_ACC_CMD_SET   (0u)
#define MMC_SWITCH_ACC_SET_BITS  (1u)
#define MMC_SWITCH_ACC_CLR_BITS  (2u)
#define MMC_SWITCH_ACC_WR_BYTE   (3u)
#define MMC_SWITCH_ARG(acc, idx, val, cmdSet)  (((acc)&3u)<<24 | ((idx)&0xFFu)<<16 | ((val)&0xFFu)<<8 | ((cmdSet)&7u))

// 8.4 Extended CSD register.
// size in bytes, access, description.
#define EXT_CSD_SEC_BAD_BLK_MGMNT            (134u) //  1,                    R/W, Bad Block Management mode.
#define EXT_CSD_ENH_START_ADDR               (136u) //  4,                    R/W, Enhanced User Data Start Address.
#define EXT_CSD_ENH_SIZE_MULT                (140u) //  3,                    R/W, Enhanced User Data Area Size.
#define EXT_CSD_GP_SIZE_MULT                 (143u) // 12,                    R/W, General Purpose Partition Size.
#define EXT_CSD_PARTITION_SETTING_COMPLETED  (155u) //  1,                    R/W, Paritioning Setting.
#define EXT_CSD_PARTITIONS_ATTRIBUTE         (156u) //  1,                    R/W, Partitions attribute.
#define EXT_CSD_MAX_ENH_SIZE_MULT            (157u) //  3,                      R, Max Enhanced Area Size.
#define EXT_CSD_PARTITIONING_SUPPORT         (160u) //  1,                      R, Partitioning Support.
#define EXT_CSD_HPI_MGMT                     (161u) //  1,                R/W/E_P, HPI management.
#define EXT_CSD_RST_n_FUNCTION               (162u) //  1,                    R/W, H/W reset function.
#define EXT_CSD_BKOPS_EN                     (163u) //  1,                    R/W, Enable background operations handshake.
#define EXT_CSD_BKOPS_START                  (164u) //  1,                  W/E_P, Manually start background operations.
#define EXT_CSD_WR_REL_PARAM                 (166u) //  1,                      R, Write reliability parameter register.
#define EXT_CSD_WR_REL_SET                   (167u) //  1,                    R/W, Write reliability setting register.
#define EXT_CSD_RPMB_SIZE_MULT               (168u) //  1,                      R, RPMB Size.
#define EXT_CSD_FW_CONFIG                    (169u) //  1,                    R/W, FW configuration.
#define EXT_CSD_USER_WP                      (171u) //  1, R/W, R/W/C_P & R/W/E_P, User area write protection register.
#define EXT_CSD_BOOT_WP                      (173u) //  1,          R/W & R/W/C_P, Boot area write protection register.
#define EXT_CSD_ERASE_GROUP_DEF              (175u) //  1,                R/W/E_P, High-density erase group definition.
#define EXT_CSD_BOOT_BUS_WIDTH               (177u) //  1,                  R/W/E, Boot bus width1.
#define EXT_CSD_BOOT_CONFIG_PROT             (178u) //  1,          R/W & R/W/C_P, Boot config protection.
#define EXT_CSD_PARTITION_CONFIG             (179u) //  1,        R/W/E & R/W/E_P, Partition configuration.
#define EXT_CSD_ERASED_MEM_CONT              (181u) //  1,                      R, Erased memory content.
#define EXT_CSD_BUS_WIDTH                    (183u) //  1,                  W/E_P, Bus width mode.
#define EXT_CSD_HS_TIMING                    (185u) //  1,                R/W/E_P, High-speed interface timing.
#define EXT_CSD_POWER_CLASS                  (187u) //  1,                R/W/E_P, Power class.
#define EXT_CSD_CMD_SET_REV                  (189u) //  1,                      R, Command set revision.
#define EXT_CSD_CMD_SET                      (191u) //  1,                R/W/E_P, Command set.
#define EXT_CSD_EXT_CSD_REV                  (192u) //  1,                      R, Extended CSD revision.
#define EXT_CSD_CSD_STRUCTURE                (194u) //  1,                      R, CSD structure version.
#define EXT_CSD_CARD_TYPE                    (196u) //  1,                      R, Card type.
#define EXT_CSD_OUT_OF_INTERRUPT_TIME        (198u) //  1,                      R, Out-of-interrupt busy timing.
#define EXT_CSD_PARTITION_SWITCH_TIME        (199u) //  1,                      R, Partition switching timing.
#define EXT_CSD_PWR_CL_52_195                (200u) //  1,                      R, Power class for 52MHz at 1.95V.
#define EXT_CSD_PWR_CL_26_195                (201u) //  1,                      R, Power class for 26MHz at 1.95V.
#define EXT_CSD_PWR_CL_52_360                (202u) //  1,                      R, Power class for 52MHz at 3.6V.
#define EXT_CSD_PWR_CL_26_360                (203u) //  1,                      R, Power class for 26MHz at 3.6V.
#define EXT_CSD_MIN_PERF_R_4_26              (205u) //  1,                      R, Minimum Read Performance for 4bit at 26MHz.
#define EXT_CSD_MIN_PERF_W_4_26              (206u) //  1,                      R, Minimum Write Performance for 4bit at 26MHz.
#define EXT_CSD_MIN_PERF_R_8_26_4_52         (207u) //  1,                      R, Minimum Read Performance for 8bit at 26MHz, for 4bit at 52MHz.
#define EXT_CSD_MIN_PERF_W_8_26_4_52         (208u) //  1,                      R, Minimum Write Performance for 8bit at 26MHz, for 4bit at 52MHz.
#define EXT_CSD_MIN_PERF_R_8_52              (209u) //  1,                      R, Minimum Read Performance for 8bit at 52MHz.
#define EXT_CSD_MIN_PERF_W_8_52              (210u) //  1,                      R, Minimum Write Performance for 8bit at 52MHz.
#define EXT_CSD_SEC_COUNT                    (212u) //  4,                      R, Sector Count.
#define EXT_CSD_S_A_TIMEOUT                  (217u) //  1,                      R, Sleep/awake timeout.
#define EXT_CSD_S_C_VCCQ                     (219u) //  1,                      R, Sleep current (VCCQ).
#define EXT_CSD_S_C_VCC                      (220u) //  1,                      R, Sleep current (VCC).
#define EXT_CSD_HC_WP_GRP_SIZE               (221u) //  1,                      R, High-capacity write protect group size.
#define EXT_CSD_REL_WR_SEC_C                 (222u) //  1,                      R, Reliable write sector count.
#define EXT_CSD_ERASE_TIMEOUT_MULT           (223u) //  1,                      R, High-capacity erase timeout.
#define EXT_CSD_HC_ERASE_GRP_SIZE            (224u) //  1,                      R, High-capacity erase unit size.
#define EXT_CSD_ACC_SIZE                     (225u) //  1,                      R, Access size.
#define EXT_CSD_BOOT_SIZE_MULTI              (226u) //  1,                      R, Boot partition size.
#define EXT_CSD_BOOT_INFO                    (228u) //  1,                      R, Boot information.
#define EXT_CSD_SEC_TRIM_MULT                (229u) //  1,                      R, Secure TRIM Multiplier.
#define EXT_CSD_SEC_ERASE_MULT               (230u) //  1,                      R, Secure Erase Multiplier.
#define EXT_CSD_SEC_FEATURE_SUPPORT          (231u) //  1,                      R, Secure Feature support.
#define EXT_CSD_TRIM_MULT                    (232u) //  1,                      R, TRIM Multiplier.
#define EXT_CSD_MIN_PERF_DDR_R_8_52          (234u) //  1,                      R, Minimum Read Performance for 8bit at 52MHz in DDR mode.
#define EXT_CSD_MIN_PERF_DDR_W_8_52          (235u) //  1,                      R, Minimum Write Performance for 8bit at 52MHz in DDR mode.
#define EXT_CSD_PWR_CL_DDR_52_195            (238u) //  1,                      R, Power class for 52MHz, DDR at 1.95V.
#define EXT_CSD_PWR_CL_DDR_52_360            (239u) //  1,                      R, Power class for 52MHz, DDR at 3.6V.
#define EXT_CSD_INI_TIMEOUT_AP               (241u) //  1,                      R, 1st initialization time after partitioning.
#define EXT_CSD_CORRECTLY_PRG_SECTORS_NUM    (242u) //  4,                      R, Number of correctly programmed sectors.
#define EXT_CSD_BKOPS_STATUS                 (246u) //  1,                      R, Background operations status.
#define EXT_CSD_BKOPS_SUPPORT                (502u) //  1,                      R, Background operations support.
#define EXT_CSD_HPI_FEATURES                 (503u) //  1,                      R, HPI features.
#define EXT_CSD_S_CMD_SET                    (504u) //  1,                      R, Supported Command Sets.
