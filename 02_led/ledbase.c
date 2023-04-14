#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/delay.h>
#include<linux/ide.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/errno.h>
#include<linux/gpio.h>
#include<asm/mach/map.h>
#include<asm/uaccess.h>
#include<asm/io.h>

#define LED_MAJOR     200
#define LED_NAME      "LED"

#define LEDOFF       0
#define LEDON        1

#define CCM_CCGR1_BASE              (0x020C406C)
#define SW_MUX_GPIO1_IO03_BASE      (0x020E0068)
#define SW_PAD_GPIO1_IO03_BASE      (0x020E02F4)
#define GPIO1_DR_BASE               (0x0209C000)
#define GPIO2_GDIR_BASE             (0x0209C004)


static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

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

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
}

static void led_resigister_init(void)
{
     IMX6U_CCM_CCGR1 =  ioremap (CCM_CCGR1_BASE,4);


}

static int __init led_init(void)
{

}





