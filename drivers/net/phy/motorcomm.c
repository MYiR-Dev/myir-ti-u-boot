/*
 * drivers/net/phy/motorcomm.c
 *
 * Driver for Motorcomm PHYs
 *
 * Author: Leilei Zhao <leilei.zhao@motorcomm.com>
 *
 * Copyright (c) 2019 Motorcomm, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Support : Motorcomm Phys:
 *		Giga phys: yt8511, yt8521
 *		100/10 Phys : yt8512, yt8512b, yt8510
 *		Automotive 100Mb Phys : yt8010
 *		Automotive 100/10 hyper range Phys: yt8510
 */

#include <phy.h>
#include <common.h>
#include <miiphy.h>

#define PHY_ID_YT8511		0x0000010a
#define MOTORCOMM_PHY_ID_MASK	0x00000fff
#define REG_DEBUG_ADDR_OFFSET		0x1e
#define REG_DEBUG_DATA			0x1f

#if (YTPHY_ENABLE_WOL)
#undef SYS_WAKEUP_BASED_ON_ETH_PKT
#define SYS_WAKEUP_BASED_ON_ETH_PKT 	1
#endif

static int ytphy_read_ext(struct phy_device *phydev, u32 regnum);
static int ytphy_write_ext(struct phy_device *phydev, u32 regnum, u16 val);
static int yt8511_config(struct phy_device *phydev);

static int ytphy_read_ext(struct phy_device *phydev, u32 regnum)
{
	int ret;
	int val;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, MDIO_DEVAD_NONE, REG_DEBUG_DATA);

	return val;
}

static int ytphy_write_ext(struct phy_device *phydev, u32 regnum, u16 val)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_DATA, val);

	return ret;
}

static int yt8511_config(struct phy_device *phydev)
{
	int ret;
	int val;

    /* disable auto sleep */
    val = ytphy_read_ext(phydev, 0x27);
    val &= (~BIT(15));

    ret = ytphy_write_ext(phydev, 0x27, val);

    /* enable RXC clock when no wire plug */
    val = ytphy_read_ext(phydev, 0xc);

    /* ext reg 0xc b[7:4]
	Tx Delay time = 150ps * N - 250ps
    */
    val |= (0xf << 4);
	val |= 0x1;
    ret = ytphy_write_ext(phydev, 0xc, val);
	
    genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return ret;
}

static struct phy_driver YT8511_driver =  {
	.name = "YT8511 Ethernet",
	.uid = PHY_ID_YT8511,
	.mask = MOTORCOMM_PHY_ID_MASK,
	.features = PHY_BASIC_FEATURES,
	.config = yt8511_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};

int phy_motorcomm_init(void)
{
	phy_register(&YT8511_driver);
	
	return 0;
}