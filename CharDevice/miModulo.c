#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define DEVICE_NAME "DeviceMauro"
#define BUF_LEN 80 

static int Major; 
static int Device_Open = 0; 

static char msg[BUF_LEN]; 
static char *msg_Ptr;
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0) {
        printk(KERN_ALERT "UNGS: el registro del Char Device fallo %d\n", Major);
        return Major;
    }
    printk(KERN_INFO "UNGS: Se le fue asignado el Major number de %d \n", Major);
    printk(KERN_INFO "UNGS: Se puede crear el device con: 'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

    return 0;
}

void cleanup_module(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open){
        return -EBUSY;
    }   
    Device_Open++;
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read( struct file *filp,
                            char *buffer,
                            size_t length, 
                            loff_t * offset)
{
    int bytes_read = 0;

    if (*msg_Ptr == 0)
        return 0;

    while (length && *msg_Ptr) {

        put_user(*(msg_Ptr++), buffer++);
        length--;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *filp,
                            const char *buff,
                            size_t len,
                            loff_t * off)
{   
    int i;
    
    for (i = 0; i < len && i < BUF_LEN; i++)
        get_user(msg[i], buff + i);

    msg_Ptr = msg;

    return i;
}


MODULE_LICENSE ("GPL ") ;
MODULE_AUTHOR ("UNGS ") ;
MODULE_DESCRIPTION ("Un primer driver ") ;