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
#define GPIO1_GDIR_BASE             (0x0209C004)


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

}

static int __init led_init(void)
{
    int retvalue = 0;
    led_register_init();

    retvalue = register_chrdev(LED_MAJOR,LED_NAME,&led_fops);
    if(retvalue <0){
        printk("register_chrdev failed \n\r");
        return -EIO;
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

    unregister_chrdev(LED_MAJOR,LED_NAME);

}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("k");







