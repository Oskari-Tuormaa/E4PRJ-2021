#include <linux/fs.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>


/******* MACRO SETTINGS *****************/
#define DEV_MINOR 0
#define NBR_MINORS 1

#define GPIO_PIN 17




/******* DRIVER DATA STRUCTURE **********/
struct dev_data {
	int devno;
	struct class* class;
	struct cdev cdev;
};
struct dev_data dev_data;




/******* FILE OPERATIONS ****************/

int rumble_motor_stop_rumble_thread(void *pv)
{
	unsigned int* sleep_time = (unsigned int*)pv;
	msleep(*sleep_time);
	gpio_set_value(GPIO_PIN, 0);
	kfree(sleep_time);
	return 0;
}

static int rumble_motor_write(struct file *file, const char __user *buff,
		size_t len, loff_t* offset)
{
	int err;
	unsigned int* sleep_time = kmalloc(sizeof(unsigned int), GFP_KERNEL);
	char msg[64];
	static struct task_struct *stop_thread;

	/* Get new value from userspace */
	err = copy_from_user(msg, buff, len);
	if (err < 0)
	{
		printk(KERN_ERR "RUMBLE: Error copying from user.\n");
		goto from_user_err;
	}

	/* Ensure proper termination of string */
	msg[len] = '\0';

	/* Parse input to unsigned int */
	err = kstrtouint(msg, 10, sleep_time);
	if (err < 0)
	{
		printk(KERN_ERR "RUMBLE: Error parsing input.\n");
		goto from_user_err;
	}

	if (*sleep_time > 0)
	{
		/* Set GPIO high */
		gpio_set_value(GPIO_PIN, 1);

		/* Create stop thread */
		stop_thread = kthread_run(rumble_motor_stop_rumble_thread, sleep_time, "RUMBLE stop thread");
		if (!stop_thread)
			printk(KERN_ALERT "RUMBLE: Couldn't create stop rumble thread.");
	}
	else
	{
		kfree(sleep_time);
	}

	return len;

from_user_err:
	kfree(sleep_time);
	return err;
}

static struct file_operations rumble_motor_fops = {
	.owner = THIS_MODULE,
	.write = rumble_motor_write
};




/******* INIT / EXIT ********************/

/* This method ensures that the device has read/write access */
static char * rumble_devnode(struct device * dev, umode_t * mode) {
	if(!mode) {
		return NULL;
	}
	if(dev->devt == MKDEV(MAJOR(dev_data.devno), 0)) {
		*mode = 0666;
	}
	return NULL;
}

static int rumble_motor_init(void)
{
	int err;

	/* Make device numbers */
	err = alloc_chrdev_region(&dev_data.devno, 0, NBR_MINORS, "Rumble motor");
	if (err < 0)
		goto err_alloc;

	/* Create class */
	dev_data.class = class_create(THIS_MODULE, "rumble_motor_cls");
	if (IS_ERR(dev_data.class))
		goto err_cls;

	/* Create character device */
	cdev_init(&dev_data.cdev, &rumble_motor_fops);
	err = cdev_add(&dev_data.cdev, dev_data.devno, NBR_MINORS);
	if (err < 0)
		goto err_cdev;

	/* Change permissions */
	dev_data.class->devnode = rumble_devnode;

	/* Create device node */
	struct device* dev = device_create(dev_data.class, NULL, MKDEV(MAJOR(dev_data.devno), DEV_MINOR), NULL, "RUMBLE");
	if (IS_ERR(dev))
		goto err_dev;

	err = gpio_request(GPIO_PIN, "RUMBLE_GPIO");
	if (err < 0)
		goto err_gpio;

	gpio_direction_output(GPIO_PIN, 0);

	printk("RUMBLE: Driver succesfully initialized.\n");

	return 0;

	/*** Error handling ***/
err_gpio:
err_dev:
	/* Delete character device */
	cdev_del(&dev_data.cdev);
err_cdev:
	/* Delete class */
	class_destroy(dev_data.class);
err_cls:
	/* Remove device number */
	unregister_chrdev_region(dev_data.devno, NBR_MINORS);
err_alloc:
	return err;
}



static void rumble_motor_exit(void)
{
	gpio_free(GPIO_PIN); /* Free GPIO pins */
	device_destroy(dev_data.class, MKDEV(MAJOR(dev_data.devno), DEV_MINOR)); /* Delete device */
	cdev_del(&dev_data.cdev); /* Delete character device */
	class_destroy(dev_data.class); /* Delete class */
	unregister_chrdev_region(dev_data.devno, NBR_MINORS); /* Remove device number */

	printk("RUMBLE: Driver succesfully exited.\n");
}





module_init(rumble_motor_init);
module_exit(rumble_motor_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Oskari Tuormaa");
MODULE_DESCRIPTION("Simple GPIO driver for rumble motor");
