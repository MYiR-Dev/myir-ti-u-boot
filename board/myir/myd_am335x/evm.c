/*
 * evm.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

//#define DEBUG

#include <common.h>
#include <asm/cache.h>
#include <asm/omap_common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <asm/arch/clock.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <net.h>
#include <miiphy.h>
#include <netdev.h>
#include <spi_flash.h>
#include "common_def.h"
//#include "pmic.h"
#include "tps65217.h"
#include <i2c.h>
#include <serial.h>
#include "myir_header.h" /* Add for MY-TFT070-K LCD */

DECLARE_GLOBAL_DATA_PTR;

/* UART Defines */
#define UART_SYSCFG_OFFSET	(0x54)
#define UART_SYSSTS_OFFSET	(0x58)

#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)

/* Timer Defines */
#define TSICR_REG		0x54
#define TIOCP_CFG_REG		0x10
#define TCLR_REG		0x38

/* CPLD registers */
#define CFG_REG			0x10

/*
 * I2C Address of various board
 */
#define I2C_BASE_BOARD_ADDR	0x50
#define I2C_DAUGHTER_BOARD_ADDR 0x51
#define I2C_LCD_BOARD_ADDR	0x52

#define I2C_CPLD_ADDR		0x35

/* RGMII mode define */
#define RGMII_MODE_ENABLE	0xA
#define RMII_MODE_ENABLE	0x5
#define MII_MODE_ENABLE		0x0

#define NO_OF_MAC_ADDR          3
#define ETH_ALEN		6

struct am335x_baseboard_id {
	unsigned int  magic;
	char name[8];
	char version[4];
	char serial[12];
	char config[32];
	char mac_addr[NO_OF_MAC_ADDR][ETH_ALEN];
};

static struct am335x_baseboard_id header;
extern void cpsw_eth_set_mac_addr(const u_int8_t *addr);
static unsigned char daughter_board_connected;
static volatile int board_id = BASE_BOARD;

/*
 * dram_init:
 * At this point we have initialized the i2c bus and can read the
 * EEPROM which will tell us what board and revision we are on.
 */
int dram_init(void)
{
	gd->ram_size = PHYS_DRAM_1_SIZE;

	return 0;
}

void dram_init_banksize (void)
{
	/* Fill up board info */
	gd->bd->bi_dram[0].start = PHYS_DRAM_1;
	gd->bd->bi_dram[0].size = PHYS_DRAM_1_SIZE;
}

#ifdef CONFIG_SPL_BUILD
static void Data_Macro_Config(int dataMacroNum)
{
	u32 BaseAddrOffset = 0x00;;

	if (dataMacroNum == 1)
		BaseAddrOffset = 0xA4;

	__raw_writel(((DDR3_RD_DQS<<30)|(DDR3_RD_DQS<<20)
			|(DDR3_RD_DQS<<10)|(DDR3_RD_DQS<<0)),
			(DATA0_RD_DQS_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR3_RD_DQS>>2,
			(DATA0_RD_DQS_SLAVE_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR3_WR_DQS<<30)|(DDR3_WR_DQS<<20)
			|(DDR3_WR_DQS<<10)|(DDR3_WR_DQS<<0)),
			(DATA0_WR_DQS_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR3_WR_DQS>>2,
			(DATA0_WR_DQS_SLAVE_RATIO_1 + BaseAddrOffset));
	
//	__raw_writel(1, (DATA0_WRLVL_INIT_MODE_0 + BaseAddrOffset));
//	__raw_writel(1, (DATA0_GATELVL_INIT_MODE_0 + BaseAddrOffset));
	
	__raw_writel(((DDR3_PHY_WRLVL<<30)|(DDR3_PHY_WRLVL<<20)
			|(DDR3_PHY_WRLVL<<10)|(DDR3_PHY_WRLVL<<0)),
			(DATA0_WRLVL_INIT_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR3_PHY_WRLVL>>2,
			(DATA0_WRLVL_INIT_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR3_PHY_GATELVL<<30)|(DDR3_PHY_GATELVL<<20)
			|(DDR3_PHY_GATELVL<<10)|(DDR3_PHY_GATELVL<<0)),
			(DATA0_GATELVL_INIT_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR3_PHY_GATELVL>>2,
			(DATA0_GATELVL_INIT_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR3_PHY_FIFO_WE<<30)|(DDR3_PHY_FIFO_WE<<20)
			|(DDR3_PHY_FIFO_WE<<10)|(DDR3_PHY_FIFO_WE<<0)),
			(DATA0_FIFO_WE_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR3_PHY_FIFO_WE>>2,
			(DATA0_FIFO_WE_SLAVE_RATIO_1 + BaseAddrOffset));
	__raw_writel(((DDR3_PHY_WR_DATA<<30)|(DDR3_PHY_WR_DATA<<20)
			|(DDR3_PHY_WR_DATA<<10)|(DDR3_PHY_WR_DATA<<0)),
			(DATA0_WR_DATA_SLAVE_RATIO_0 + BaseAddrOffset));
	__raw_writel(DDR3_PHY_WR_DATA>>2,
			(DATA0_WR_DATA_SLAVE_RATIO_1 + BaseAddrOffset));
	__raw_writel(PHY_DLL_LOCK_DIFF,
			(DATA0_DLL_LOCK_DIFF_0 + BaseAddrOffset));
}

static void Cmd_Macro_Config(void)
{
	__raw_writel(DDR3_RATIO, CMD0_CTRL_SLAVE_RATIO_0);
	__raw_writel(CMD_FORCE, CMD0_CTRL_SLAVE_FORCE_0);
	__raw_writel(CMD_DELAY, CMD0_CTRL_SLAVE_DELAY_0);
	__raw_writel(DDR3_DLL_LOCK_DIFF, CMD0_DLL_LOCK_DIFF_0);
	__raw_writel(DDR3_INVERT_CLKOUT, CMD0_INVERT_CLKOUT_0);

	__raw_writel(DDR3_RATIO, CMD1_CTRL_SLAVE_RATIO_0);
	__raw_writel(CMD_FORCE, CMD1_CTRL_SLAVE_FORCE_0);
	__raw_writel(CMD_DELAY, CMD1_CTRL_SLAVE_DELAY_0);
	__raw_writel(DDR3_DLL_LOCK_DIFF, CMD1_DLL_LOCK_DIFF_0);
	__raw_writel(DDR3_INVERT_CLKOUT, CMD1_INVERT_CLKOUT_0);

	__raw_writel(DDR3_RATIO, CMD2_CTRL_SLAVE_RATIO_0);
	__raw_writel(CMD_FORCE, CMD2_CTRL_SLAVE_FORCE_0);
	__raw_writel(CMD_DELAY, CMD2_CTRL_SLAVE_DELAY_0);
	__raw_writel(DDR3_DLL_LOCK_DIFF, CMD2_DLL_LOCK_DIFF_0);
	__raw_writel(DDR3_INVERT_CLKOUT, CMD2_INVERT_CLKOUT_0);
}

static void config_vtp(void)
{
	__raw_writel(__raw_readl(VTP0_CTRL_REG) | VTP_CTRL_ENABLE,
			VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP0_CTRL_REG) & (~VTP_CTRL_START_EN),
			VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP0_CTRL_REG) | VTP_CTRL_START_EN,
			VTP0_CTRL_REG);

	/* Poll for READY */
	while ((__raw_readl(VTP0_CTRL_REG) & VTP_CTRL_READY) != VTP_CTRL_READY);
}

static void config_emif_ddr3(void)
{
	u32 i;

	/*Program EMIF0 CFG Registers*/
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1);
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);
	__raw_writel(EMIF_READ_LATENCY, EMIF4_0_DDR_PHY_CTRL_2);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1);
	__raw_writel(EMIF_TIM1, EMIF4_0_SDRAM_TIM_1_SHADOW);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2);
	__raw_writel(EMIF_TIM2, EMIF4_0_SDRAM_TIM_2_SHADOW);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3);
	__raw_writel(EMIF_TIM3, EMIF4_0_SDRAM_TIM_3_SHADOW);

	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG);
	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG2);

	/* __raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL_SHD); */
	__raw_writel(0x00004650, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(0x00004650, EMIF4_0_SDRAM_REF_CTRL_SHADOW);

	for (i = 0; i < 5000; i++) {

	}

	/* __raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL);
	__raw_writel(EMIF_SDMGT, EMIF0_0_SDRAM_MGMT_CTRL_SHD); */
	__raw_writel(EMIF_SDREF, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(EMIF_SDREF, EMIF4_0_SDRAM_REF_CTRL_SHADOW);

	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG);
	__raw_writel(EMIF_SDCFG, EMIF4_0_SDRAM_CONFIG2);
}

static void config_am335x_ddr(void)
{
	int data_macro_0 = 0;
	int data_macro_1 = 1;

	enable_ddr_clocks();

	config_vtp();

	Cmd_Macro_Config();

	Data_Macro_Config(data_macro_0);
	Data_Macro_Config(data_macro_1);

	__raw_writel(PHY_RANK0_DELAY, DATA0_RANK0_DELAYS_0);
	__raw_writel(PHY_RANK0_DELAY, DATA1_RANK0_DELAYS_0);

	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD0_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD1_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_CMD2_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_DATA0_IOCTRL);
	__raw_writel(DDR_IOCTRL_VALUE, DDR_DATA1_IOCTRL);

	__raw_writel(__raw_readl(DDR_IO_CTRL) & 0xefffffff, DDR_IO_CTRL);
	__raw_writel(__raw_readl(DDR_CKE_CTRL) | 0x00000001, DDR_CKE_CTRL);

	config_emif_ddr3();
}

static void init_timer(void)
{
	/* Reset the Timer */
	__raw_writel(0x2, (DM_TIMER2_BASE + TSICR_REG));

	/* Wait until the reset is done */
	while (__raw_readl(DM_TIMER2_BASE + TIOCP_CFG_REG) & 1);

	/* Start the Timer */
	__raw_writel(0x1, (DM_TIMER2_BASE + TCLR_REG));
}
#endif

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_BOARD_INIT)

/* Added by MYIR for tps65217 */
int myir_pmic_init(void)
{
	/* BeagleBone PMIC Code */
	int usb_cur_lim;
	int mpu_vdd;

	/* Configure the i2c0 pin mux */
	enable_i2c0_pin_mux();

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	
	if (i2c_probe(TPS65217_CHIP_PM))
		return;

	/*
	 * Increase USB current limit to 1300mA or 1800mA and set
	 * the MPU voltage controller as needed.
	 */
	usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
	mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
		
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
				   TPS65217_POWER_PATH,
				   usb_cur_lim,
				   TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set DCDC3 (CORE) voltage to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
					TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/*
	 * Set LDO3 to 1.8V and LDO4 to 3.3V.
	 */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
				   TPS65217_DEFLS1,
				   TPS65217_LDO_VOLTAGE_OUT_1_8,
				   TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
				   TPS65217_DEFLS2,
				   TPS65217_LDO_VOLTAGE_OUT_3_3,
				   TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");
		
	mpu_pll_config(MPUPLL_M_800);/* Added by MYIR, our chip is 800MHz */
}

void spl_board_init(void)
{
	myir_pmic_init();
}
#endif

/* 
 * Added by MYIR, turn off lcd backlight by setting GPIO0_2 output LOW.
 */
extern void enable_backlight_pin_mux(void);
extern void enable_wdt_pin_mux(void);
static void myir_init_backlight()
{
#define SOC_PRCM_REGS                        (0x44E00000)
#define SOC_CM_WKUP_REGS                     (SOC_PRCM_REGS + 0x400)
#define CM_WKUP_GPIO0_CLKCTRL   (0x8)
#define CM_WKUP_GPIO0_CLKCTRL_MODULEMODE_ENABLE   (0x2u)
#define CM_WKUP_GPIO0_CLKCTRL_MODULEMODE   (0x00000003u)
#define CM_WKUP_GPIO0_CLKCTRL_OPTFCLKEN_GPIO0_GDBCLK   (0x00040000u)
#define CM_WKUP_CONTROL_CLKCTRL_IDLEST_FUNC   (0x0u)
#define CM_WKUP_CONTROL_CLKCTRL_IDLEST_SHIFT   (0x00000010u)
#define CM_WKUP_CONTROL_CLKCTRL   (0x4)
#define CM_WKUP_CONTROL_CLKCTRL_IDLEST   (0x00030000u)
#define CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK   (0x00000008u)
#define CM_WKUP_CM_L3_AON_CLKSTCTRL   (0x18)
#define CM_WKUP_L4WKUP_CLKCTRL_IDLEST_FUNC   (0x0u)
#define CM_WKUP_L4WKUP_CLKCTRL_IDLEST_SHIFT   (0x00000010u)
#define CM_WKUP_L4WKUP_CLKCTRL   (0xc)
#define CM_WKUP_L4WKUP_CLKCTRL_IDLEST   (0x00030000u)
#define CM_WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK   (0x00000004u)
#define CM_WKUP_CLKSTCTRL   (0x0)
#define CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK   (0x00000004u)
#define CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL   (0xcc)
#define CM_WKUP_GPIO0_CLKCTRL_IDLEST_FUNC   (0x0u)
#define CM_WKUP_GPIO0_CLKCTRL_IDLEST_SHIFT   (0x00000010u)
#define CM_WKUP_GPIO0_CLKCTRL   (0x8)
#define CM_WKUP_GPIO0_CLKCTRL_IDLEST   (0x00030000u)
#define CM_WKUP_CLKSTCTRL_CLKACTIVITY_GPIO0_GDBCLK   (0x00000100u)

#define GPIO0_BASE              0x44E07000
#define GPIO0_OE                (GPIO0_BASE + 0x134)
#define GPIO0_DATAOUT           (GPIO0_BASE + 0x13c)
#define GPIO0_SETDATAOUT        (GPIO0_BASE + 0x194)
#define GPIO0_CLEARDATAOUT      (GPIO0_BASE + 0x190)
#define BL_BIT                 (1 << 2)

    /* Writing to MODULEMODE field of CM_WKUP_GPIO0_CLKCTRL register. */
    __raw_writel(__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_GPIO0_CLKCTRL) |=
        CM_WKUP_GPIO0_CLKCTRL_MODULEMODE_ENABLE, SOC_CM_WKUP_REGS + CM_WKUP_GPIO0_CLKCTRL);

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_WKUP_GPIO0_CLKCTRL_MODULEMODE_ENABLE !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_GPIO0_CLKCTRL) &
           CM_WKUP_GPIO0_CLKCTRL_MODULEMODE));

    /*
    ** Writing to OPTFCLKEN_GPIO0_GDBCLK field of CM_WKUP_GPIO0_CLKCTRL
    ** register.
    */
    __raw_writel(__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_GPIO0_CLKCTRL) |=
        CM_WKUP_GPIO0_CLKCTRL_OPTFCLKEN_GPIO0_GDBCLK, SOC_CM_WKUP_REGS + CM_WKUP_GPIO0_CLKCTRL);

    /* Waiting for OPTFCLKEN_GPIO0_GDBCLK field to reflect the written value. */
    while(CM_WKUP_GPIO0_CLKCTRL_OPTFCLKEN_GPIO0_GDBCLK !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_GPIO0_CLKCTRL) &
           CM_WKUP_GPIO0_CLKCTRL_OPTFCLKEN_GPIO0_GDBCLK));

    /* Verifying if the other bits are set to required settings. */

    /*
    ** Waiting for IDLEST field in CM_WKUP_CONTROL_CLKCTRL register to attain
    ** desired value.
    */
    while((CM_WKUP_CONTROL_CLKCTRL_IDLEST_FUNC <<
           CM_WKUP_CONTROL_CLKCTRL_IDLEST_SHIFT) !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_CONTROL_CLKCTRL) &
           CM_WKUP_CONTROL_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_L3_AON_GCLK field in CM_L3_AON_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_CM_L3_AON_CLKSTCTRL) &
           CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK));

    /*
    ** Waiting for IDLEST field in CM_WKUP_L4WKUP_CLKCTRL register to attain
    ** desired value.
    */
    while((CM_WKUP_L4WKUP_CLKCTRL_IDLEST_FUNC <<
           CM_WKUP_L4WKUP_CLKCTRL_IDLEST_SHIFT) !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_L4WKUP_CLKCTRL) &
           CM_WKUP_L4WKUP_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_L4_WKUP_GCLK field in CM_WKUP_CLKSTCTRL register
    ** to attain desired value.
    */
    while(CM_WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
           CM_WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK));

    /*
    ** Waiting for CLKACTIVITY_L4_WKUP_AON_GCLK field in CM_L4_WKUP_AON_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL) &
           CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK));


    /* Writing to IDLEST field in CM_WKUP_GPIO0_CLKCTRL register. */
    while((CM_WKUP_GPIO0_CLKCTRL_IDLEST_FUNC <<
           CM_WKUP_GPIO0_CLKCTRL_IDLEST_SHIFT) !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_GPIO0_CLKCTRL) &
           CM_WKUP_GPIO0_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_GPIO0_GDBCLK field in CM_WKUP_GPIO0_CLKCTRL
    ** register to attain desired value.
    */
    while(CM_WKUP_CLKSTCTRL_CLKACTIVITY_GPIO0_GDBCLK !=
          (__raw_readl(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
           CM_WKUP_CLKSTCTRL_CLKACTIVITY_GPIO0_GDBCLK));



	enable_backlight_pin_mux();
	__raw_writel(__raw_readl(GPIO0_OE) & ~BL_BIT, GPIO0_OE);
	__raw_writel(BL_BIT, GPIO0_CLEARDATAOUT);

}

/*
 * early system init of muxing and clocks.
 */
void s_init(void)
{
	/* Can be removed as A8 comes up with L2 enabled */
	l2_cache_enable();

	/* WDT1 is already running when the bootloader gets control
	 * Disable it to avoid "random" resets
	 */
	__raw_writel(0xAAAA, WDT_WSPR);
	while(__raw_readl(WDT_WWPS) != 0x0);
	__raw_writel(0x5555, WDT_WSPR);
	while(__raw_readl(WDT_WWPS) != 0x0);

	/* Added by MYIR, init LCD backlight(turn_off) */
	myir_init_backlight();
	enable_wdt_pin_mux();
	enable_e2pwp_pin_mux();

#ifdef CONFIG_SPL_BUILD
	/* Setup the PLLs and the clocks for the peripherals */
	pll_init();

	/* UART softreset */
	u32 regVal;
	u32 uart_base = DEFAULT_UART_BASE;

	enable_uart0_pin_mux();
	/* IA Motor Control Board has default console on UART3*/
	/* XXX: This is before we've probed / set board_id */
	if (board_id == IA_BOARD) {
		uart_base = UART3_BASE;
	}

	regVal = __raw_readl(uart_base + UART_SYSCFG_OFFSET);
	regVal |= UART_RESET;
	__raw_writel(regVal, (uart_base + UART_SYSCFG_OFFSET) );
	while ((__raw_readl(uart_base + UART_SYSSTS_OFFSET) &
			UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK);

	/* Disable smart idle */
	regVal = __raw_readl((uart_base + UART_SYSCFG_OFFSET));
	regVal |= UART_SMART_IDLE_EN;
	__raw_writel(regVal, (uart_base + UART_SYSCFG_OFFSET));

	/* Initialize the Timer */
	init_timer();

	preloader_console_init();

	config_am335x_ddr();
#endif
}

static unsigned char profile = PROFILE_0;

/*
 * Basic board specific setup
 */
#ifndef CONFIG_SPL_BUILD
int board_evm_init(void)
{
	/* mach type passed to kernel */
	if (board_id == IA_BOARD)
		gd->bd->bi_arch_number = MACH_TYPE_TIAM335IAEVM;
	else
		gd->bd->bi_arch_number = MACH_TYPE_TIAM335EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;

	return 0;
}
#endif

/*
 * LCD type -- Conway
 */
const unsigned char *lcd_type(void)
{
	static myir_header_t header;

	memset(&header, '\0', sizeof(myir_header_t));
	
	if (get_header(&header))
		return NULL;	
	return get_header_subtype(&header);
}
	
/*
 * LCD identify -- Conway
 */
void lcd_identify(void)
{
	char *env_optargs = getenv("optargs");
	char *tmp = "";
	if (!env_optargs)
		env_optargs = tmp;
	
	int  optargs_len = strlen(env_optargs);
	char *display = "board-am335xevm.display_mode=";
	int  display_len = strlen(display);
	char mode[14] = { '\0' };
	unsigned char *type = NULL;
	char *new_optargs = malloc(optargs_len + display_len + 16);
	if (!new_optargs) {
		printf("Alloc memory failed\n");
		return;
	}
	
	memset(new_optargs, '\0', optargs_len + display_len + 5);

	int idx = 0;
	int mode_idx = 0;
	while (idx < optargs_len) {
		if (env_optargs[idx] == ' ' || idx == 0) {
			if (env_optargs[idx] == ' ') 
				strncat(new_optargs, &env_optargs[idx++], 1);

			if ((optargs_len - idx) >= display_len) {
				if (strncmp(&env_optargs[idx], display, display_len) == 0) {
					idx += display_len;
					while (env_optargs[idx] != ' ' && env_optargs[idx] != '\0') {
						if (mode_idx + 1 > 14) {
							printf("The length of displaymode string is to long, should be smaller tha [14]\n");
							break;
						}
						mode[mode_idx++] = env_optargs[idx++];
					}
					continue;
				}
			}
		}
		strncat(new_optargs, &env_optargs[idx++], 1);
	}

	strncat(new_optargs, " ", 1);
	if (type = lcd_type()) {
		strncat(new_optargs, display, display_len);
		strncat(new_optargs, type, strlen(type));
	} else if (mode_idx > 0) {
		strncat(new_optargs, display, display_len);
		strncat(new_optargs, mode, mode_idx);
	}

	setenv("optargs", new_optargs);
	free(new_optargs);
}

#if 0
struct serial_device *default_serial_console(void)
{

	if (board_id != IA_BOARD) {
		return &eserial1_device;	/* UART0 */
	} else {
		return &eserial4_device;	/* UART3 */
	}
}
#endif

int board_init(void)
{
	u32 i2c_base_old;

	/* Configure the i2c0 pin mux */
	enable_i2c0_pin_mux();
	
	/* Modified by Conway. Added i2c1 init for eeprom of lcd */
	i2c_base_old = get_i2c_base();
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	set_i2c_base(0x4802A000);
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	set_i2c_base(i2c_base_old);
	
	printf("Did not find a recognized configuration, "
		"assuming General purpose EVM in Profile 0 with "
		"Daughter board\n");
	board_id = GP_BOARD;
	profile = 1;	/* profile 0 is internally considered as 1 */
	daughter_board_connected = 1;

	configure_evm_pin_mux(board_id, header.version, profile, daughter_board_connected);
	
#ifndef CONFIG_SPL_BUILD
	board_evm_init();
#endif
	gpmc_init();

	return 0;
}

int misc_init_r(void)
{
#ifdef DEBUG
	unsigned int cntr;
	unsigned char *valPtr;

	debug("EVM Configuration - ");
	debug("\tBoard id %x, profile %x, db %d\n", board_id, profile,
						daughter_board_connected);
	debug("Base Board EEPROM Data\n");
	valPtr = (unsigned char *)&header;
	for(cntr = 0; cntr < sizeof(header); cntr++) {
		if(cntr % 16 == 0)
			debug("\n0x%02x :", cntr);
		debug(" 0x%02x", (unsigned int)valPtr[cntr]);
	}
	debug("\n\n");

	debug("Board identification from EEPROM contents:\n");
	debug("\tBoard name   : %.8s\n", header.name);
	debug("\tBoard version: %.4s\n", header.version);
	debug("\tBoard serial : %.12s\n", header.serial);
	debug("\tBoard config : %.6s\n\n", header.config);
#endif
	/* Add for MY-TFT070-K LCD */
	lcd_identify();

#ifdef AUTO_UPDATESYS
        run_command("run updatesys", 0);
#endif
	return 0;
}

#ifdef BOARD_LATE_INIT
int board_late_init(void)
{
	if (board_id == IA_BOARD) {
		/*
		* SPI bus number is switched to in case Industrial Automation
		* motor control EVM.
		*/
		setenv("spi_bus_no", "1");
		/* Change console to tty03 for IA Motor Control EVM */
		setenv("console", "ttyO3,115200n8");
	}

	return 0;
}
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
/* TODO : Check for the board specific PHY */
static void evm_phy_init(char *name, int addr)
{
	unsigned short val;
	unsigned int cntr = 0;
	unsigned short phyid1, phyid2;
	int bone_pre_a3 = 0;

	if (board_id == BONE_BOARD && (!strncmp(header.version, "00A1", 4) ||
		    !strncmp(header.version, "00A2", 4)))
		bone_pre_a3 = 1;

	/*
	 * This is done as a workaround to support TLK110 rev1.0 PHYs.
	 * We can only perform these reads on these PHYs (currently
	 * only found on the IA EVM).
	 */
	if ((miiphy_read(name, addr, MII_PHYSID1, &phyid1) != 0) ||
			(miiphy_read(name, addr, MII_PHYSID2, &phyid2) != 0)) {
		printf("miiphy read id fail\n");
		return;
	}

	/* Enable Autonegotiation */
	if (miiphy_read(name, addr, MII_BMCR, &val) != 0) {
		printf("failed to read bmcr\n");
		return;
	}

	if (bone_pre_a3) {
		val &= ~(BMCR_FULLDPLX | BMCR_ANENABLE | BMCR_SPEED100);
		val |= BMCR_FULLDPLX;
	} else
		val |= BMCR_FULLDPLX | BMCR_ANENABLE | BMCR_SPEED100;

	if (miiphy_write(name, addr, MII_BMCR, val) != 0) {
		printf("failed to write bmcr\n");
		return;
	}

	miiphy_read(name, addr, MII_BMCR, &val);

	/*
	 * The 1.0 revisions of the GP board don't have functional
	 * gigabit ethernet so we need to disable advertising.
	 */
	if (board_id == GP_BOARD && !strncmp(header.version, "1.0", 3)) {
		miiphy_read(name, addr, MII_CTRL1000, &val);
		val &= ~PHY_1000BTCR_1000FD;
		val &= ~PHY_1000BTCR_1000HD;
		miiphy_write(name, addr, MII_CTRL1000, val);
		miiphy_read(name, addr, MII_CTRL1000, &val);
	}

	/* Setup general advertisement */
	if (miiphy_read(name, addr, MII_ADVERTISE, &val) != 0) {
		printf("failed to read anar\n");
		return;
	}

	if (bone_pre_a3)
		val |= (LPA_10HALF | LPA_10FULL);
	else
		val |= (LPA_10HALF | LPA_10FULL | LPA_100HALF | LPA_100FULL);

	if (miiphy_write(name, addr, MII_ADVERTISE, val) != 0) {
		printf("failed to write anar\n");
		return;
	}

	miiphy_read(name, addr, MII_ADVERTISE, &val);


#if 0 /* wo don't do the negotiation */
	/* Restart auto negotiation*/
	miiphy_read(name, addr, MII_BMCR, &val);
	val |= BMCR_ANRESTART;
	miiphy_write(name, addr, MII_BMCR, val);

	/*check AutoNegotiate complete - it can take upto 3 secs*/
	do {
		udelay(40000);
		cntr++;
		if (!miiphy_read(name, addr, MII_BMSR, &val)) {
			if (val & BMSR_ANEGCOMPLETE)
				break;
		}
	} while (cntr < 250);

	if (cntr >= 250)
		printf("Auto negotitation failed\n");
#endif

	return;
}

static void cpsw_control(int enabled)
{
	/* nothing for now */
	/* TODO : VTP was here before */
	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,	
		.sliver_reg_ofs	= 0xd80,
		.phy_id		= 4/*0*/,/* modified by MYIR */
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_id		= 6/*2*/,/* Modified by MYIR */
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= AM335X_CPSW_MDIO_BASE,
	.cpsw_base		= AM335X_CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.mac_control		= (1 << 5) /* MIIEN */,
	.control		= cpsw_control,
	.phy_init		= evm_phy_init,
	.gigabit_en		= 1,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int board_eth_init(bd_t *bis)
{
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	u_int32_t i;

	if (!eth_getenv_enetaddr("ethaddr", mac_addr)) {
		debug("<ethaddr> not set. Reading from E-fuse\n");
		/* try reading mac address from efuse */
		mac_lo = __raw_readl(MAC_ID0_LO);
		mac_hi = __raw_readl(MAC_ID0_HI);
		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;

		if (!is_valid_ether_addr(mac_addr)) {
			debug("Did not find a valid mac address in e-fuse. "
					"Trying the one present in EEPROM\n");

			for (i = 0; i < ETH_ALEN; i++)
				mac_addr[i] = header.mac_addr[0][i];
		}

		if (is_valid_ether_addr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
		else {
			printf("Caution: Using hardcoded mac address. "
				"Set <ethaddr> variable to overcome this.\n");
		}
	}
	
	/* set mii mode to rgmii in in device configure register */
	__raw_writel(RGMII_MODE_ENABLE, MAC_MII_SEL);
	

	cpsw_data.gigabit_en = 0;

	return cpsw_register(&cpsw_data);
}
#endif

#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0);
	return 0;
}
#endif

#ifdef CONFIG_NAND_TI81XX
/******************************************************************************
 * Command to switch between NAND HW and SW ecc
 *****************************************************************************/
extern void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode);
static int do_switch_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int type = 0;
	if (argc < 2)
		goto usage;

	if (strncmp(argv[1], "hw", 2) == 0) {
		if (argc == 3)
			type = simple_strtoul(argv[2], NULL, 10);
		ti81xx_nand_switch_ecc(NAND_ECC_HW, type);
	}
	else if (strncmp(argv[1], "sw", 2) == 0)
		ti81xx_nand_switch_ecc(NAND_ECC_SOFT, 0);
	else
		goto usage;

	return 0;

usage:
	printf("Usage: nandecc %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandecc, 3, 1,	do_switch_ecc,
	"Switch NAND ECC calculation algorithm b/w hardware and software",
	"[sw|hw <hw_type>] \n"
	"   [sw|hw]- Switch b/w hardware(hw) & software(sw) ecc algorithm\n"
	"   hw_type- 0 for Hamming code\n"
	"            1 for bch4\n"
	"            2 for bch8\n"
	"            3 for bch16\n"
);

#endif /* CONFIG_NAND_TI81XX */
#endif /* CONFIG_SPL_BUILD */
