#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eugene Zubkov <evzubkov@inbox.ru>");
MODULE_VERSION("0.9");

#define MODULE_NAME "chardevice_module"
#define DEVICE_NAME "chardevice"
#define DEVICE_CLASS "chardevice_class"
#define NUM_DEVICES 2
#define DEVICE_FIRST_INDEX 0  

static int major;
static struct cdev some_device;
static struct class *device_class;

static int __init device_init(void);
static void __exit device_exit(void);
static ssize_t device_read(struct file * file, char * buf, size_t count, loff_t *pos);

static const struct file_operations device_file_operations = {
   .owner = THIS_MODULE,
   .read  = device_read,
};

static int __init device_init(void) 
{
	int ret, i;
	dev_t dev;
	struct device *device_ptr;

	if(major) {
		dev = MKDEV(major, DEVICE_FIRST_INDEX);
		ret = register_chrdev_region(dev, NUM_DEVICES, MODULE_NAME);
	} else {
		ret = alloc_chrdev_region(&dev, DEVICE_FIRST_INDEX, NUM_DEVICES, MODULE_NAME);
		major = MAJOR(dev);
	}
	if(ret < 0) {
		printk(KERN_ERR "Can't get major %d\n", major);
		return ret;
	}

	cdev_init(&some_device, &device_file_operations);
	some_device.owner = THIS_MODULE;

	ret = cdev_add(&some_device, dev, NUM_DEVICES);
	if(ret < 0) {
		printk(KERN_ERR "Can't add char device\n");
		goto cdev_err;
	}

	device_class = class_create(DEVICE_CLASS);
    if (IS_ERR(device_class)) {
        ret = PTR_ERR(device_class);
        printk(KERN_ERR "Failed to create device class: %d\n", ret);
        goto class_err;
    }
	
	for(i = 0; i < NUM_DEVICES; i++) {
        device_ptr = device_create(device_class, NULL, MKDEV(major, i), NULL, "%s%d", DEVICE_NAME, i);
        if (IS_ERR(device_ptr)) {
            ret = PTR_ERR(device_ptr);
            printk(KERN_ERR "Failed to create device %s%d\n", DEVICE_NAME, i);
            
            while (--i >= 0)
                device_destroy(device_class, MKDEV(major, i));
            
            goto device_err;
        }
    }
	
	printk(KERN_INFO "Module installed %d:[%d-%d]\n", 
		MAJOR(dev), 
		DEVICE_FIRST_INDEX, 
		DEVICE_FIRST_INDEX + NUM_DEVICES - 1);
	return 0;

device_err:
    class_destroy(device_class);
class_err:
    cdev_del(&some_device);
cdev_err:
    unregister_chrdev_region(MKDEV(major, DEVICE_FIRST_INDEX), NUM_DEVICES);
    return ret;
}

static void __exit device_exit(void) 
{
   dev_t dev;
   int i;
   for( i = 0; i < NUM_DEVICES; i++ ) {
      dev = MKDEV(major, DEVICE_FIRST_INDEX + i);
      device_destroy(device_class, dev);
   }
   class_destroy(device_class);
   cdev_del(&some_device);
   unregister_chrdev_region(MKDEV( major, DEVICE_FIRST_INDEX), NUM_DEVICES);
   printk(KERN_INFO "Module removed\n");
}

static const char *buffer = "some data in module\n";

static ssize_t device_read(struct file * file, char * buf, size_t count, loff_t *pos)
{
	int len = strlen(buffer);
	printk(KERN_INFO "Read : %ld\n", (long)count);
	if(count < len) 
		return -EINVAL;
	if(*pos != 0) {
		printk(KERN_INFO "Read return : 0\n");
		return 0;
	}

	if(copy_to_user(buf, buffer, len)) 
		return -EINVAL;
	*pos = len;
	printk(KERN_INFO "Read return : %d\n", len);
	return len;
}

module_init(device_init);
module_exit(device_exit);