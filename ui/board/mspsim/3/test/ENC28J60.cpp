/*
reset && (export GCC_DIR=~/ti/msp430_gcc ; 
    $GCC_DIR/bin/msp430-elf-c++ \
    -I $GCC_DIR/include -L $GCC_DIR/include \
    -Werror -Wall -mmcu=msp430f1611 -O2 ENC28J60.cpp -o ENC28J60_10.0.0.42.out)
*/

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

void uart_print(const char *str) {
  while (*str) {
    while (!(IFG1 & UTXIFG0)); // USART0 TX buffer ready?
    TXBUF0 = *str++;
  }
}

void uart_printf(const char* format, ...) {
  static char dest[1024];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(dest, format, argptr);
  va_end(argptr);
  uart_print(dest);
}

// slaa281

#define halSPIRXBUF  U1RXBUF
#define halSPI_SEND(x) U1TXBUF=x
#define halSPITXREADY  (IFG2&UTXIFG1)         /* Wait for TX to be ready */
#define halSPITXDONE  (U1TCTL&TXEPT)          /* Wait for TX to finish */
#define halSPIRXREADY (IFG2&URXIFG1)          /* Wait for TX to be ready */
#define halSPIRXFG_CLR IFG2 &= ~URXIFG1
#define halSPI_PxIN  SPI_USART1_PxIN
#define halSPI_SOMI  SPI_USART1_SOMI

// SPI port definitions              // Adjust the values for the chosen
#define SPI_PxSEL         P5SEL      // interfaces, according to the pin
#define SPI_PxDIR         P5DIR      // assignments indicated in the
#define SPI_PxIN          P5IN       // chosen MSP430 device datasheet.
#define SPI_PxOUT         P5OUT
#define SPI_SIMO          0x02
#define SPI_SOMI          0x04
#define SPI_UCLK          0x08

// SPI port definitions              // Adjust the values for the chosen
#define MMC_PxSEL         SPI_PxSEL  // interfaces, according to the pin
#define MMC_PxDIR         SPI_PxDIR  // assignments indicated in the
#define MMC_PxIN          SPI_PxIN   // chosen MSP430 device datasheet.
#define MMC_PxOUT         SPI_PxOUT      
#define MMC_SIMO          SPI_SIMO
#define MMC_SOMI          SPI_SOMI
#define MMC_UCLK          SPI_UCLK

// Chip Select
#define MMC_CS_PxOUT      P5OUT
#define MMC_CS_PxDIR      P5DIR
#define MMC_CS            0x01

#define CS_LOW()          MMC_CS_PxOUT &= ~MMC_CS               // Card Select
#define CS_HIGH()         while(!halSPITXDONE); MMC_CS_PxOUT |= MMC_CS  // Card Deselect


void halSPISetup(void)
{
  UCTL1 = CHAR + SYNC + MM + SWRST;         // 8-bit SPI Master **SWRST**
  UTCTL1 = CKPL + SSEL1 + SSEL0 + STC;      // SMCLK, 3-pin mode
  UBR01 = 0x02;                             // UCLK/2
  UBR11 = 0x00;                             // 0
  UMCTL1 = 0x00;                            // No modulation
  ME2 |= USPIE1;                            // Enable USART1 SPI mode
  UCTL1 &= ~SWRST;                          // Initialize USART state machine
}

//Send one byte via SPI
unsigned char spiSendByte(const unsigned char data)
{
  while ( halSPITXREADY == 0 );    // wait while not ready for TX
  halSPI_SEND(data);               // write
  while ( halSPIRXREADY == 0 );    // wait for RX buffer (full)
  return (halSPIRXBUF);
}

// http://we.easyelectronics.ru/electro-and-pc/podklyuchenie-mikrokontrollera-k-lokalnoy-seti-rabotaem-s-enc28j60.html

#define enc28j60_select()  CS_LOW()
#define enc28j60_release() CS_HIGH()

#define enc28j60_rx()     spiSendByte(0xff)
#define enc28j60_tx(data) spiSendByte(data)

#define ENC28J60_BUFSIZE  0x2000
#define ENC28J60_RXSIZE   0x1A00
#define ENC28J60_BUFEND   (ENC28J60_BUFSIZE-1)

#define ENC28J60_MAXFRAME 1500

#define ENC28J60_RXSTART  0
#define ENC28J60_RXEND    (ENC28J60_RXSIZE-1)
#define ENC28J60_TXSTART  ENC28J60_RXSIZE

#define ENC28J60_SPI_RCR  0x00
#define ENC28J60_SPI_RBM  0x3A
#define ENC28J60_SPI_WCR  0x40
#define ENC28J60_SPI_WBM  0x7A
#define ENC28J60_SPI_BFS  0x80
#define ENC28J60_SPI_BFC  0xA0
#define ENC28J60_SPI_SC   0xFF

#define ENC28J60_ADDR_MASK  0x1F
#define ENC28J60_COMMON_CR  0x1B

/*
 * Main registers
 */
 
#define EIE         0x1B
#define EIR         0x1C
#define ESTAT         0x1D
#define ECON2         0x1E
#define ECON1         0x1F


// Buffer read pointer
#define ERDPTL        0x00
#define ERDPTH        0x01
#define ERDPT       ERDPTL

// Buffer write pointer
#define EWRPTL        0x02
#define EWRPTH        0x03
#define EWRPT       EWRPTL

// Tx packet start pointer
#define ETXSTL        0x04
#define ETXSTH        0x05
#define ETXST       ETXSTL

// Tx packet end pointer
#define ETXNDL        0x06
#define ETXNDH        0x07
#define ETXND       ETXNDL

// Rx FIFO start pointer
#define ERXSTL        0x08
#define ERXSTH        0x09
#define ERXST       ERXSTL

// Rx FIFO end pointer
#define ERXNDL        0x0A
#define ERXNDH        0x0B
#define ERXND       ERXNDL

// Rx FIFO read pointer
#define ERXRDPTL      0x0C
#define ERXRDPTH      0x0D
#define ERXRDPT       ERXRDPTL

// Rx FIFO write pointer
#define ERXWRPTL      0x0E
#define ERXWRPTH      0x0F
#define ERXWRPT       ERXWRPTL

// DMA source block start pointer
#define EDMASTL       0x10
#define EDMASTH       0x11
#define EDMAST        EDMASTL

// DMA source block end pointer
#define EDMANDL       0x12
#define EDMANDH       0x13
#define EDMAND        EDMANDL

// DMA destination pointer
#define EDMADSTL      0x14
#define EDMADSTH      0x15
#define EDMADST       EDMADSTL

// DMA checksum
#define EDMACSL       0x16
#define EDMACSH       0x17
#define EDMACS        EDMACSL

// Hash table registers 
#define EHT0        (0x00 | 0x20)
#define EHT1        (0x01 | 0x20)
#define EHT2        (0x02 | 0x20)
#define EHT3        (0x03 | 0x20)
#define EHT4        (0x04 | 0x20)
#define EHT5        (0x05 | 0x20)
#define EHT6        (0x06 | 0x20)
#define EHT7        (0x07 | 0x20)

// Pattern match registers
#define EPMM0         (0x08 | 0x20)
#define EPMM1         (0x09 | 0x20)
#define EPMM2         (0x0A | 0x20)
#define EPMM3         (0x0B | 0x20)
#define EPMM4         (0x0C | 0x20)
#define EPMM5         (0x0D | 0x20)
#define EPMM6         (0x0E | 0x20)
#define EPMM7         (0x0F | 0x20)
#define EPMCSL        (0x10 | 0x20)
#define EPMCSH        (0x11 | 0x20)
#define EPMOL         (0x14 | 0x20)
#define EPMOH         (0x15 | 0x20)

// Wake-on-LAN interrupt registers
#define EWOLIE        (0x16 | 0x20)
#define EWOLIR        (0x17 | 0x20)

// Receive filters mask
#define ERXFCON       (0x18 | 0x20)

// Packet counter
#define EPKTCNT       (0x19 | 0x20)


// MAC control registers
#define MACON1        (0x00 | 0x40 | 0x80)
#define MACON2        (0x01 | 0x40 | 0x80)
#define MACON3        (0x02 | 0x40 | 0x80)
#define MACON4        (0x03 | 0x40 | 0x80)

// MAC Back-to-back gap
#define MABBIPG       (0x04 | 0x40 | 0x80)

// MAC Non back-to-back gap
#define MAIPGL        (0x06 | 0x40 | 0x80)
#define MAIPGH        (0x07 | 0x40 | 0x80)

// Collision window & rexmit timer
#define MACLCON1      (0x08 | 0x40 | 0x80)
#define MACLCON2      (0x09 | 0x40 | 0x80)

// Max frame length
#define MAMXFLL       (0x0A | 0x40 | 0x80)
#define MAMXFLH       (0x0B | 0x40 | 0x80)
#define MAMXFL        MAMXFLL

// MAC-PHY support register
#define MAPHSUP       (0x0D | 0x40 | 0x80)
#define MICON         (0x11 | 0x40 | 0x80)

// MII registers
#define MICMD         (0x12 | 0x40 | 0x80)
#define MIREGADR      (0x14 | 0x40 | 0x80)

#define MIWRL         (0x16 | 0x40 | 0x80)
#define MIWRH         (0x17 | 0x40 | 0x80)
#define MIWR        MIWRL

#define MIRDL         (0x18 | 0x40 | 0x80)
#define MIRDH         (0x19 | 0x40 | 0x80)
#define MIRD        MIRDL

// MAC Address
#define MAADR1        (0x00 | 0x60 | 0x80)
#define MAADR0        (0x01 | 0x60 | 0x80)
#define MAADR3        (0x02 | 0x60 | 0x80)
#define MAADR2        (0x03 | 0x60 | 0x80)
#define MAADR5        (0x04 | 0x60 | 0x80)
#define MAADR4        (0x05 | 0x60 | 0x80)

// Built-in self-test
#define EBSTSD        (0x06 | 0x60)
#define EBSTCON       (0x07 | 0x60)
#define EBSTCSL       (0x08 | 0x60)
#define EBSTCSH       (0x09 | 0x60)
#define MISTAT        (0x0A | 0x60 | 0x80)

// Revision ID
#define EREVID        (0x12 | 0x60)

// Clock output control register
#define ECOCON        (0x15 | 0x60)

// Flow control registers
#define EFLOCON       (0x17 | 0x60)
#define EPAUSL        (0x18 | 0x60)
#define EPAUSH        (0x19 | 0x60)

// PHY registers
#define PHCON1        0x00
#define PHSTAT1       0x01
#define PHID1         0x02
#define PHID2         0x03
#define PHCON2        0x10
#define PHSTAT2       0x11
#define PHIE        0x12
#define PHIR        0x13
#define PHLCON        0x14

// EIE
#define EIE_INTIE     0x80
#define EIE_PKTIE     0x40
#define EIE_DMAIE     0x20
#define EIE_LINKIE      0x10
#define EIE_TXIE      0x08
#define EIE_WOLIE     0x04
#define EIE_TXERIE      0x02
#define EIE_RXERIE      0x01

// EIR
#define EIR_PKTIF     0x40
#define EIR_DMAIF     0x20
#define EIR_LINKIF      0x10
#define EIR_TXIF      0x08
#define EIR_WOLIF     0x04
#define EIR_TXERIF      0x02
#define EIR_RXERIF      0x01

// ESTAT
#define ESTAT_INT     0x80
#define ESTAT_LATECOL   0x10
#define ESTAT_RXBUSY    0x04
#define ESTAT_TXABRT    0x02
#define ESTAT_CLKRDY    0x01

// ECON2
#define ECON2_AUTOINC   0x80
#define ECON2_PKTDEC    0x40
#define ECON2_PWRSV     0x20
#define ECON2_VRPS      0x08

// ECON1
#define ECON1_TXRST     0x80
#define ECON1_RXRST     0x40
#define ECON1_DMAST     0x20
#define ECON1_CSUMEN    0x10
#define ECON1_TXRTS     0x08
#define ECON1_RXEN      0x04
#define ECON1_BSEL1     0x02
#define ECON1_BSEL0     0x01

// EWOLIE
#define EWOLIE_UCWOLIE    0x80
#define EWOLIE_AWOLIE   0x40
#define EWOLIE_PMWOLIE    0x10
#define EWOLIE_MPWOLIE    0x08
#define EWOLIE_HTWOLIE    0x04
#define EWOLIE_MCWOLIE    0x02
#define EWOLIE_BCWOLIE    0x01

// EWOLIR
#define EWOLIR_UCWOLIF    0x80
#define EWOLIR_AWOLIF   0x40
#define EWOLIR_PMWOLIF    0x10
#define EWOLIR_MPWOLIF    0x08
#define EWOLIR_HTWOLIF    0x04
#define EWOLIR_MCWOLIF    0x02
#define EWOLIR_BCWOLIF    0x01

// ERXFCON
#define ERXFCON_UCEN    0x80
#define ERXFCON_ANDOR   0x40
#define ERXFCON_CRCEN   0x20
#define ERXFCON_PMEN    0x10
#define ERXFCON_MPEN    0x08
#define ERXFCON_HTEN    0x04
#define ERXFCON_MCEN    0x02
#define ERXFCON_BCEN    0x01

// MACON1
#define MACON1_LOOPBK   0x10
#define MACON1_TXPAUS   0x08
#define MACON1_RXPAUS   0x04
#define MACON1_PASSALL    0x02
#define MACON1_MARXEN   0x01

// MACON2
#define MACON2_MARST    0x80
#define MACON2_RNDRST   0x40
#define MACON2_MARXRST    0x08
#define MACON2_RFUNRST    0x04
#define MACON2_MATXRST    0x02
#define MACON2_TFUNRST    0x01

// MACON3
#define MACON3_PADCFG2    0x80
#define MACON3_PADCFG1    0x40
#define MACON3_PADCFG0    0x20
#define MACON3_TXCRCEN    0x10
#define MACON3_PHDRLEN    0x08
#define MACON3_HFRMEN   0x04
#define MACON3_FRMLNEN    0x02
#define MACON3_FULDPX   0x01

// MACON4
#define MACON4_DEFER    0x40
#define MACON4_BPEN     0x20
#define MACON4_NOBKOFF    0x10
#define MACON4_LONGPRE    0x02
#define MACON4_PUREPRE    0x01

// MAPHSUP
#define MAPHSUP_RSTINTFC  0x80
#define MAPHSUP_RSTRMII   0x08

// MICON
#define MICON_RSTMII    0x80

// MICMD
#define MICMD_MIISCAN   0x02
#define MICMD_MIIRD     0x01

// EBSTCON
#define EBSTCON_PSV2    0x80
#define EBSTCON_PSV1    0x40
#define EBSTCON_PSV0    0x20
#define EBSTCON_PSEL    0x10
#define EBSTCON_TMSEL1    0x08
#define EBSTCON_TMSEL0    0x04
#define EBSTCON_TME     0x02
#define EBSTCON_BISTST    0x01

// MISTAT
#define MISTAT_NVALID   0x04
#define MISTAT_SCAN     0x02
#define MISTAT_BUSY     0x01

// ECOCON
#define ECOCON_COCON2   0x04
#define ECOCON_COCON1   0x02
#define ECOCON_COCON0   0x01

// EFLOCON
#define EFLOCON_FULDPXS   0x04
#define EFLOCON_FCEN1   0x02
#define EFLOCON_FCEN0   0x01

// PHCON1
#define PHCON1_PRST     0x8000
#define PHCON1_PLOOPBK    0x4000
#define PHCON1_PPWRSV   0x0800
#define PHCON1_PDPXMD   0x0100

// PHSTAT1
#define PHSTAT1_PFDPX   0x1000
#define PHSTAT1_PHDPX   0x0800
#define PHSTAT1_LLSTAT    0x0004
#define PHSTAT1_JBSTAT    0x0002

// PHCON2
#define PHCON2_FRCLNK   0x4000
#define PHCON2_TXDIS    0x2000
#define PHCON2_JABBER   0x0400
#define PHCON2_HDLDIS   0x0100

// PHSTAT2
#define PHSTAT2_TXSTAT    0x2000
#define PHSTAT2_RXSTAT    0x1000
#define PHSTAT2_COLSTAT   0x0800
#define PHSTAT2_LSTAT   0x0400
#define PHSTAT2_DPXSTAT   0x0200
#define PHSTAT2_PLRITY    0x0010

// PHIE
#define PHIE_PLNKIE     0x0010
#define PHIE_PGEIE      0x0002

// PHIR
#define PHIR_PLNKIF     0x0010
#define PHIR_PGIF     0x0004

// PHLCON
#define PHLCON_LACFG3   0x0800
#define PHLCON_LACFG2   0x0400
#define PHLCON_LACFG1   0x0200
#define PHLCON_LACFG0   0x0100
#define PHLCON_LBCFG3   0x0080
#define PHLCON_LBCFG2   0x0040
#define PHLCON_LBCFG1   0x0020
#define PHLCON_LBCFG0   0x0010
#define PHLCON_LFRQ1    0x0008
#define PHLCON_LFRQ0    0x0004
#define PHLCON_STRCH    0x0002

// Generic SPI read command
uint8_t enc28j60_read_op(uint8_t cmd, uint8_t adr)
{
  uint8_t data;

  enc28j60_select();
  enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
  if(adr & 0x80) // throw out dummy byte 
    enc28j60_rx(); // when reading MII/MAC register
  data = enc28j60_rx();
  enc28j60_release();
  return data;
}

// Generic SPI write command
void enc28j60_write_op(uint8_t cmd, uint8_t adr, uint8_t data)
{
  enc28j60_select();
  enc28j60_tx(cmd | (adr & ENC28J60_ADDR_MASK));
  enc28j60_tx(data);
  enc28j60_release();
}

uint8_t enc28j60_current_bank = 0;
uint16_t enc28j60_rxrdpt = 0;

// Initiate software reset
void enc28j60_soft_reset()
{
  enc28j60_select();
  enc28j60_tx(ENC28J60_SPI_SC);
  enc28j60_release();
  
  enc28j60_current_bank = 0;
  //_delay_ms(1); // Wait until device initializes
}

/*
 * Memory access
 */

// Set register bank
void enc28j60_set_bank(uint8_t adr)
{
  uint8_t bank;

  if( (adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR )
  {
    bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
    if(bank != enc28j60_current_bank)
    {
      enc28j60_write_op(ENC28J60_SPI_BFC, ECON1, 0x03);
      enc28j60_write_op(ENC28J60_SPI_BFS, ECON1, bank);
      enc28j60_current_bank = bank;
    }
  }
}

// Read register
uint8_t enc28j60_rcr(uint8_t adr)
{
  enc28j60_set_bank(adr);
  return enc28j60_read_op(ENC28J60_SPI_RCR, adr);
}

// Read register pair
uint16_t enc28j60_rcr16(uint8_t adr)
{
  enc28j60_set_bank(adr);
  return enc28j60_read_op(ENC28J60_SPI_RCR, adr) |
    (enc28j60_read_op(ENC28J60_SPI_RCR, adr+1) << 8);
}

// Write register
void enc28j60_wcr(uint8_t adr, uint8_t arg)
{
  enc28j60_set_bank(adr);
  enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void enc28j60_wcr16(uint8_t adr, uint16_t arg)
{
  enc28j60_set_bank(adr);
  enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
  enc28j60_write_op(ENC28J60_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void enc28j60_bfc(uint8_t adr, uint8_t mask)
{
  enc28j60_set_bank(adr);
  enc28j60_write_op(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void enc28j60_bfs(uint8_t adr, uint8_t mask)
{
  enc28j60_set_bank(adr);
  enc28j60_write_op(ENC28J60_SPI_BFS, adr, mask);
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_buffer(uint8_t *buf, uint16_t len)
{
  enc28j60_select();
  enc28j60_tx(ENC28J60_SPI_RBM);
  while(len--)
    *(buf++) = enc28j60_rx();
  enc28j60_release();
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_buffer(uint8_t *buf, uint16_t len)
{
  enc28j60_select();
  enc28j60_tx(ENC28J60_SPI_WBM);
  while(len--)
    enc28j60_tx(*(buf++));
  enc28j60_release();
}

// Read PHY register
uint16_t enc28j60_read_phy(uint8_t adr)
{
  enc28j60_wcr(MIREGADR, adr);
  enc28j60_bfs(MICMD, MICMD_MIIRD);
  while(enc28j60_rcr(MISTAT) & MISTAT_BUSY)
    ;
  enc28j60_bfc(MICMD, MICMD_MIIRD);
  return enc28j60_rcr16(MIRD);
}

// Write PHY register
void enc28j60_write_phy(uint8_t adr, uint16_t data)
{
  enc28j60_wcr(MIREGADR, adr);
  enc28j60_wcr16(MIWR, data);
  while(enc28j60_rcr(MISTAT) & MISTAT_BUSY)
    ;
}


/*
 * Init & packet Rx/Tx
 */

void enc28j60_init(const uint8_t *macadr)
{
  // Reset ENC28J60
  enc28j60_soft_reset();

  while(!(enc28j60_rcr(ESTAT) & ESTAT_CLKRDY));

  // Setup Rx/Tx buffer
  enc28j60_wcr16(ERXST, ENC28J60_RXSTART);
  enc28j60_wcr16(ERXRDPT, ENC28J60_RXSTART);
  enc28j60_wcr16(ERXND, ENC28J60_RXEND);
  enc28j60_rxrdpt = ENC28J60_RXSTART;

  // Setup MAC
  enc28j60_wcr(MACON1, MACON1_TXPAUS| // Enable flow control
    MACON1_RXPAUS|MACON1_MARXEN); // Enable MAC Rx
  enc28j60_wcr(MACON2, 0); // Clear reset
  enc28j60_wcr(MACON3, MACON3_PADCFG0| // Enable padding,
    MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX); // Enable crc & frame len chk
  enc28j60_wcr16(MAMXFL, ENC28J60_MAXFRAME);
  enc28j60_wcr(MABBIPG, 0x15); // Set inter-frame gap
  enc28j60_wcr(MAIPGL, 0x12);
  enc28j60_wcr(MAIPGH, 0x0c);
  enc28j60_wcr(MAADR5, macadr[0]); // Set MAC address
  enc28j60_wcr(MAADR4, macadr[1]);
  enc28j60_wcr(MAADR3, macadr[2]);
  enc28j60_wcr(MAADR2, macadr[3]);
  enc28j60_wcr(MAADR1, macadr[4]);
  enc28j60_wcr(MAADR0, macadr[5]);

  // Setup PHY
  enc28j60_write_phy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
  enc28j60_write_phy(PHCON2, PHCON2_HDLDIS); // Disable loopback
  enc28j60_write_phy(PHLCON, PHLCON_LACFG2| // Configure LED ctrl
    PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|
    PHLCON_LFRQ0|PHLCON_STRCH);

  // Enable Rx packets
  enc28j60_bfs(ECON1, ECON1_RXEN);
}

void enc28j60_send_packet(uint8_t *data, uint16_t len)
{
  while(enc28j60_rcr(ECON1) & ECON1_TXRTS)
  {
    // TXRTS may not clear - ENC28J60 bug. We must reset
    // transmit logic in cause of Tx error
    if(enc28j60_rcr(EIR) & EIR_TXERIF)
    {
      enc28j60_bfs(ECON1, ECON1_TXRST);
      enc28j60_bfc(ECON1, ECON1_TXRST);
    }
  }

  enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);
  enc28j60_write_buffer((uint8_t*)"\x00", 1);
  enc28j60_write_buffer(data, len);

  enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
  enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len);

  enc28j60_bfs(ECON1, ECON1_TXRTS); // Request packet send
}

uint16_t enc28j60_recv_packet(uint8_t *buf, uint16_t buflen)
{
  uint16_t len = 0, rxlen, status, temp;

  if(enc28j60_rcr(EPKTCNT))
  {
    enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

    enc28j60_read_buffer((uint8_t*)&enc28j60_rxrdpt, sizeof(enc28j60_rxrdpt));
    enc28j60_read_buffer((uint8_t*)&rxlen, sizeof(rxlen));
    enc28j60_read_buffer((uint8_t*)&status, sizeof(status));

    if(status & 0x80) //success
    {
      len = rxlen - 4; //throw out crc
      if(len > buflen) len = buflen;
      enc28j60_read_buffer(buf, len); 
    }

    // Set Rx read pointer to next packet
    temp = (enc28j60_rxrdpt - 1) & ENC28J60_BUFEND;
    enc28j60_wcr16(ERXRDPT, temp);

    // Decrement packet counter
    enc28j60_bfs(ECON2, ECON2_PKTDEC);
  }

  return len;
}

typedef struct eth_frame {
  uint8_t to_addr_mac[6];
  uint8_t from_addr_mac[6];
  uint16_t type;
  uint8_t data[];
} __attribute__((packed)) eth_frame_t;

#define htons(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define ntohs(n) htons(n)

#define htonl(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))

#define ntohl(n) htonl(n)

#define inet_addr(a,b,c,d)  ( ((uint32_t)a) | ((uint32_t)b << 8) |\
                ((uint32_t)c << 16) | ((uint32_t)d << 24) )

#define ETH_TYPE_ARP    htons(0x0806)
#define ETH_TYPE_IP     htons(0x0800)

uint8_t net_buf[ENC28J60_MAXFRAME];
const uint32_t ip_addr = inet_addr(10,0,0,42);
const uint8_t mac_addr[6] = {0x00,0x13,0x37,0x01,0x23,0x45};

void eth_reply(eth_frame_t *frame, uint16_t len)
{
  uint8_t mac_addr[6];
  memcpy(mac_addr, frame->to_addr_mac, 6);
  memcpy(frame->to_addr_mac, frame->from_addr_mac, 6);
  memcpy(frame->from_addr_mac, mac_addr, 6);
  enc28j60_send_packet((uint8_t*)frame, len + sizeof(eth_frame_t));
}

/*
 * ARP
 */

#define ARP_HW_TYPE_ETH   htons(0x0001)
#define ARP_PROTO_TYPE_IP htons(0x0800)

#define ARP_TYPE_REQUEST  htons(1)
#define ARP_TYPE_RESPONSE htons(2)

typedef struct arp_message {
  uint16_t hw_type;
  uint16_t proto_type;
  uint8_t hw_addr_len;
  uint8_t proto_addr_len;
  uint16_t type;
  uint8_t mac_addr_from[6];
  uint32_t ip_addr_from;
  uint8_t mac_addr_to[6];
  uint32_t ip_addr_to;
} __attribute__((packed)) arp_message_t;

void arp_filter(eth_frame_t *frame, uint16_t len)
{
  arp_message_t *msg = (arp_message_t*)frame->data;

  if(len >= sizeof(arp_message_t))
  {
    if( (msg->hw_type == ARP_HW_TYPE_ETH) &&
      (msg->proto_type == ARP_PROTO_TYPE_IP) )
    {



      if( (msg->type == ARP_TYPE_REQUEST) && 
        (msg->ip_addr_to == ip_addr) )
      {
        msg->type = ARP_TYPE_RESPONSE;
        memcpy(msg->mac_addr_to, msg->mac_addr_from, 6);
        memcpy(msg->mac_addr_from, mac_addr, 6);
        msg->ip_addr_to = msg->ip_addr_from;
        msg->ip_addr_from = ip_addr;

        eth_reply(frame, sizeof(arp_message_t));
      }
    }
  }
}

/*
 * ICMP
 */

#define ICMP_TYPE_ECHO_RQ 8
#define ICMP_TYPE_ECHO_RPLY 0

typedef struct icmp_echo_packet {
  uint8_t type;
  uint8_t code;
  uint16_t cksum;
  uint16_t id;
  uint16_t seq;
  uint8_t data[];
} __attribute__((packed)) icmp_echo_packet_t;

#define IP_PROTOCOL_ICMP  1
#define IP_PROTOCOL_TCP   6
#define IP_PROTOCOL_UDP   17

typedef struct ip_packet {
  uint8_t ver_head_len;
  uint8_t tos;
  uint16_t total_len;
  uint16_t fragment_id;
  uint16_t flags_framgent_offset;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t cksum;
  uint32_t from_addr;
  uint32_t to_addr;
  uint8_t data[];
} __attribute__((packed)) ip_packet_t;

#define ICMP_TYPE_ECHO_RQ 8
#define ICMP_TYPE_ECHO_RPLY 0

#define IP_PACKET_TTL   64

uint16_t ip_cksum(uint32_t sum, uint8_t *buf, size_t len)
{
  while(len >= 2)
  {
    sum += ((uint16_t)*buf << 8) | *(buf+1);
    buf += 2;
    len -= 2;
  }

  if(len)
    sum += (uint16_t)*buf << 8;

  while(sum >> 16)
    sum = (sum & 0xffff) + (sum >> 16);

  return ~htons((uint16_t)sum);
}

void ip_reply(eth_frame_t *frame, uint16_t len)
{
  ip_packet_t *packet = (ip_packet_t*)(frame->data);

  packet->total_len = htons(len + sizeof(ip_packet_t));
  packet->fragment_id = 0;
  packet->flags_framgent_offset = 0;
  packet->ttl = IP_PACKET_TTL;
  packet->cksum = 0;
  packet->to_addr = packet->from_addr;
  packet->from_addr = ip_addr;
  packet->cksum = ip_cksum(0, (uint8_t*)packet, sizeof(ip_packet_t));

  eth_reply(frame, len + sizeof(ip_packet_t));
}

void icmp_filter(eth_frame_t *frame, uint16_t len)
{
  ip_packet_t *packet = (ip_packet_t*)frame->data;
  icmp_echo_packet_t *icmp = (icmp_echo_packet_t*)packet->data;

  if(len >= sizeof(icmp_echo_packet_t) )
  {
    if(icmp->type == ICMP_TYPE_ECHO_RQ)
    {
      icmp->type = ICMP_TYPE_ECHO_RPLY;
      icmp->cksum += 8; // update cksum
      ip_reply(frame, len);
    }
  }
}

void ip_filter(eth_frame_t *frame, uint16_t len)
{
  ip_packet_t *packet = (ip_packet_t*)(frame->data);

  if( (packet->ver_head_len == 0x45) && (packet->to_addr == ip_addr) )
  {
    len = ntohs(packet->total_len) - sizeof(ip_packet_t);

    switch(packet->protocol)
    {
    case IP_PROTOCOL_ICMP:
      icmp_filter(frame, len);
      break;
    case IP_PROTOCOL_UDP:
      //udp_filter(frame, len);
      uart_print("udp_filter not implemented\n");
      break;
    }
  }
}

void eth_filter(eth_frame_t *frame, uint16_t len)
{
  if(len >= sizeof(eth_frame_t))
  {
    switch(frame->type)
    {
    case ETH_TYPE_ARP:
      arp_filter(frame, len - sizeof(eth_frame_t));
      break;
    case ETH_TYPE_IP:
      ip_filter(frame, len - sizeof(eth_frame_t));
      break;
    }
  }
}

int main(void) {
  WDTCTL = WDTPW | WDTHOLD;     // Stop watchdog timer

  // http://www.glitovsky.com/Tutorialv0_4.pdf

  P3SEL |= 0x30; // P3.4,5 = USART0 TXD/RXD
  ME1 |= UTXE0 + URXE0; // Enable USART0 TXD/RXD
  UCTL0 |= CHAR; // 8-bit character
  UTCTL0 |= SSEL0; // UCLK = ACLK = 32.768kHz
  UBR00 = 0x03; // 32.768kHz/9600 - 3.41
  UBR10 = 0x00;
  UMCTL0 = 0x4a; // Modulation
  UCTL0 &= ~SWRST; // Initialize USART state machine

  uart_print("ENC28J60 Test !\n");

  // Init Port for MMC (default high)
  MMC_PxOUT |= MMC_SIMO + MMC_UCLK;
  MMC_PxDIR |= MMC_SIMO + MMC_UCLK;

  // Chip Select
  MMC_CS_PxOUT |= MMC_CS;
  MMC_CS_PxDIR |= MMC_CS;

  // Init SPI Module
  halSPISetup();

  enc28j60_init(mac_addr);

  uint16_t len;
  eth_frame_t *frame = (eth_frame_t*)net_buf;

  for (;;) {
    while ( (len = enc28j60_recv_packet(net_buf, sizeof(net_buf))) ) {
      uart_printf("%d | %x.%x.%x.%x.%x.%x | %x.%x.%x.%x.%x.%x\n", len, 
        frame->to_addr_mac[0], frame->to_addr_mac[1], frame->to_addr_mac[2],
        frame->to_addr_mac[3], frame->to_addr_mac[4], frame->to_addr_mac[5],
        frame->from_addr_mac[0], frame->from_addr_mac[1], frame->from_addr_mac[2],
        frame->from_addr_mac[3], frame->from_addr_mac[4], frame->from_addr_mac[5]);
      eth_filter(frame, len);
    }
  }

  return 0;
}
