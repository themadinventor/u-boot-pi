/*
 * Support for SDHCI device on 2835
 * Based on sdhci-bcm2708.c (c) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Supports:
 * SDHCI platform device - Arasan SD controller in BCM2708
 *
 * Inspired by sdhci-pci.c, by Pierre Ossman
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/arch/regs.h>

#undef BCM2835_TRACE_REGISTERS
#define WRITE_DELAY	40

static char *BCMSDH_NAME = "bcm2835_sdh";
static struct sdhci_ops bcm2835_ops;

inline static void bcm2835_sdhci_raw_writel(struct sdhci_host *host, u32 val, int reg)
{
	writel(val, host->ioaddr + reg);
	udelay(20000);
}

inline static u32 bcm2835_sdhci_raw_readl(struct sdhci_host *host, int reg)
{

	return readl(host->ioaddr + reg);
}

static void bcm2835_sdhci_writel(struct sdhci_host *host, u32 val, int reg)
{
#ifdef BCM2835_TRACE_REGISTERS
	printf("SDHCI[%02x] writel %08x\n", reg, val);
#endif
	writel(val, host->ioaddr + reg);
}

static void bcm2835_sdhci_writew(struct sdhci_host *host, u16 val, int reg)
{
	static u32 shadow = 0;

#ifdef BCM2835_TRACE_REGISTERS
	printf("SDHCI[%02x] writew %04x\n", reg, val);
#endif

	u32 p = reg == SDHCI_COMMAND ? shadow :
               bcm2835_sdhci_raw_readl(host, reg & ~3);
	u32 s = reg << 3 & 0x18;
	u32 l = val << s;
	u32 m = 0xffff << s;

	if (reg == SDHCI_TRANSFER_MODE)
		shadow = (p & ~m) | l;
	else {
		bcm2835_sdhci_raw_writel(host, (p & ~m) | l, reg & ~3);
	}
}

static void bcm2835_sdhci_writeb(struct sdhci_host *host, u8 val, int reg)
{
#ifdef BCM2835_TRACE_REGISTERS
	printf("SDHCI[%02x] writeb %02x\n", reg, val);
#endif

	u32 p = bcm2835_sdhci_raw_readl(host, reg & ~3);
	u32 s = reg << 3 & 0x18;
	u32 l = val << s;
	u32 m = 0xff << s;

	bcm2835_sdhci_raw_writel(host, (p & ~m) | l, reg & ~3);
}

static u32 bcm2835_sdhci_readl(struct sdhci_host *host, int reg)
{
	u32 val = readl(host->ioaddr + reg);
#ifdef BCM2835_TRACE_REGISTERS
	printf("SDHCI[%02x] readl %08x\n", reg, val);
#endif
	return val;
}

static u16 bcm2835_sdhci_readw(struct sdhci_host *host, int reg)
{
	u32 val = bcm2835_sdhci_raw_readl(host, (reg & ~3));
	val = val >> (reg << 3 & 0x18) & 0xffff;

#ifdef BCM2835_TRACE_REGISTERS
	printf("SDHCI[%02x] readw %04x\n", reg, val);
#endif

	return (u16)val;
}

static u8 bcm2835_sdhci_readb(struct sdhci_host *host, int reg)
{
	u32 val = bcm2835_sdhci_raw_readl(host, (reg & ~3));
	val = val >> (reg << 3 & 0x18) & 0xff;

#ifdef BCM2835_TRACE_REGISTERS
	printf("SDHCI[%02x] readb %02x\n", reg, val);
#endif

	return (u8)val;
}

int bcm2835_sdh_init(u32 regbase)
{
	struct sdhci_host *host = NULL;

	host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
	if (!host) {
		printf("sdh_host malloc fail!\n");
		return 1;
	}

	host->name = BCMSDH_NAME;
	host->ioaddr = (void *)regbase;
	host->quirks = SDHCI_QUIRK_BROKEN_VOLTAGE;
	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;;

	memset(&bcm2835_ops, 0, sizeof(struct sdhci_ops));
	bcm2835_ops.write_l = bcm2835_sdhci_writel;
	bcm2835_ops.write_w = bcm2835_sdhci_writew;
	bcm2835_ops.write_b = bcm2835_sdhci_writeb;
	bcm2835_ops.read_l = bcm2835_sdhci_readl;
	bcm2835_ops.read_w = bcm2835_sdhci_readw;
	bcm2835_ops.read_b = bcm2835_sdhci_readb;
	host->ops = &bcm2835_ops;

	host->version = sdhci_readw(host, SDHCI_HOST_VERSION) & 0xff;
	add_sdhci(host, 80000000, 0);

	return 0;
}

int board_mmc_init()
{

	bcm2835_sdh_init(BCM2835_EMMC_PHYSADDR);
}
