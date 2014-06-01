/*
 * Copyright (C) 2010 Stollmann E+V GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/st21nfca.h>
#include <mach/board.h>

#define MAX_BUFFER_SIZE	256

#define ST21_FS_DEUBG

#if defined(ST21_FS_DEUBG)
#define st21_debug(fmt, ...) \
	printk("ST21Debug ////////// " pr_fmt(fmt), ##__VA_ARGS__)
#endif


#define ST21_IRQ_ACTIVE_HIGH 1
#define ST21_IRQ_ACTIVE_LOW 0







       
static int st21_debug_log = 0; 
 
struct st21nfca_dev	{
	wait_queue_head_t	read_wq;
	struct mutex		read_mutex;
	struct i2c_client	*client;
	struct miscdevice	st21nfca_device;
	unsigned int		irq_gpio;
  unsigned int    ena_gpio;
  unsigned int    polarity_mode;
  unsigned int    active_polarity; 
	bool			irq_enabled;
	spinlock_t		irq_enabled_lock;
};

static void st21nfca_disable_irq(struct st21nfca_dev *st21nfca_dev)
{
	unsigned long flags;


	spin_lock_irqsave(&st21nfca_dev->irq_enabled_lock, flags);
	if (st21nfca_dev->irq_enabled) {
		disable_irq_nosync(st21nfca_dev->client->irq);
		st21nfca_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&st21nfca_dev->irq_enabled_lock, flags);
}

static irqreturn_t st21nfca_dev_irq_handler(int irq, void *dev_id)
{
	struct st21nfca_dev *st21nfca_dev = dev_id;


	st21nfca_disable_irq(st21nfca_dev);
	

	
	wake_up(&st21nfca_dev->read_wq);

	return IRQ_HANDLED;
}

static ssize_t st21nfca_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	struct st21nfca_dev *st21nfca_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE];
	int ret;

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if(st21_debug_log) st21_debug("%s : reading %zu bytes.\n", __func__, count);

	mutex_lock(&st21nfca_dev->read_mutex);

	
	ret = i2c_master_recv(st21nfca_dev->client, tmp, count);
	mutex_unlock(&st21nfca_dev->read_mutex);

	if (ret < 0) {
		if(st21_debug_log) pr_err("%s: i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > count) {
		pr_err("%s: received too many bytes from i2c (%d)\n",
			__func__, ret);
		return -EIO;
	}
	if (copy_to_user(buf, tmp, ret)) {
		pr_warning("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}
	return ret;
}

static ssize_t st21nfca_dev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *offset)
{
	struct st21nfca_dev  *st21nfca_dev;
	char tmp[MAX_BUFFER_SIZE];
	int ret=count;

	st21nfca_dev = filp->private_data;
	if(st21_debug_log) st21_debug("%s: st21nfca_dev ptr %p\n", __func__,st21nfca_dev);

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (copy_from_user(tmp, buf, count)) {
		pr_err("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	if(st21_debug_log) st21_debug("%s : writing %zu bytes.\n", __func__, count);
	
	ret = i2c_master_send(st21nfca_dev->client, tmp, count);
	if (ret != count) {
		if(st21_debug_log) pr_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}
	return ret;
}

static int st21nfca_dev_open(struct inode *inode, struct file *filp)
{
	struct st21nfca_dev *st21nfca_dev = container_of(filp->private_data,
						struct st21nfca_dev,
						st21nfca_device);

	filp->private_data = st21nfca_dev;

	if(st21_debug_log) st21_debug("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

	if(st21_debug_log) st21_debug("%s: st21nfca_dev ptr %p\n", __func__,st21nfca_dev);

	return 0;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int st21nfca_dev_ioctl(struct inode *inode, struct file *filp,
                              unsigned int cmd, unsigned long arg)
#else
static long st21nfca_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	struct st21nfca_dev *st21nfca_dev = filp->private_data;
	int ret = 0;

	switch (cmd) {
	case ST21NFCA_GET_WAKEUP:
	    
		ret = gpio_get_value(st21nfca_dev->irq_gpio);
    if(ret >= 0  )
    {
       ret = (ret==st21nfca_dev->active_polarity);
    }
		if(st21_debug_log) st21_debug("%s get gpio result %d\n", __func__,ret);
		break;
	default:
		pr_err("%s bad ioctl %u\n", __func__, cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}
static unsigned int st21nfca_poll(struct file *file, poll_table *wait)
{
	struct st21nfca_dev  *st21nfca_dev = file->private_data;
	unsigned int mask = 0;

	
	if(st21_debug_log) st21_debug("%s call poll_Wait\n", __func__);
	poll_wait(file, &st21nfca_dev->read_wq, wait);

    if( st21nfca_dev->active_polarity == gpio_get_value(st21nfca_dev->irq_gpio)){

		if(st21_debug_log) st21_debug("%s return ready\n", __func__);
		mask = POLLIN | POLLRDNORM; 
		st21nfca_disable_irq(st21nfca_dev);
	}
	else{
		
		if(!st21nfca_dev->irq_enabled){
			if(st21_debug_log) st21_debug("%s enable irq\n", __func__);
			st21nfca_dev->irq_enabled = true;
			enable_irq(st21nfca_dev->client->irq);
		}
		else{
			if(st21_debug_log) st21_debug("%s irq already enabled\n", __func__);
		}

	}
	return mask;
}


static const struct file_operations st21nfca_dev_fops = {
	.owner	= THIS_MODULE,
	.llseek	= no_llseek,
	.read	= st21nfca_dev_read,
	.write	= st21nfca_dev_write,
	.open	= st21nfca_dev_open,
	.poll   = st21nfca_poll,

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
	.ioctl  = st21nfca_dev_ioctl,
#else
  .unlocked_ioctl = st21nfca_dev_ioctl,
#endif
};

#if defined(ST21_FS_DEUBG)
int St21GetRegisterLog(void)
{
	return (st21_debug_log);
}

EXPORT_SYMBOL(St21GetRegisterLog);

static ssize_t
nfc_show_log_read(struct device *dev,
						struct device_attribute *attr,
						char *buf)
{
	return sprintf(buf, "%d\n", st21_debug_log);
}

static ssize_t
nfc_show_log_write(struct device *dev,
						struct device_attribute *attr,
						const char *buf,
						size_t count)
{
	int value;
	
	value = simple_strtoul(buf, NULL, 10);
	printk(">>>>>>>> nfc_show_all_store value=%d buf=%s", value, buf);
	if (value != 0 && value != 1) {
		printk(">>>>>>>> nfc_show_all_store buff is not 0 or 1 ! is %d\n", value);
		return count;
	}
	if (value) {
		st21_debug_log = 1;
	}
	else {
		st21_debug_log = 0;
	}

	return count;
}

static DEVICE_ATTR(show_log, S_IRUGO|S_IWUSR|S_IWGRP,
        nfc_show_log_read, nfc_show_log_write);

static struct kobject *android_nfc_kobj;

static int st21_sysfs_init(void)
{
	int ret ;

	android_nfc_kobj = kobject_create_and_add("android_nfc", NULL) ;
	if (android_nfc_kobj == NULL) {
		printk(">>>>>>>> st21_sysfs_init: subsystem_register failed\n");
		ret = -ENOMEM;
		goto err;
	}

	ret = sysfs_create_file(android_nfc_kobj, &dev_attr_show_log.attr);
	if (ret) {
		printk(">>>>>>>> st21_sysfs_init: sysfs_create_group vendor failed\n");
		goto err4;
	}

	return 0 ;
err4:
		kobject_del(android_nfc_kobj);
err:
		return ret ;
}
#endif

static int st21nfca_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret;
	struct st21nfca_i2c_platform_data *platform_data;
	struct st21nfca_dev *st21nfca_dev;
  unsigned int  irq_type;

	platform_data = client->dev.platform_data;
    st21_debug("%s : clientptr %p\n", __func__, client);

	if (platform_data == NULL) {
		pr_err("%s : st21nfca probe fail\n", __func__);
		return  -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s : need I2C_FUNC_I2C\n", __func__);
		return  -ENODEV;
	}

	client->adapter->timeout = msecs_to_jiffies(3 * 10); 
	client->adapter->retries = 0;



	st21nfca_dev = kzalloc(sizeof(*st21nfca_dev), GFP_KERNEL);
	if (st21nfca_dev == NULL) {
		dev_err(&client->dev,
				"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}
    st21_debug("%s : dev_cb_addr %p\n", __func__, st21nfca_dev);

	st21nfca_dev->irq_gpio      = platform_data->irq_gpio;
	st21nfca_dev->ena_gpio      = platform_data->ena_gpio;
  st21nfca_dev->polarity_mode = platform_data->polarity_mode;
	st21nfca_dev->client        = client;

  
  switch(platform_data->polarity_mode)
  {
   case IRQF_TRIGGER_RISING:
     irq_type  = IRQ_TYPE_EDGE_RISING;
     st21nfca_dev->active_polarity = 1;
     break;
   case IRQF_TRIGGER_FALLING:
     irq_type  = IRQ_TYPE_EDGE_FALLING;
     st21nfca_dev->active_polarity = 0;
     break;
   case IRQF_TRIGGER_HIGH:
     irq_type  = IRQ_TYPE_LEVEL_HIGH;
     st21nfca_dev->active_polarity = 1;
     break;
   case IRQF_TRIGGER_LOW:
     irq_type  = IRQ_TYPE_LEVEL_LOW;
     st21nfca_dev->active_polarity = 0;
     break;
   default:
     irq_type  = IRQF_TRIGGER_FALLING;
     st21nfca_dev->active_polarity = 0;
     break;
  }


	ret = gpio_request(platform_data->irq_gpio, "st21nfca");
	if (ret){
		pr_err("%s : gpio_request failed\n", __FILE__);
		return  -ENODEV;
	}

	ret = gpio_direction_input(platform_data->irq_gpio);
	if (ret){
		pr_err("%s : gpio_direction_input failed\n", __FILE__);
		return  -ENODEV;
	}
	
  ret = gpio_request(platform_data->ena_gpio, "st21nfca_ena");
	if (ret){
		pr_err("%s : ena gpio_request failed\n", __FILE__);
		return  -ENODEV;
	}
	ret = gpio_direction_output(platform_data->ena_gpio,1);
	if (ret){
		pr_err("%s : ena gpio_direction_output failed\n", __FILE__);
		return  -ENODEV;
	}

	client->irq = gpio_to_irq(platform_data->irq_gpio);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36))
	ret = irq_set_irq_type(client->irq , irq_type);
#else
	ret = set_irq_type(client->irq , irq_type);
#endif  
	if (ret){
		pr_err("%s : set_irq_type failed\n", __FILE__);
		return  -ENODEV;
	}

	enable_irq_wake(client->irq);
	
	init_waitqueue_head(&st21nfca_dev->read_wq);
	mutex_init(&st21nfca_dev->read_mutex);
	spin_lock_init(&st21nfca_dev->irq_enabled_lock);

	st21nfca_dev->st21nfca_device.minor = MISC_DYNAMIC_MINOR;
	st21nfca_dev->st21nfca_device.name = "st21nfca";
	st21nfca_dev->st21nfca_device.fops = &st21nfca_dev_fops;

	ret = misc_register(&st21nfca_dev->st21nfca_device);
	if (ret) {
		pr_err("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	


	st21_debug("%s : requesting IRQ %d\n", __func__, client->irq);
	st21nfca_dev->irq_enabled = true;
	
  ret = request_irq(client->irq, st21nfca_dev_irq_handler,
			  st21nfca_dev->polarity_mode, client->name, st21nfca_dev);
        
	if (ret) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}
	st21nfca_disable_irq(st21nfca_dev);
	i2c_set_clientdata(client, st21nfca_dev);
#if defined(ST21_FS_DEUBG)
	st21_sysfs_init();
#endif
	return 0;

err_request_irq_failed:
	misc_deregister(&st21nfca_dev->st21nfca_device);
err_misc_register:
	mutex_destroy(&st21nfca_dev->read_mutex);
	kfree(st21nfca_dev);
err_exit:
	gpio_free(platform_data->irq_gpio);
	gpio_free(platform_data->ena_gpio);
	return ret;
}

static int st21nfca_remove(struct i2c_client *client)
{
	struct st21nfca_dev *st21nfca_dev;

	st21nfca_dev = i2c_get_clientdata(client);
	free_irq(client->irq, st21nfca_dev);
	misc_deregister(&st21nfca_dev->st21nfca_device);
	mutex_destroy(&st21nfca_dev->read_mutex);
	gpio_free(st21nfca_dev->irq_gpio);
	gpio_free(st21nfca_dev->ena_gpio);
	kfree(st21nfca_dev);

	return 0;
}

static const struct i2c_device_id st21nfca_id[] = {
	{ "st21nfca", 0 },
	{ }
};

static int st21nfca_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct st21nfca_dev *st21nfca_dev;
  if(client)
  {
	 st21nfca_dev = i2c_get_clientdata(client);
   if(st21nfca_dev)
   {
    gpio_set_value( st21nfca_dev->ena_gpio,1); 
    return 0;
   }
	st21_debug("%s : failing no st21 context %p !!!\n", __func__, st21nfca_dev);
  }
	st21_debug("%s : failing no client context  %p!!!\n", __func__,client);
  return 0; 
  
}

static int st21nfca_resume(struct i2c_client *client)
{
	struct st21nfca_dev *st21nfca_dev;
  if(client)
  {
	 st21nfca_dev = i2c_get_clientdata(client);
   if(st21nfca_dev)
   {
    gpio_set_value( st21nfca_dev->ena_gpio,1); 
    return 0;
   }
   st21_debug("%s : failing no st21 context %p !!!\n", __func__, st21nfca_dev);
  }
	st21_debug("%s : failing no context %p!!!\n", __func__,client);
  return 0; 
}

static struct i2c_driver st21nfca_driver = {
           .id_table = st21nfca_id,
           .probe             = st21nfca_probe,
           .remove            = st21nfca_remove,
           .suspend          = st21nfca_suspend,
           .resume            = st21nfca_resume,
           .driver              = {
                     .owner  = THIS_MODULE,
                     .name   = "st21nfca",
           },
};






static int __init st21nfca_dev_init(void)
{
	st21_debug("Loading st21nfca driver\n");
	return i2c_add_driver(&st21nfca_driver);
}
module_init(st21nfca_dev_init);

static void __exit st21nfca_dev_exit(void)
{
	st21_debug("Unloading st21nfca driver\n");
	i2c_del_driver(&st21nfca_driver);
}
module_exit(st21nfca_dev_exit);

MODULE_AUTHOR("Norbert Kawulski");
MODULE_DESCRIPTION("NFC ST21NFCA driver");
MODULE_LICENSE("GPL");
