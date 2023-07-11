#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h> 
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>

#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define TIMER_CNT   1
#define TIMER_NAME "timer"
#define CLOSE_CMD (__IO(0xEF,0x1))
#define OPEN_CMD  (__IO(0xEF,0x2))
#define SETPERIOD_CMD (__IO(0xEF,0x3))
#define LED_ON  1
#define LED_OFF 0

static struct timer_dev{
    dev_t  devid;
    struct cdev cdev;
    struct class *class;
    struct device* device;
    int major, minor;
    struct device_node* nd;
    int led_gpio;
    int timeperiod;
    struct timer_list timer;
    spinlock lock; // lock
}

struct timer_dev timerdev;

static int led_init(void)
{
    int ret =0;
    timerdev.nd = of_find_node_by_path("/gpioled");
    if(timerdev.nd == NULL){
        return -EINVAL;
    }

    timerdev.led_gpio = of_get_named_gpio(timerdev.nd,"led-gpio",0);

    if(timerdev.led_gpio < 0 ){
        printk("can't find LED\n\r");
        return -EINVAL;
    }

    gpio_request(timerdev.led_gpio,"led");
    ret = gpio_direction_output(timerdev.led_gpio,1);
    if(ret <0 ){
        printk("can't set gpio \r\n");
    }
    return 0;
}

static int timer_open(struct inode *inode , struct file *filp)
{
    int ret = 0;
    filp->private_data = &timerdev;

    timerdev.period = 1000;
    ret = led_init();

    if(ret <0){
        return ret;
    }
    return 0;

}

static long timer_unlocked_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    struct timer_dev* dev = filp->private_data;
    int timerperiod;
    unsigned long flags = 0;

    switch(cmd){
        case CLOSE_CMD:
            del_timer_sync(&dev->timer);
            break;
        case OPEN_CMD:
            spin_lock_irqsave(&dev->lock,flags);
            timerperiod = dev->timeperiod;
            spin_unlock_irqrestore(&dev->lock,flags);
            mod_timer(&dev->timer,jiffier + msecs_to_jiffies(timerperiod));

            break;
        case SETPERIOD_CMD:
            spin_lock_irqsave(&dev->lock,flags);
            dev->timeperiod = arg;
            spin_unlock_irqrestore(&dev->lock,flags);
            mod_timer(&dev->timer,jiffier + msecs_to_jiffies(arg));
            break;
        default:
            break;
    }

}

static struct file_operations timer_fops={
    .owner = THIS_MODULE,
    .open = timer_open,
    .unlocked_ioctl = timer_unlocked_ioctl,
};

void timer_function(unsigned long args)
{
    struct timer_dev* dev = (struct timer_dev *)args;

    static int sta =1;
    int timerperiod = 0;
    unsigned long flags = 0;

    sta = !sta;

    gpio_set_value(dev->led_gpio,sta);

    spin_lock_irqsave(&dev->lock, flags);
    timerperiod = dev->timeperiod;
    spin_unlock_irqrestore(&dev->lock, flags);
    mod_timer(&dev->timer,jiffier + msecs_to_jiffies(timerperiod));   
}

static int __init timer_init(void)
{
    spin_lock_init(&timerdev.lock);

    if(timerdev.major){
        timerdev.devid = MKDEV(timerdev.major,0);
        register_chrdev_region(timerdev.devid,TIMER_CNT,TIMER_NAME);
    }
    else{
        alloc_chrdev_region(&timerdev.devid,0,TIMER_CNT,TIMER_NAME);
        timerdev.major = MAJOR(timerdev.devid);
        timerdev.minor = MINOR(timerdev.devid);
    }

    timerdev.cdev.owner = THIS_MODULE;
    cdev_init(&timersdev.cdev,&timer_fops);
    cdev_add(&timerdev.cdev,timerdev.devid,TIMER_CNT);

    timerdev.class = class_create(THIS_MODULE,TIMER_NAME);
    if(IS_ERR(timerdev.class)){
        return PTR_ERR( timerdev.class);
    }

    timerdev.device = device_create(timerdev.class,NULL,timerdev.devid,NULL,timerdev.name);
    if(IS_ERR(timerdev.device)){
        return PTR_ERR(timerdev.device);
    }

    init_timer(&timerdev.timer);
    timerdev.timer.function = timer_function;
    timerdev.timer.data = (unsigned long)&(timerdev);

}

static void timer_exit(void)
{
    gpio_set_value(timerdev.led_gpio,1);
    del_time_sync(&timerdev.timer);

    cdev_del(&timerdev.cdev);
    unregister_chrdev_region(timerdev.devid,TIMER_CNT);
    device_destory(timerdev.class,timerdev.devid);
    class_destory(timerdev.class);

}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL")
MODULE_AUTHOR("k")















