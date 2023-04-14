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

#define CCM_CCGR1_MODE              (0x020C406C)
#define SW_MUX_GPIO1_IO03_BASE      (0x020E0068)
#define SW_PAD_GPIO1_IO03_BASE      (0x020E02F4)
#define GPIO1_DR_BASE               (0x0209C000)
#define GPIO2_GDIR_BASE             (0x0209C004)


static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

void led_switch(u8 sta)
{

    u32 val =0;
    if(LED0N == sta){
        val = readl(GDIO1_DR);
        val &= ~(1 << 3);
        writel(val, GPIO1_DR);
    }

}





