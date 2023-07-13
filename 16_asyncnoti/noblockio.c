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
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/fcntl.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define  IMX6UIRQ_CNT   1
#define  IMX6UIRQ_NAME  "asyncnoti"
#define  KEY0VALUE      0X01
#define  KEY0INVALID    0xFF
#define  KEY_NUM        1


struct irq_keydesc{
    int gpio;
    int irqnum;
    unsigned char value;
    char name[10];
    irqreturn_t (*handler)(int,void *);
};

struct imx6uirq_dev{
    dev_t devid;
    struct cdev cdev;
    struct class* class;
    struct device* device;
    int major;
    int minor;
    struct device_node* nd;
    atomic_t    keyvalue;
    atomic_t    releasekey;
    struct timer_list timer;
    struct irq_keydesc irqkeydesc[KEY_NUM];
    struct char  curkeynum;
    wait_queue_head_t r_wait;
    struct fasync_struct* async_queue;
}


struct imx6uirq_dev  imx6ulirq;

static irqreturn_t  key0_handler(int irq,void*dev_id)
{
    struct imx6uirq_dev* dev = (struct imx6uirq_dev*)(dev_id);

    dev->curkeynum = 0;
    dev->timer.data = (volatile long)dev_id;

    mod_timer(&dev->timer,jiffies + msecs_to_jiffies(10));
    return IRQ_RETVAL(IRQ_HANDLED);
}

void timer_function(unsigned long arg)
{
    unsigned char value;
    unsigned char num;
    struct irq_keydesc * keydesc;
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)(arg);
    num = dev->curkeynum;
    keydesc = &dev->irqkeydesc[num];
    value = gpio_get_value(keydesc->gpio);
    if(value == 0){
        atomic_set(&dev->keyvalue,keydesc->value); 
    }
    else{
        atomic_set(&dev->keyvalue,0x80|(keydesc->value));
        atomic_set(&dev->releasekey,1);
    }

    if(atomic_read(&dev->releasekey)){
        if(dev->async_queue){
            kill_fasync(&dev->async_queue,SIGIO,POLL_IN);
        }
    }

#if 0 
    if(atomic_read(&dev->releasekey)){
        wake_up_interruptible(&dev->r_wait);
    }
#endif
    
}

static int imx6uirq_fasync(int fd, struct file *filp,int on)
{
    struct imx6uireq *dev = (struct imx6uireq *)(filp->private_data);

    return fasync_helper(fd,filp,on,&dev->async_queue);
}

static int imx6uirq_release(struct inode* inode, struct file *filp)
{
    return imx6ulirq_fasync(-1,filp,0);
}

static int keyio_init(void)
{
    unsigned int i =0;
    int ret =0;

    imx6ulirq.nd = of_find_node_by_path("/key");
    if(imx6ulirq.nd == NULL){
        printk("key node not find\r\n");
        return -EINVAL;
    }

    for(i=0;i<KEY_NUM;i++){
        imx6ulirq.irqkeydesc[i].gpio = of_get_named_gpio(imx6ulirq.nd,"key-gpio",1);
        if(imx6ulirq.irqkeydesc[i].gpio < 0){
            printk("can't get key : %d\r\n" ,i);
        }
    }

    for(i=0;i<KEY_NUM;i++){
        memset(imx6ulirq.irqkeydesc[i].name,0,sizeof(imx6ulirq.irqkeydesc[i].name));
        sprintf(imx6ulirq.irqkeydesc[i].name,"KEY%d",i);
        gpio_request(imx6ulirq.irqkeydesc[i].gpio,imx6ulirq.irqkeydesc[i].name);
        gpio_direction_input(imx6ulirq.irqkeydesc[i].gpio);
        printk("key:%d:gpio = %d,irqnum = %d\r\n",i,imx6ulirq.irqkeydesc[i],gpio,
                imx6ulirq.irqkeydesc[i].irqnum);
    }

    imx6ulirq.irqkeydesc[0].handler = key0_handler;
    imx6ulirq.irqkeydesc[0].value = KEY0VALUE;

    for(i=0;i<KEY_NUM;i++){
        ret =  request_irq(imx6ulirq.irqkeydesc[i].irqnum,
                            imx6ulirq.irqkeydesc[i].handler,
                            IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                            imx6ulirq.irqkeydesc[i].name,&imx6ulirq);
        if(ret < 0){
            printk("irq %d request failed \r\n",imx6ulirq.irqkeydesc[i].irqnum);
            return -EFAULT;
        } 

    }

    init_timer(&imx6ulirq.timer);
    imx6ulirq.timer.function = timer_function;

    init_waitqueue_head(&imx6ulirq.r_wait);
    
    return 0;

}

static int imx6ulirq_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &imx6ulirq;
    return 0;
}

static ssize_t imx6ulirq_read(struct file* filp ,char __user *buf, 
                                size_t cnt, loff_t* offt)
{
    int ret = 0;
    unsigned char keyvalue =0;
    unsigned char releasekey = 0;
    struct imx6uirq_dev * dev = (strcut imx6uirq_dev*)(filp->private_data);
    DECLARE_WAITQUEUE(wait,current);
    if(filp->f_flags & O_NONBLOCK){
        if(atomic_read(&dev->releasekey) == 0){
            return -EAGAIN;
        }
    }
    else{
        if(atomic_read(&dev->releasekey) == 0){
            add_wait_queue(&dev->r_wait,&wait);
            __set_current_state(TASK_INTERRUPTIBLE);
            schedule();
            if(signal_pending(current)){
                ret = -ERESTARTSYS;
                goto wait_error;
            }
            __set_current_state(TASK_RUNNING);
            remove_wait_queue((&dev->r_wait),&wiat);
        }

    }


    keyvalue = atomic_read(&dev->keyvalue);
    releasekey = atomic_read(&dev->releasekey);
    if(releasekey){
        if(keyvalue & 0x80){
            keyvalue &= ~0x80;
            ret = copy_to_user(buf, &keyvalue,sizeof(keyvalue));
        }
        else{
            goto data_error;
        }
        atomic_set(&dev->releasekey,0);     
    }
    else{
        goto data_error;
    }

    return 0;

wait_error:
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&dev->r_wait,&wait);
    return ret;
data_error:
    return  -EINVAL;
}

unsigned int imx6uirq_poll(struct file* filp, struct poll_table_struct *wait)
{
    unsigned int mask ;
    struct imx6uirq_dev *dev = (strcut imx6uirq_dev*)(filp->private_data);
    
    poll_wait(filp,&dev->r_wait,wait);

    if(atomic_read(&dev->releasekey)){
        mask = POLLIN| POLLRDNORM;
    }
    
    return mask;

}

static struct file_operations imx6ulirq_fop{
    .owner = THIS_MODULE,
    .open = imx6ulirq_open,
    .read = imx6ulirq_read,
    .poll = imx6uirq_poll,
    .release = imx6uirq_release,
    .fasync = imx6uirq_fasync,
};

static int __init imx6ulirq_init(void)
{
    if(imx6ulirq.major){
        imx6ulirq.devid = MKDEV(imx6ulirq.major,0);
        register_chrdev_region(imx6ulirq.devid,IMX6UIRQ_CNT,IMX6UIRQ_NAME);
    }
    else{
        alloc_chrdev_region(&imx6ulirq.devid,0,IMX6UIRQ_CNT,IMX6UIRQ_NAME);
        imx6ulirq.major = MAJOR(imx6ulirq.devid);
        imx6ulirq.minor = MINOR(imx6ulirq.devid);
    }

    cdev_init(&imx6ulirq.cdev,&imx6ulirq_fop);
    cdev_add(&imx6ulirq.cdev,imx6ulirq.devid,IMX6UIRQ_CNT);

    imx6ulirq.class = class_create(THIS_MODULE,IMX6UIRQ_NAME);
    if(IS_ERR(imx6ulirq.class)){
        return PTR_ERR(imx6ulirq.class);
    }
    
    imx6ulirq.device = device_create(imx6ulirq.class,NULL,imx6ulirq.devid,NULL,IMX6UIRQ_NAME);
    if(IS_ERR(imx6ulirq.device)){
        return PTR_ERR(imx6ulirq.device);
    }

    atomic_set(&imx6ulirq.keyvalue,KEY0INVALID);
    atomic_set(&imx6ulirq.releasekey,0);
    keyio_init();

    return 0;
}

static void __exit imx6ulirq_exit(void)
{
    unsigned int i =0;
    del_timer_sync(&imx6ulirq.timer);
    for(i=0;i<KEY_NUM;i++){
        free_irq(&imx6ulirq.irqkeydesc[i].irqnum,&imx6ulirq);
    }

    cdev_del(&imx6ulirq.cdev);
    unregister_chrdev_region(imx6ulirq.devid,IMX6UIRQ_CNT);
    device_destory(imx6ulirq.class,imx6ulirq.devid);
    class_destory(imx6ulirq.class);

}

module_init(imx6ulirq_init);
module_exit(imx6ulirq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("k")














