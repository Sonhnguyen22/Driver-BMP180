#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/math64.h> 

// ALL REG_BMP180_ADDR
struct bmp180_data{
    struct {
        s16 AC1;
        s16 AC2;
        s16 AC3;
        u16 AC4;
        u16 AC5;
        u16 AC6;
        s16 B1;
        s16 B2;
        s16 MB;
        s16 MC;
        s16 MD;

        s32 UT;
        s32 UP;
    }Calib;

  
    s32 T;
    s32 P;
   

    
    u8 OSS;
 
};
struct bmp180_data BMP180_D;
#define SEA_PRES                1013.25f  // Áp suất mực nước biển 
#define POWDIV                  0.1902949572f
#define REG_BMP180_CALI         0xAA
#define REG_BMP180_CONTROL      0xF4
#define REG_BMP180_DATA_MSB     0xF6
#define REG_BMP180_DATA_LSB     0xF7
#define REG_BMP180_DATA_XMSB    0xF8

#define DRIVER_NAME "bmp180_driver"
#define CLASS_NAME  "bmp180"
#define DEVICE_NAME "bmp180"

// IOCTL commands
#define bmp180_IOCTL_MAGIC 'b'
#define bmp180_IOCTL_WRITE_OSS  _IOR(bmp180_IOCTL_MAGIC, 1, int)
#define bmp180_IOCTL_READ_CALIB _IOR(bmp180_IOCTL_MAGIC, 2, int)
#define bmp180_IOCTL_READ_T     _IOR(bmp180_IOCTL_MAGIC, 3, int)
#define bmp180_IOCTL_READ_P     _IOR(bmp180_IOCTL_MAGIC, 4, int)
#define bmp180_IOCTL_READ_TP    _IOR(bmp180_IOCTL_MAGIC, 5, int)


static struct i2c_client *bmp180_client;
static struct class* bmp180_class = NULL;
static struct device* bmp180_device = NULL;
static int major_number;

static int bmp180_read_calib(struct i2c_client *client){
    u8 buf[22];
    if (i2c_smbus_read_i2c_block_data(client, REG_BMP180_CALI, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read accelerometer data\n");
        return -EIO;
    }

    // Combine high and low bytes to form 16-bit values
    BMP180_D.Calib.AC1   = (buf[0] << 8) | buf[1]; 
    BMP180_D.Calib.AC2   = (buf[2] << 8) | buf[3]; 
    BMP180_D.Calib.AC3   = (buf[4] << 8) | buf[5];
    BMP180_D.Calib.AC4   = (buf[6] << 8) | buf[7]; 
    BMP180_D.Calib.AC5   = (buf[8] << 8) | buf[9]; 
    BMP180_D.Calib.AC6   = (buf[10] << 8) | buf[11];
    BMP180_D.Calib.B1    = (buf[12] << 8) | buf[13]; 
    BMP180_D.Calib.B2    = (buf[14] << 8) | buf[15]; 
    BMP180_D.Calib.MB    = (buf[16] << 8) | buf[17];
    BMP180_D.Calib.MC    = (buf[18] << 8) | buf[19]; 
    BMP180_D.Calib.MD    = (buf[20] << 8) | buf[21]; 
   
    return 0;
}

static int bmp180_read_ut(struct i2c_client *client){
    u8 buf[2];
    if (i2c_smbus_write_byte_data(client, REG_BMP180_CONTROL, 0x2E) < 0) {
        printk(KERN_ERR "Failed to read Calib data\n");
        return -EIO;
    }
    
    // wait 
    switch (BMP180_D.OSS)
    {
        case 0:
            usleep_range(4500, 5000);  // Ngủ từ 4500 µs đến 5000 µs 
            break;
        case 1:
            usleep_range(7500, 8000);  // Ngủ từ 7500 µs đến 8000 µs 
            break;
        case 2:
            usleep_range(13500, 14000);  // Ngủ từ 13500 µs đến 14000 µs 
            break;
        case 3:
            usleep_range(25500, 26000);  // Ngủ từ 25500 µs đến 26000 µs 
            break;
    
        default:
            break;
    }

    if (i2c_smbus_read_i2c_block_data(client, REG_BMP180_DATA_MSB, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read UT data\n");
        return -EIO;
    }

    // Combine high and low bytes to form 16-bit values
    BMP180_D.Calib.UT = (buf[0] << 8) + buf[1]; 
    
   
    return 0;
}

static int bmp180_read_up(struct i2c_client *client){
    u8 buf[3];
    if (i2c_smbus_write_byte_data(client, REG_BMP180_CONTROL, 0x34 + (BMP180_D.OSS << 6)) < 0) {
        printk(KERN_ERR "Failed to read Calib data\n");
        return -EIO;
    }
    
    // wait 
    switch (BMP180_D.OSS)
    {
        case 0:
            usleep_range(4500, 5000);  // Ngủ từ 4500 µs đến 5000 µs 
            break;
        case 1:
            usleep_range(7500, 8000);  // Ngủ từ 7500 µs đến 8000 µs 
            break;
        case 2:
            usleep_range(13500, 14000);  // Ngủ từ 13500 µs đến 14000 µs 
            break;
        case 3:
            usleep_range(25500, 26000);  // Ngủ từ 25500 µs đến 26000 µs 
            break;
    
        default:
            break;
    }

    if (i2c_smbus_read_i2c_block_data(client, REG_BMP180_DATA_MSB, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read UT data\n");
        return -EIO;
    }

    // Combine high and low bytes to form 16-bit values
    BMP180_D.Calib.UP = ((buf[0] << 16) + (buf[1] << 8) + buf[2]) >> (8 - BMP180_D.OSS);
    return 0;
}

static int bmp180_read_all(void){
    bmp180_read_calib(bmp180_client);
    bmp180_read_ut(bmp180_client);
    bmp180_read_up(bmp180_client);
    return 0;
}

static int bmp180_calculate_temp(void){
    s32 X1, X2;
    X1 = ((BMP180_D.Calib.UT - BMP180_D.Calib.AC6) * BMP180_D.Calib.AC5) >> 15;
    X2 = (BMP180_D.Calib.MC << 11) / (X1 + BMP180_D.Calib.MD);
    BMP180_D.T = (X1 + X2 + 8) >> 4;
    return 0;
}

static int bmp180_calculate_pres(void){
    s32 X1, X2, X3, B3, B6;
    u32 B4, B7;
    X1 = ((BMP180_D.Calib.UT - BMP180_D.Calib.AC6) * BMP180_D.Calib.AC5) >> 15;
    X2 = (BMP180_D.Calib.MC << 11) / (X1 + BMP180_D.Calib.MD);
    B6 = X1 + X2 - 4000;
    X1 = (BMP180_D.Calib.B2 * (B6 * B6 >> 12)) >> 11;
    X2 = (BMP180_D.Calib.AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = (((BMP180_D.Calib.AC1 * 4 + X3) << BMP180_D.OSS) + 2) / 4;

    X1 = (BMP180_D.Calib.AC3 * B6) >> 13;
    X2 = (BMP180_D.Calib.B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = (BMP180_D.Calib.AC4 * (u32)(X3 + 32768)) >> 15;
    B7 = ((u32)(BMP180_D.Calib.UP - B3) * (50000 >> BMP180_D.OSS));

    if (B7 < 0x80000000)
    BMP180_D.P = (B7 * 2) / B4;
    else
    BMP180_D.P = (B7 / B4) * 2;

    X1 = (BMP180_D.P >> 8) * (BMP180_D.P >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * BMP180_D.P) >> 16;
    BMP180_D.P = BMP180_D.P + ((X1 + X2 + 3791) >> 4);
    return 0;
}

static long bmp180_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

    switch (cmd) {
        case bmp180_IOCTL_READ_T:
            bmp180_read_all();
            bmp180_calculate_temp();
            if (copy_to_user((int __user *)arg, &BMP180_D.T, sizeof(BMP180_D.T))) {
                return -EFAULT;
            }
            break;
        case bmp180_IOCTL_READ_P:
            bmp180_read_all();
            bmp180_calculate_pres();
            if (copy_to_user((int __user *)arg, &BMP180_D.P, sizeof(BMP180_D.P))) {
                return -EFAULT;
            }
            break;
        case bmp180_IOCTL_READ_TP:
            int TP[2];
            bmp180_read_all();
            bmp180_calculate_temp();
            bmp180_calculate_pres();
            TP[0] = BMP180_D.T;
            TP[1] = BMP180_D.P;
            if (copy_to_user((int __user *)arg, &TP, sizeof(TP))) {
                return -EFAULT;
            }
            break;
        case bmp180_IOCTL_READ_CALIB:
            bmp180_read_all();
            if (copy_to_user((int __user *)arg, &BMP180_D.Calib, sizeof(BMP180_D.Calib))) {
                return -EFAULT;
            }
            break;
        case bmp180_IOCTL_WRITE_OSS:
            if (copy_from_user(&BMP180_D.OSS, (u8 __user *)arg, sizeof(BMP180_D.OSS))) {
                return -EFAULT;
            }
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static int bmp180_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "bmp180 device opened\n");
    return 0;
}

static int bmp180_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "bmp180 device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = bmp180_open,
    .unlocked_ioctl = bmp180_ioctl,
    .release = bmp180_release,
};

static int bmp180_probe(struct i2c_client *client)
{
    printk(KERN_INFO "bmp180 probe called\n");  
    bmp180_client = client;

    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    bmp180_class = class_create(CLASS_NAME);
    if (IS_ERR(bmp180_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(bmp180_class);
    }

    bmp180_device = device_create(bmp180_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(bmp180_device)) {
        class_destroy(bmp180_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(bmp180_device);
    }

    printk(KERN_INFO "bmp180 driver installed\n");
    return 0;
}

static void bmp180_remove(struct i2c_client *client)
{
    device_destroy(bmp180_class, MKDEV(major_number, 0));
    class_unregister(bmp180_class);
    class_destroy(bmp180_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "bmp180 driver removed\n");
}

static const struct of_device_id bmp180_of_match[] = {
    { .compatible = "invensense,bmp180", },
    { },
};

MODULE_DEVICE_TABLE(of, bmp180_of_match);

static struct i2c_driver bmp180_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(bmp180_of_match),
    },
    .probe      = bmp180_probe,
    .remove     = bmp180_remove,
};

static int __init bmp180_init(void)
{
    printk(KERN_INFO "Initializing bmp180 driver\n");
    return i2c_add_driver(&bmp180_driver);
}

static void __exit bmp180_exit(void)
{
    printk(KERN_INFO "Exiting bmp180 driver\n");
    i2c_del_driver(&bmp180_driver);
}

module_init(bmp180_init);
module_exit(bmp180_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("bmp180 I2C Client Driver with IOCTL Interface");
MODULE_LICENSE("GPL");