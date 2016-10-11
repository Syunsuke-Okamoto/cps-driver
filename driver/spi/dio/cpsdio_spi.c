/*
 *  Driver for CPS-DIO digital I/O for SPI
 *
 *  Copyright (C) 2016 shinichiro nagasato <shinichiro_nagasato@contec.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#ifdef CONFIG_CONPROSYS_SDK
 #include "../include/cpsdio.h"
#else
 #include "../../../include/cpsdio.h"
#endif

#define CPSDIOX_PACKET_SIZE	4
#define CPSDIO_SPI_CHANNELS	1
#define CPSDIO_SPI_MAJOR 0
#define CPSDIO_SPI_MINOR 0
#define CPSDIO_SPI_NUM_DEVS 1
#define CPSDIO_SPI_DRIVER_NAME	"cpsdio"
#define DRV_VERSION "0.0.1"
#define max_speed 6000000

/*	Define the name of the device and sysfs class name*/
static struct cdev cpsdio_spi_cdev;
static struct class *cpsdio_spi_class = NULL;

static int cpsdio_spi_major = 0;
static int cpsdio_spi_minor = 0;

struct CPSDIO_SPI_drvdata {
	struct spi_device *spi;
	struct mutex lock;
};

static struct spi_board_info CPSDIO_SPI_info = {
		.modalias = "CPSDIO_SPI",
		.max_speed_hz = max_speed,
		.bus_num = 2,
		.chip_select = 0,
		.mode = SPI_MODE_2,
};

/*	To be able to specify the bus number and the CS in the parameter*/
static int spi_bus_num = 2;
static int spi_chip_select = 0;
module_param(spi_bus_num, int, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
module_param(spi_chip_select, int, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);

static int cpsdio_open(struct inode *inode, struct file *filp) {

	pr_info("Entered function: %s\n", __FUNCTION__);
	return 0;
}

static int cpsdio_write_output(struct spi_device *spi, const void *buf, size_t len) {
	int status;

	status = spi_write(spi, buf, len);
	if (status < 0) {
		pr_err("FAILURE: %s failed \n", __FUNCTION__);
		return -ENODEV;
	} else {
		return 0;
	}
}
EXPORT_SYMBOL_GPL(cpsdio_write_output);

static int cpsdio_read_input(struct spi_device *spi, unsigned char buf[], size_t len) {
	int status;
	unsigned char rx_cmd[]={0x00, 0x00};
	unsigned char rx_val[]={0x00, 0x00};

	rx_cmd[0] = buf[0];
	rx_cmd[1] = buf[1];

	status = spi_write_then_read(spi,rx_cmd, sizeof(rx_cmd), rx_val, sizeof(rx_val));
	if (status < 0) {
		pr_err("FAILURE: %s failed \n", __FUNCTION__);
		return -ENODEV;
	} else {
		return rx_val[1];
	}
}

static int cpsdio_close(struct inode *inode, struct file *filp) {
	pr_info("Entered function: %s\n", __FUNCTION__);
	return 0;
}

static long cpsdio_get_digital_filter( struct spi_device *spi, unsigned char buf[], size_t len )
{
	int status;
	unsigned char rx_cmd[]={0x00, 0x00};
	unsigned char rx_val[]={0x00, 0x00};

	rx_cmd[0] = buf[0];
	rx_cmd[1] = buf[1];

	status = spi_write_then_read(spi,rx_cmd, sizeof(rx_cmd), rx_val, sizeof(rx_val));
	if (status < 0) {
		pr_err("FAILURE: %s failed \n", __FUNCTION__);
		return -ENODEV;
	} else {
		return rx_val[0];
	}
}

static long cpsdio_set_digital_filter( struct spi_device *spi, const void *buf, size_t len )
{
	int status;

	status = spi_write(spi, buf, len);
	if (status < 0) {
		pr_err("FAILURE: %s failed \n", __FUNCTION__);
		return -ENODEV;
	} else {
		return 0;
	}
}

static long cpsdio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	struct device *dev;
	struct spi_device *spi;
	struct spi_master *master;
	struct CPSDIO_SPI_drvdata *data;
	char str[128];

	unsigned char tx[]={0x00, 0x00, 0x00, 0x00};
	unsigned char rx[]={0x00, 0x00, 0x00, 0x00};
	struct cpsdio_ioctl_arg ioc;

	master = spi_busnum_to_master(CPSDIO_SPI_info.bus_num);
	snprintf(str, sizeof(str), "%s.%u", dev_name(&master->dev),
			CPSDIO_SPI_info.chip_select);
	dev = bus_find_device_by_name(&spi_bus_type, NULL, str);
	spi = to_spi_device(dev);
	data = (struct CPSDIO_SPI_drvdata *)spi_get_drvdata(spi);

	memset( &ioc, 0 , sizeof(ioc) );

	switch (cmd) {
		case IOCTL_CPSDIO_INP_PORT:
			if(!access_ok(VERITY_WRITE, (void __user *)arg, _IOC_SIZE(cmd) ) ){
				return -EFAULT;
			}
			if( copy_from_user( &ioc, (int __user *)arg, sizeof(ioc) ) ){
				return -EFAULT;
			}
			mutex_lock( &data->lock );
			rx[1] = (unsigned char)ioc.port;
			ioc.port = ioc.port >> 8;
			rx[0] = (unsigned char)ioc.port;
			ioc.val = (unsigned int)cpsdio_read_input(spi,rx,sizeof(&rx));

			mutex_unlock( &data->lock );
			if( copy_to_user( (int __user *)arg, &ioc, sizeof(ioc) ) ){
				return -EFAULT;
			}
			break;
		case IOCTL_CPSDIO_OUT_PORT:
			if(!access_ok(VERITY_READ, (void __user *)arg, _IOC_SIZE(cmd) ) ){
				return -EFAULT;
			}
			if( copy_from_user( &ioc, (int __user *)arg, sizeof(ioc) ) ){
				return -EFAULT;
			}
			mutex_lock( &data->lock );
			tx[1] = (unsigned char)ioc.port;
			ioc.port = ioc.port >> 8;
			tx[0] = (unsigned char)ioc.port;
			tx[2] = tx[3] = (unsigned char)ioc.val;
			cpsdio_write_output(spi, (const void *)&tx, sizeof(&tx));

			mutex_unlock( &data->lock );
			break;
		case IOCTL_CPSDIO_SET_FILTER:
			if(!access_ok(VERITY_READ, (void __user *)arg, _IOC_SIZE(cmd) ) ) {
				return -EFAULT;
			}

			if (copy_from_user(&ioc, (int __user *) arg, sizeof(ioc))) {
				return -EFAULT;
			}
			mutex_lock(&data->lock);

			tx[0] = (unsigned char)0x01;
			tx[1] = (unsigned char)0x8C;
			tx[2] = (unsigned char)ioc.val;
			cpsdio_set_digital_filter(spi,(const void *)&tx, sizeof(&tx));
			mutex_unlock(&data->lock);
			break;
		case IOCTL_CPSDIO_GET_FILTER:
			if(!access_ok(VERITY_WRITE, (void __user *)arg, _IOC_SIZE(cmd) ) ) {
				return -EFAULT;
			}
			if (copy_from_user(&ioc, (int __user *) arg, sizeof(ioc))) {
				return -EFAULT;
			}
			mutex_lock(&data->lock);
			rx[0] = (unsigned char)0x01;
			rx[1] = (unsigned char)0x84;
			ioc.val = cpsdio_get_digital_filter(spi,rx,sizeof(&rx));
			mutex_unlock(&data->lock);
			if (copy_to_user((int __user *) arg, &ioc, sizeof(ioc))) {
				return -EFAULT;
			}
			break;
	}
	return 0;
}

static struct file_operations cpsdio_spi_fops = { .owner = THIS_MODULE,
		.open = cpsdio_open,
		.release = cpsdio_close,
		.unlocked_ioctl = cpsdio_ioctl,
};

static int CPSDIO_SPI_register_dev(void) {
	dev_t dev = MKDEV(CPSDIO_SPI_MAJOR, CPSDIO_SPI_MINOR);
	int ret = 0;
	struct device *devlp = NULL;

/*	Registration of character device number*/
	ret = alloc_chrdev_region(&dev, 0, CPSDIO_SPI_NUM_DEVS,
			CPSDIO_SPI_DRIVER_NAME);
	if (ret < 0) {
		pr_err( "alloc_chrdev_region failed.\n");
		return ret;
	}
	cpsdio_spi_major = MAJOR(dev);
	cpsdio_spi_minor = MINOR(dev);

/*	Registration of character device and operation function*/
	cdev_init(&cpsdio_spi_cdev, &cpsdio_spi_fops);
	cpsdio_spi_cdev.owner = THIS_MODULE;
	cpsdio_spi_cdev.ops = &cpsdio_spi_fops;
	ret = cdev_add(&cpsdio_spi_cdev, dev, CPSDIO_SPI_NUM_DEVS);
	if (ret) {
		pr_err(" cpsdio : driver init. devices error \n");
		unregister_chrdev_region(dev, CPSDIO_SPI_NUM_DEVS);
		return ret;
	}

/*	Creating a sysfs (/ sys) class*/
	cpsdio_spi_class = class_create(THIS_MODULE, CPSDIO_SPI_DRIVER_NAME);
	if (IS_ERR(cpsdio_spi_class)) {
		pr_err(" cpsdio : driver init. devices error \n");
		cdev_del(&cpsdio_spi_cdev);
		unregister_chrdev_region(dev, CPSDIO_SPI_NUM_DEVS);
		return PTR_ERR(cpsdio_spi_class);
	}

/*	Registration to the sysfs*/
	devlp = device_create(cpsdio_spi_class, NULL, dev, NULL,
			CPSDIO_SPI_DRIVER_NAME"%d", 0);
	if (IS_ERR(devlp)) {
		pr_err(" cpsdio : driver init. devices error \n");
		cdev_del(&cpsdio_spi_cdev);
		unregister_chrdev_region(dev, CPSDIO_SPI_NUM_DEVS);
		return PTR_ERR(devlp);
	}
	return 0;
}

static int CPSDIO_SPI_probe(struct spi_device *spi) {
	struct CPSDIO_SPI_drvdata *data;

/*	Setting the SPI*/
	spi->max_speed_hz = CPSDIO_SPI_info.max_speed_hz;
	spi->mode = CPSDIO_SPI_info.mode;
	spi->bits_per_word = 8;
	if (spi_setup(spi)) {
		pr_err( "spi_setup returned error\n");
		return -ENODEV;
	}
	data = kzalloc(sizeof(struct CPSDIO_SPI_drvdata), GFP_KERNEL);
	if (data == NULL) {
		pr_err( "%s: no memory\n", __func__ );
		return -ENODEV;
	}
	data->spi = spi;
	mutex_init(&data->lock);

/*	Save CPSDIO_SPI_drvdata structure*/
	spi_set_drvdata(spi, data);

/*	Registration of the device node*/
	CPSDIO_SPI_register_dev();

	pr_info( "cpsdio_spi driver registered\n");

	return 0;
}

static int CPSDIO_SPI_remove(struct spi_device *spi) {
	struct CPSDIO_SPI_drvdata *data;
	unsigned char tx[]={0x01, 0x2C, 0x00, 0x00};

	data = (struct CPSDIO_SPI_drvdata *) spi_get_drvdata(spi);

	/*	DIO LED initialization processing*/
	cpsdio_write_output(spi, (const void *)&tx, sizeof(&tx));

	kfree(data);
	pr_info( "cpsdio_spi driver removed\n");

	return 0;
}

static struct spi_device_id CPSDIO_SPI_id[] = {
		{ "CPSDIO_SPI", 0 },
		{ }, };
MODULE_DEVICE_TABLE(spi, CPSDIO_SPI_id);

static struct spi_driver CPSDIO_SPI_driver = {
		.driver = {
				.name =	CPSDIO_SPI_DRIVER_NAME,
				.owner = THIS_MODULE, },
				.id_table =	CPSDIO_SPI_id,
				.probe = CPSDIO_SPI_probe,
				.remove = CPSDIO_SPI_remove,
};

static void spi_remove_device(struct spi_master *master, unsigned int cs) {
	struct device *dev;
	char str[128];

	snprintf(str, sizeof(str), "%s.%u", dev_name(&master->dev), cs);
/*	Find SPI device*/
	dev = bus_find_device_by_name(&spi_bus_type, NULL, str);

/*	Delete any SPI device*/
	if (dev) {
		pr_info( "Delete %s\n", str);
		device_del(dev);
	}
}

static int CPSDIO_SPI_init(void) {
	struct spi_master *master;
	struct spi_device *spi_device;

	spi_register_driver(&CPSDIO_SPI_driver);

	CPSDIO_SPI_info.bus_num = spi_bus_num;
	CPSDIO_SPI_info.chip_select = spi_chip_select;

	master = spi_busnum_to_master(CPSDIO_SPI_info.bus_num);
	if (!master) {
		pr_err( "spi_busnum_to_master returned NULL\n");
		goto init_err;
	}
/*	Remove because spidev2.0 in the initial state is occupying*/
	spi_remove_device(master, CPSDIO_SPI_info.chip_select);
	spi_device = spi_new_device(master, &CPSDIO_SPI_info);
	if (!spi_device) {
		pr_err( "spi_new_device returned NULL\n" );
		goto init_err;
	}
	return 0;
init_err:
	spi_unregister_driver(&CPSDIO_SPI_driver);
	return -ENODEV;
}
module_init(CPSDIO_SPI_init);

static void CPSDIO_SPI_exit(void) {
	struct spi_master *master;
	dev_t dev_exit;

	master = spi_busnum_to_master(CPSDIO_SPI_info.bus_num);

/*	Delete SPI devices*/
	if (master) {
		spi_remove_device(master, CPSDIO_SPI_info.chip_select);
	} else {
		pr_err( "CPSDIO_SPI remove error\n");
	}
	spi_unregister_driver(&CPSDIO_SPI_driver);

/*	And deletion of the device, deletion of the class*/
	dev_exit = MKDEV(cpsdio_spi_major, cpsdio_spi_minor);
	device_destroy(cpsdio_spi_class, dev_exit);
	class_destroy(cpsdio_spi_class);

/*	Deleting a character device, the return of the character device number*/
	cdev_del(&cpsdio_spi_cdev);
	unregister_chrdev_region(dev_exit, CPSDIO_SPI_NUM_DEVS);
}
module_exit(CPSDIO_SPI_exit);

MODULE_AUTHOR("shinichiro nagasato");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("CONTEC CONPROSYS Digital I/O driver for SPI");
MODULE_VERSION(DRV_VERSION);
