// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Andreas Färber
 */

#include <linux/bitops.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>

static int lan9252_read(struct spi_device *spi, u16 addr, u8 *val, int len)
{
	u8 buf[3];
	struct spi_transfer xfer[2] = {
		{
			.tx_buf = buf,
			.rx_buf = NULL,
			.len = 3,
		}, {
			.tx_buf = NULL,
			.rx_buf = val,
			.len = len,
		}
	};

	buf[0] = 0x03;
#define LAN9252_READ_ADDR_INC	BIT(0)
#define LAN9252_READ_ADDR_DEC	BIT(1)
	buf[1] = (LAN9252_READ_ADDR_INC << 6) | ((addr >> 8) & GENMASK(5, 0));
	buf[2] = addr & 0xff;
	return spi_sync_transfer(spi, xfer, 2);
}

#define LAN9252_ID_REV		0x0050
#define LAN9252_BYTE_TEST	0x0064

static int lan9252_probe(struct spi_device *spi)
{
	u8 buf[4];
	int ret;

	dev_info(&spi->dev, "lan9252 probe\n");

	spi->bits_per_word = 8;
	spi_setup(spi);

	ret = lan9252_read(spi, LAN9252_BYTE_TEST, buf, 4);
	if (ret < 0) {
		dev_err(&spi->dev, "read failed (%d)\n", ret);
		return ret;
	}

	dev_dbg(&spi->dev, "read: %02x %02x %02x %02x\n",
		(unsigned)buf[0], (unsigned)buf[1], (unsigned)buf[2], (unsigned)buf[3]);

	dev_info(&spi->dev, "lan9252 probed\n");

	return 0;
}

static int lan9252_remove(struct spi_device *spi)
{
	dev_info(&spi->dev, "lan9252 removed\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id lan9252_dt_ids[] = {
	{ .compatible = "microchip,lan9252" },
	{}
};
MODULE_DEVICE_TABLE(of, lan9252_dt_ids);
#endif

static struct spi_driver lan9252_spi_driver = {
	.driver = {
		.name = "lan9252",
		.of_match_table = lan9252_dt_ids,
	},
	.probe = lan9252_probe,
	.remove = lan9252_remove,
};

module_spi_driver(lan9252_spi_driver);

MODULE_DESCRIPTION("LAN9252 SPI driver");
MODULE_AUTHOR("Andreas Färber <afaerber@suse.de>");
MODULE_LICENSE("GPL");
