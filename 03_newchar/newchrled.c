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

#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define  NEWCHRLED_CNT        1
#define  NEWCHRLED_NAME       "newcharled"

#define  LEDOFF               0
#define  LEDON                1

#define CCM_CCGR1_BASE              (0x020C406C)
#define SW_MUX_GPIO1_IO03_BASE      (0x020E0068)
#define SW_PAD_GPIO1_IO03_BASE      (0x020E02F4)
#define GPIO1_DR_BASE               (0x0209C000)
#define GPIO1_GDIR_BASE             (0x0209C004)


static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

struct newchrled_dev{
	dev_t devid;
	struct cdev  cdev;
	struct class *class;
	struct device *device;
	int major;
	int minor;
}

struct newchrled_dev newchrled;


static void led_switch(u8 sta)
{

    u32 val =0;
    if(LEDON == sta){
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);
        writel(val, GPIO1_DR);
    }
    else if(LEDOFF == sta){
        val = readl(GPIO1_GDIR);
        val |= (1 << 3);
        writel(val,GPIO1_DR);
    }

}


static int led_open(struct inode *inode,   struct file *filp)
{
	filp->private_data = &newchrled;
    return 0;
}

static ssize_t led_read(struct file *filp,char __user *buf,size_t cnt,loff_t *offt)
{
    return 0;
}

static ssize_t led_write(struct file *filp,char __user *buf,size_t cnt,loff_t *off_t)
{
    int retvalue = 0;
    unsigned char databuf[1];
    unsigned char ledstat;

    retvalue = copy_from_user(databuf, buf,cnt);

    if(retvalue < 0){
        printk("kernel write failed! \r\n");
        return -EFAULT;
    }

    ledstat = databuf[0];

    if(LEDON== ledstat){
        led_switch(LEDON);
    }else if(LEDOFF== ledstat){
        led_switch(LEDOFF);
    }

    return 0;

}

static int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}


static struct file_operations newchrled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
}


static void led_register_init(void)
{   
    u32 val =0;
    //寄存器地址映射
    IMX6U_CCM_CCGR1 =  ioremap (CCM_CCGR1_BASE,4);
    SW_MUX_GPIO1_IO03 = ioremap (SW_MUX_GPIO1_IO03_BASE,4);
    SW_PAD_GPIO1_IO03 = ioremap (SW_PAD_GPIO1_IO03_BASE,4);
    GPIO1_DR = ioremap (GPIO1_DR_BASE,4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE,4);

    //使能GPIO时钟
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26);
    val |= (3 << 26);
    writel(val,IMX6U_CCM_CCGR1);

    //gpio1_io03 引脚复用
    writel(5,SW_MUX_GPIO1_IO03);

    //设置IO属性
    writel(0x10B0,SW_PAD_GPIO1_IO03);

    //设置gpio1_io3为输出功能
    val = readl(GPIO1_GDIR);
    val &= ~(1 << 3);
    val |= (1<<3);
    writel(val,GPIO1_GDIR);

    //默认关闭LED
	val = readl(GPIO1_DR);
	val |= (1 << 3);
    writel(val,GPIO1_DR);


}


static int __init led_init(void)
{
    int retvalue = 0;

    led_register_init();
     
	if(newchrled.major){
		newcheled.devid = MKDEV(newchrled.major,0);
        register_chrdev_region(newchrled.devid,NEWCHRLED_CNT,NEWCHRLED_NAME);   
	}
    else{
        alloc_chrdev_region(&newchrled.devid,0,NEWCHRLED_CNT,NEWCHRLED_NAME);
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
    }

    printk("newchrled major=%d,minor=%d",newchrled.major,newchrled.minor);

    //初始化cdev
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &newchrled_fops);
    cdev_add(&newchrled.cdev,newchrled.devid,NEWCHRLED_CNT);

    newchrled.class = class_create(THIS_MODULE,NEWCHRLED_NAME);

    if(IS_ERR(newchrled.class)){
        return PTR_ERR(newchrled.class);
    }

    newchrled.device = device_create(newchrled.class,NULL,newchrled.devid,NULL,NEWCHRLED_NAME);

    if(IS_ERR(newchrled.device)){
        return PTR_ERR(newchrled.device);
    }

    return 0;


}

static void __exit led_exit(void)
{
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

    cdev_del(&newchrled.cdev);
    unregister_chrdev_region(newchrled.devid,NEWCHRLED_CNT);

    device_destroy(newchrled.class,newchrled.devid);
    class_destroy(newchrled.class);

}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("k");













