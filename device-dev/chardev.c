/*
 * chardev.c: Creates read-only char (unblocked) device that returns how many times the device has been read
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/* Function Prototypes */

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name in /proc/devices */
#define BUF_LEN 80		/* Max length of the messages from the device */

/*
 * Global variables are declared as static, so are global within the file
 */

static int Major; /* Major number (Device driver used) */
static int Device_Open = 0;	/* Device Open? */

static char msg[BUF_LEN];	/* string device will use when asked */
static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
}; /* file operation information */

/*
 * Called on load
 */

int init_module(void) {
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various numbers and try cat + echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

/*
 * This function is called when unloading
 */

void cleanup_module(void)
{
	/*
	 * Unregister the device
	 */

	unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */

static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you, %d times Hello!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/*
 * Called when a process closes the device file
 */

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;	/* Device is now ready for next process */

	/*
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get rid of the module.
	 */

	module_put(THIS_MODULE);

	return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to read from it
 */

static ssize_t device_read(struct file *filp,
				char *buffer,
				size_t length,
				loff_t * offset)
{
	/*
	 * Number of bytes written to buffer
	 */

	int bytes_read = 0;

	/*
	 * If at end of message,
	 * return 0 signifying end of file
	 */

	if (*msg_Ptr == 0)
		return 0;

	/*
	 * Put data in buffer
	 */

	while (length && *msg_Ptr) {
		
		/*
		 * The buffer is in the user data segment, not the kernel so "*" won't work, we have to put_user, copying data from kernel segment to our user segment.
		 */

		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/*
	 * Most read functions return the number of bytes put into the buffer
	 */

	return bytes_read;

}

/*
 * Called upon process write to dev file ie: echo "hiya!" > /dev/hello
 */


static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation is not supported.\n");
	return -EINVAL;
}
