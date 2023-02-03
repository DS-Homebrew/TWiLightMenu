#pragma once

// SPDX-License-Identifier: MIT

#include <assert.h>
#include <nds.h>


// For simplicity we will name the accessible 2 controllers 1 and 2.
// The real controller number is in the comment.
#define TMIO1_REGS_BASE (0x04004800u) // Controller 1.
#define TMIO2_REGS_BASE (0x04004A00u) // Controller 2.

#define TMIO_HCLK       (33513982u) // In Hz.

typedef struct
{
	vu16 sd_cmd;              // 0x000
	vu16 sd_portsel;          // 0x002
	vu32 sd_arg;              // 0x004 SD_ARG0 and SD_ARG1 combined.
	vu16 sd_stop;             // 0x008
	vu16 sd_blockcount;       // 0x00A
	const vu32 sd_resp[4];    // 0x00C SD_RESP0-7 16 bit reg pairs combined.
	vu32 sd_status;           // 0x01C SD_STATUS1 and SD_STATUS2 combined.
	vu32 sd_status_mask;      // 0x020 SD_STATUS1_MASK and SD_STATUS2_MASK combined.
	vu16 sd_clk_ctrl;         // 0x024
	vu16 sd_blocklen;         // 0x026
	vu16 sd_option;           // 0x028 Card detect timer, data timeout and bus width.
	u8 _0x2a[2];
	const vu32 sd_err_status; // 0x02C SD_ERR_STATUS1 and SD_ERR_STATUS2 combined.
	vu16 sd_fifo;             // 0x030
	u8 _0x32[2];
	vu16 sdio_mode;           // 0x034
	vu16 sdio_status;         // 0x036
	vu16 sdio_status_mask;    // 0x038
	u8 _0x3a[0x9e];
	vu16 dma_ext_mode;        // 0x0D8
	u8 _0xda[6];
	vu16 soft_rst;            // 0x0E0
	const vu16 revision;      // 0x0E2 Controller version/revision?
	u8 _0xe4[0xe];
	vu16 unkF2;               // 0x0F2 Power related? Default 0. Other values do nothing?
	vu16 ext_sdio_irq;        // 0x0F4 Port 1/2/3 SDIO IRQ control.
	const vu16 ext_wrprot;    // 0x0F6 Apparently for eMMC.
	vu16 ext_cdet;            // 0x0F8 Card detect status.
	vu16 ext_cdet_dat3;       // 0x0FA DAT3 card detect status.
	vu16 ext_cdet_mask;       // 0x0FC Card detect mask (IRQ).
	vu16 ext_cdet_dat3_mask;  // 0x0FE DAT3 card detect mask (IRQ).
	vu16 sd_fifo32_cnt;       // 0x100
	u8 _0x102[2];
	vu16 sd_blocklen32;       // 0x104
	u8 _0x106[2];
	vu16 sd_blockcount32;     // 0x108
	u8 _0x10a[2];
	vu32 sd_fifo32;           // 0x10C Note: This is in the FIFO region on ARM11 (3DS).
} Tmio;
static_assert(offsetof(Tmio, sd_fifo32) == 0x10C, "Error: Member sd_fifo32 of Tmio is not at offset 0x10C!");

__attribute__((always_inline)) static inline Tmio* getTmioRegs(const u8 controller)
{
	return (controller == 0 ? (Tmio*)TMIO1_REGS_BASE : (Tmio*)TMIO2_REGS_BASE);
}

__attribute__((always_inline)) static inline vu32* getTmioFifo(Tmio *const regs)
{
	return &regs->sd_fifo32;
}


// REG_SD_CMD
// Auto response supported commands:
// CMD0, CMD2, CMD3 (only SD?), CMD7 (only select?), CMD9, CMD10, CMD12, CMD13,
// CMD16, CMD17, CMD18, CMD25, CMD28, CMD55, ACMD6, ACMD23, ACMD42, ACMD51.
//
// When using auto response leave bits 11-13 unset (zero).

// Bit 0-5 command index.
#define CMD_ACMD                 (1u<<6)        // Application command.
#define CMD_RESP_AUTO            (0u)           // Response type auto. Only works with certain commands.
#define CMD_RESP_NONE            (3u<<8)        // Response type none.
#define CMD_RESP_R1              (4u<<8)        // Response type R1 48 bit.
#define CMD_RESP_R5              (CMD_RESP_R1)  // Response type R5 48 bit.
#define CMD_RESP_R6              (CMD_RESP_R1)  // Response type R6 48 bit.
#define CMD_RESP_R7              (CMD_RESP_R1)  // Response type R7 48 bit.
#define CMD_RESP_R1b             (5u<<8)        // Response type R1b 48 bit + busy.
#define CMD_RESP_R5b             (CMD_RESP_R1b) // Response type R5b 48 bit + busy.
#define CMD_RESP_R2              (6u<<8)        // Response type R2 136 bit.
#define CMD_RESP_R3              (7u<<8)        // Response type R3 48 bit OCR without CRC.
#define CMD_RESP_R4              (CMD_RESP_R3)  // Response type R4 48 bit OCR without CRC.
#define CMD_RESP_MASK            (CMD_RESP_R3)
#define CMD_DT_EN                (1u<<11)       // Data transfer enable.
#define CMD_DIR_R                (1u<<12)       // Data transfer direction read.
#define CMD_DIR_W                (0u)           // Data transfer direction write.
#define CMD_MBT                  (1u<<13)       // Multi block transfer (auto STOP_TRANSMISSION).
#define CMD_SEC_SDIO             (1u<<14)       // Security/SDIO command.

// REG_SD_PORTSEL
#define PORTSEL_P0               (0u) // Controller port 0.
#define PORTSEL_P1               (1u) // Controller port 1.
#define PORTSEL_P2               (2u) // Controller port 2.
#define PORTSEL_P3               (3u) // Controller port 3.
#define PORTSEL_MASK             (PORTSEL_P3)
// Bit 8-9 number of supported ports?
#define PORTSEL_UNK10            (1u<<10) // Unknown writable bit 10?

// REG_SD_STOP
#define STOP_STOP                (1u)    // Abort data transfer and send STOP_TRANSMISSION CMD.
#define STOP_AUTO_STOP           (1u<<8) // Automatically send STOP_TRANSMISSION on multi-block transfer end.

// REG_SD_STATUS1/2       Write 0 to acknowledge a bit.
// REG_SD_STATUS1/2_MASK  (M) = Maskable bit. 1 = disabled.
// Unmaskable bits act as status only, don't trigger IRQs and can't be acknowledged.
#define STATUS_RESP_END          (1u)     // (M) Response end.
#define STATUS_DATA_END          (1u<<2)  // (M) Data transfer end (triggers after last block).
#define STATUS_REMOVE            (1u<<3)  // (M) Card got removed.
#define STATUS_INSERT            (1u<<4)  // (M) Card got inserted. Set at the same time as DETECT.
#define STATUS_DETECT            (1u<<5)  // Card detect status (SD_OPTION detection timer). 1 = inserted.
#define STATUS_NO_WRPROT         (1u<<7)  // Write protection slider unlocked (low).
#define STATUS_DAT3_REMOVE       (1u<<8)  // (M) Card DAT3 got removed (low).
#define STATUS_DAT3_INSERT       (1u<<9)  // (M) Card DAT3 got inserted (high).
#define STATUS_DAT3_DETECT       (1u<<10) // Card DAT3 status. 1 = inserted.
#define STATUS_ERR_CMD_IDX       (1u<<16) // (M) Bad CMD index in response.
#define STATUS_ERR_CRC           (1u<<17) // (M) Bad CRC in response.
#define STATUS_ERR_STOP_BIT      (1u<<18) // (M) Stop bit error. Failed to recognize response frame end?
#define STATUS_ERR_DATA_TIMEOUT  (1u<<19) // (M) Response data timeout.
#define STATUS_ERR_RX_OVERF      (1u<<20) // (M) Receive FIFO overflow.
#define STATUS_ERR_TX_UNDERF     (1u<<21) // (M) Send FIFO underflow.
#define STATUS_ERR_CMD_TIMEOUT   (1u<<22) // (M) Response start bit timeout.
#define STATUS_SD_BUSY           (1u<<23) // SD card signals busy if this bit is 0 (DAT0 held low).
#define STATUS_RX_RDY            (1u<<24) // (M) FIFO ready for read.
#define STATUS_TX_REQ            (1u<<25) // (M) FIFO write request.
// Bit 27 is maskable. Purpose unknown.
// Bit 29 exists (not maskable). Signals when clock divider changes are allowed?
#define STATUS_CMD_BUSY          (1u<<30) // Command register busy.
#define STATUS_ERR_ILL_ACC       (1u<<31) // (M) Illegal access error. TODO: What does that mean?

#define STATUS_MASK_ALL          (0xFFFFFFFFu)
#define STATUS_MASK_DEFAULT      ((1u<<27) | STATUS_TX_REQ | STATUS_RX_RDY | \
                                  STATUS_DAT3_INSERT | STATUS_DAT3_REMOVE)
#define STATUS_MASK_ERR          (STATUS_ERR_ILL_ACC | STATUS_ERR_CMD_TIMEOUT | STATUS_ERR_TX_UNDERF | \
                                  STATUS_ERR_RX_OVERF | STATUS_ERR_DATA_TIMEOUT | STATUS_ERR_STOP_BIT | \
                                  STATUS_ERR_CRC | STATUS_ERR_CMD_IDX)

// REG_SD_CLK_CTRL
#define SD_CLK_DIV_2             (0u)    // Clock divider 2.
#define SD_CLK_DIV_4             (1u)    // Clock divider 4.
#define SD_CLK_DIV_8             (1u<<1) // Clock divider 8.
#define SD_CLK_DIV_16            (1u<<2) // Clock divider 16.
#define SD_CLK_DIV_32            (1u<<3) // Clock divider 32.
#define SD_CLK_DIV_64            (1u<<4) // Clock divider 64.
#define SD_CLK_DIV_128           (1u<<5) // Clock divider 128.
#define SD_CLK_DIV_256           (1u<<6) // Clock divider 256.
#define SD_CLK_DIV_512           (1u<<7) // Clock divider 512.
#define SD_CLK_EN                (1u<<8) // Clock enable.
#define SD_CLK_AUTO_OFF          (1u<<9) // Disables clock on idle.
// Bit 10 is writable... at least according to gbatek (can't confirm). Purpose unknown.

// Outputs the matching divider for clk.
// Shift the output right by 2 to get the value for REG_SD_CLK_CTRL.
#define TMIO_CLK2DIV(clk)                        \
({                                               \
	u32 __shift = 1;                             \
	while((clk) < TMIO_HCLK>>__shift) ++__shift; \
	1u<<__shift;                                 \
})

// Clock off by default.
// Nearest possible for 400 kHz is 261.827984375 kHz.
#define SD_CLK_DEFAULT  (TMIO_CLK2DIV(400000)>>2)

// REG_SD_OPTION
// Note on card detection time:
// The card detection timer starts only on inserting cards (including cold boot with inserted card)
// and when mapping ports between controllers. Card power doesn't have any effect on the timer.
//
// Bit 0-3 card detect timer 0x400<<x HCLKs. 0xF timer test (0x100 HCLKs).
// Bit 4-7 data timeout 0x2000<<x SDCLKs. 0xF timeout test (0x100 SDCLKs).
#define OPTION_UNK14             (1u<<14) // "no C2 module" What the fuck is a C2 module?
#define OPTION_BUS_WIDTH4        (0u)     // 4 bit bus width.
#define OPTION_BUS_WIDTH1        (1u<<15) // 1 bit bus width.

// Card detect time: 0x400<<8 / 33513982 = 0.007821929 seconds.
// Data timeout:     0x2000<<11 / (33513982 / 2) = 1.001206959 seconds.
#define OPTION_DEFAULT_TIMINGS   (11u<<4 | 8u)

// REG_SD_ERR_STATUS1/2  Write 0 to acknowledge a bit.
// TODO: Are all of these actually supported on this controller?
#define ERR_RESP_CMD_IDX         (1u)     // Manual command index error in response.
#define ERR_RESP_CMD12_IDX       (1u<<1)  // Auto command index error in response.
#define ERR_RESP_STOP_BIT        (1u<<2)  // Manual command response stop bit error.
#define ERR_RESP_STOP_BIT_CMD12  (1u<<3)  // Auto command response stop bit error.
#define ERR_STOP_BIT_DATA_READ   (1u<<4)  // Stop bit error in read data.
#define ERR_STOP_BIT_WR_CRC      (1u<<5)  // Stop bit error for write CRC status. What the hell does that mean?
#define ERR_CMD_RESP_CRC         (1u<<8)  // Manual command response CRC error.
#define ERR_CMD12_RESP_CRC       (1u<<9)  // Auto command response CRC error.
#define ERR_DATA_READ_CRC        (1u<<10) // CRC error for read data.
#define ERR_WR_CRC_STAT          (1u<<11) // "CRC error for Write CRC status for a write command". What the hell does that mean?
// Bit 13 always 1.
#define ERR_CMD_RESP_TMOUT       (1u<<16) // Manual command response timeout.
#define ERR_CMD12_RESP_TMOUT     (1u<<17) // Auto command response timeout.
// TODO: Add the correct remaining ones.

// REG_SDIO_MODE
#define SDIO_MODE_SDIO_IRQ_EN    (1u)    // SDIO IRQ enable (DAT1 low).
#define SDIO_MODE_UNK2_EN        (1u<<2) // IRQ on "read wait" requests?
#define SDIO_MODE_UNK8           (1u<<8) // Aborts command and data transfer?
#define SDIO_MODE_UNK9           (1u<<9) // Aborts command but not data transfer? CMD52 related.

// REG_SDIO_STATUS       Write 0 to acknowledge a bit.
// REG_SDIO_STATUS_MASK  (M) = Maskable bit. 1 = disabled.
#define SDIO_STATUS_SDIO_IRQ     (1u)     // (M) SDIO IRQ (DAT1 low).
#define SDIO_STATUS_UNK1_IRQ     (1u<<1)  // (M) IRQ once CMD52 can be used after abort?
#define SDIO_STATUS_UNK2_IRQ     (1u<<2)  // (M) Related to SDIO_MODE_UNK2_EN?
#define SDIO_STATUS_UNK14_IRQ    (1u<<14) // (M) Related to SDIO_MODE_UNK9?
#define SDIO_STATUS_UNK15_IRQ    (1u<<15) // (M) Related to SDIO_MODE_UNK2_EN?

#define SDIO_STATUS_MASK_ALL     (0xFFFFu)

// REG_DMA_EXT_MODE
#define DMA_EXT_CPU_MODE         (0u)    // Disables DMA requests. Actually also turns off the 32 bit FIFO.
#define DMA_EXT_DMA_MODE         (1u<<1) // Enables DMA requests.
#define DMA_EXT_UNK5             (1u<<5) // "Buffer status mode"?

// REG_SOFT_RST
#define SOFT_RST_RST             (0u) // Reset.
#define SOFT_RST_NORST           (1u) // No reset.

// REG_EXT_SDIO_IRQ
#define EXT_SDIO_IRQ_P1          (1u)     // Port 1 SDIO IRQ (DAT1 low). Write 0 to acknowledge.
#define EXT_SDIO_IRQ_P2          (1u<<1)  // Port 2 SDIO IRQ (DAT1 low). Write 0 to acknowledge.
#define EXT_SDIO_IRQ_P3          (1u<<2)  // Port 3 SDIO IRQ (DAT1 low). Write 0 to acknowledge.
#define EXT_SDIO_IRQ_P1_EN       (1u<<4)  // Port 1 SDIO IRQ enable (controller).
#define EXT_SDIO_IRQ_P2_EN       (1u<<5)  // Port 2 SDIO IRQ enable (controller).
#define EXT_SDIO_IRQ_P3_EN       (1u<<6)  // Port 3 SDIO IRQ enable (controller).
#define EXT_SDIO_IRQ_P1_MASK     (1u<<8)  // Port 1 SDIO IRQ mask. 1 = disable IRQ (CPU).
#define EXT_SDIO_IRQ_P2_MASK     (1u<<9)  // Port 2 SDIO IRQ mask. 1 = disable IRQ (CPU).
#define EXT_SDIO_IRQ_P3_MASK     (1u<<10) // Port 3 SDIO IRQ mask. 1 = disable IRQ (CPU).

#define EXT_SDIO_IRQ_MASK_ALL    (EXT_SDIO_IRQ_P3_MASK | EXT_SDIO_IRQ_P2_MASK | EXT_SDIO_IRQ_P1_MASK)

// REG_EXT_WRPROT  Each bit 1 = write protected unlike SD_STATUS.
#define EXT_WRPROT_P1            (1u)
#define EXT_WRPROT_P2            (1u<<1)
#define EXT_WRPROT_P3            (1u<<2)

// REG_EXT_CDET       Acknowledgeable?
// REG_EXT_CDET_MASK  (M) = Maskable bit. 1 = disabled (no IRQ).
#define EXT_CDET_P1_REMOVE       (1u)    // (M) Port 1 card got removed.
#define EXT_CDET_P1_INSERT       (1u<<1) // (M) Port 1 card got inserted. TODO: With detection timer?
#define EXT_CDET_P1_DETECT       (1u<<2) // Port 1 card detect status. 1 = inserted. TODO: With detection timer?
#define EXT_CDET_P2_REMOVE       (1u<<3) // (M) Port 2 card got removed.
#define EXT_CDET_P2_INSERT       (1u<<4) // (M) Port 2 card got inserted. TODO: With detection timer?
#define EXT_CDET_P2_DETECT       (1u<<5) // Port 2 card detect status. 1 = inserted. TODO: With detection timer?
#define EXT_CDET_P3_REMOVE       (1u<<6) // (M) Port 3 card got removed.
#define EXT_CDET_P3_INSERT       (1u<<7) // (M) Port 3 card got inserted. TODO: With detection timer?
#define EXT_CDET_P3_DETECT       (1u<<8) // Port 3 card detect status. 1 = inserted. TODO: With detection timer?

#define EXT_CDET_MASK_ALL        (0xFFFFu)

// REG_EXT_CDET_DAT3       Acknowledgeable?
// REG_EXT_CDET_DAT3_MASK  (M) = Maskable bit. 1 = disabled (no IRQ).
#define EXT_CDET_DAT3_P1_REMOVE  (1u)    // (M) Port 1 card DAT3 got removed (low).
#define EXT_CDET_DAT3_P1_INSERT  (1u<<1) // (M) Port 1 card DAT3 got inserted (high).
#define EXT_CDET_DAT3_P1_DETECT  (1u<<2) // Port 1 card DAT3 status. 1 = inserted.
#define EXT_CDET_DAT3_P2_REMOVE  (1u<<3) // (M) Port 2 card DAT3 got removed (low).
#define EXT_CDET_DAT3_P2_INSERT  (1u<<4) // (M) Port 2 card DAT3 got inserted (high).
#define EXT_CDET_DAT3_P2_DETECT  (1u<<5) // Port 2 card DAT3 status. 1 = inserted.
#define EXT_CDET_DAT3_P3_REMOVE  (1u<<6) // (M) Port 3 card DAT3 got removed (low).
#define EXT_CDET_DAT3_P3_INSERT  (1u<<7) // (M) Port 3 card DAT3 got inserted (high).
#define EXT_CDET_DAT3_P3_DETECT  (1u<<8) // Port 3 card DAT3 status. 1 = inserted.

#define EXT_CDET_DAT3_MASK_ALL   (0xFFFFu)

// REG_SD_FIFO32_CNT
// Bit 0 unknown, non-writable.
#define FIFO32_EN                (1u<<1)  // Enables the 32 bit FIFO.
#define FIFO32_FULL              (1u<<8)  // FIFO is full.
#define FIFO32_NOT_EMPTY         (1u<<9)  // FIFO is not empty. Inverted bit. 0 means empty.
#define FIFO32_CLEAR             (1u<<10) // Clears the FIFO.
#define FIFO32_FULL_IE           (1u<<11) // FIFO full IRQ enable.
#define FIFO32_NOT_EMPTY_IE      (1u<<12) // FIFO not empty IRQ enable.



typedef struct
{
	u8 portNum;
	u16 sd_clk_ctrl;
	u16 sd_blocklen; // Also sd_blocklen32.
	u16 sd_option;
	u32 *buf;
	u16 blocks;
	u32 resp[4];     // Little endian, MSB first.
} TmioPort;



/**
 * @brief      Initializes the tmio driver.
 */
void TMIO_init(void);

/**
 * @brief      Deinitializes the tmio driver.
 */
void TMIO_deinit(void);

/**
 * @brief      Initializes a tmio port to defaults.
 *
 * @param      port     A pointer to the port struct.
 * @param[in]  portNum  The port number.
 */
void TMIO_initPort(TmioPort *const port, const u8 portNum);

/**
 * @brief      Checks if a MMC/SD card is inserted.
 *
 * @return     Returns true if a card is inserted.
 */
bool TMIO_cardDetected(void);

/**
 * @brief      Checks if the write protect slider is set to locked.
 *
 * @return     Returns true if the card is unlocked.
 */
bool TMIO_cardWritable(void);

/**
 * @brief      Outputs a continuous clock for initialization.
 *
 * @param      port  A pointer to the port struct.
 * @param[in]  clk   The target clock in Hz. Usually 400 kHz.
 */
void TMIO_startInitClock(TmioPort *const port, const u32 clk);

/**
 * @brief      Sends a command.
 *
 * @param      port  A pointer to the port struct.
 * @param[in]  cmd   The command.
 * @param[in]  arg   The argument for the command.
 *
 * @return     Returns 0 on success otherwise see REG_SD_STATUS1/2 bits.
 */
u32 TMIO_sendCommand(TmioPort *const port, const u16 cmd, const u32 arg);

/**
 * @brief      Sets the clock for a tmio port.
 *
 * @param      port  A pointer to the port struct.
 * @param[in]  clk   The target clock in Hz.
 */
__attribute__((always_inline)) static inline void TMIO_setClock(TmioPort *const port, const u32 clk)
{
	port->sd_clk_ctrl = SD_CLK_AUTO_OFF | SD_CLK_EN | TMIO_CLK2DIV(clk)>>2;
}

/**
 * @brief      Sets the transfer block length for a tmio port.
 *
 * @param      port      A pointer to the port struct.
 * @param[in]  blockLen  The block length.
 */
__attribute__((always_inline)) static inline void TMIO_setBlockLen(TmioPort *const port, u16 blockLen)
{
	if(blockLen > 512)       blockLen = 512;
	if(blockLen < 16)        blockLen = 0; // | Depends on doCpuTransfer() in tmio.c.
	if((blockLen % 16) != 0) blockLen = 0; // |

	port->sd_blocklen = blockLen;
}

/**
 * @brief      Sets the bus width for a tmio port.
 *
 * @param      port   A pointer to the port struct.
 * @param[in]  width  The bus width.
 */
__attribute__((always_inline)) static inline void TMIO_setBusWidth(TmioPort *const port, const u8 width)
{
	port->sd_option = (width == 4 ? OPTION_BUS_WIDTH4 : OPTION_BUS_WIDTH1) |
	                  OPTION_UNK14 | OPTION_DEFAULT_TIMINGS;
}

/**
 * @brief      Sets a transfer buffer for a tmio port.
 *
 * @param      port    A pointer to the port struct.
 * @param      buf     The buffer pointer.
 * @param[in]  blocks  The number of blocks to transfer.
 */
__attribute__((always_inline)) static inline void TMIO_setBuffer(TmioPort *const port, u32 *buf, const u16 blocks)
{
	port->buf    = buf;
	port->blocks = blocks;
}
