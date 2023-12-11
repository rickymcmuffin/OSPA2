/**
 * File:	lkmasg2.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 */

#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#define DEVICE_NAME "lkmasg2" // Device name.
#define CLASS_NAME "char"	  ///< The device class -- this is a character device drive
#define BUFFER_SIZE 1024      // The max size of the buffer
#define MESSAGE_SIZE 1024      // The max size of the message

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("John Aedo");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("lkmasg2 Kernel Module"); ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *lkmasg2Class = NULL;	///< The device-driver class struct pointer
static struct device *lkmasg2Device = NULL; ///< The device-driver device struct pointer

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.read = read,
		.write = write,
};

typedef struct {
    char buffer[BUFFER_SIZE];
    int start;
    int end;
    int full; // if start and end are equal, full determines whether it is empty or full
} word_buffer;

static word_buffer g_buffer;

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "lkmasg2: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "lkmasg2 could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "lkmasg2: registered correctly with major number %d\n", major_number);

	// Register the device class
	lkmasg2Class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(lkmasg2Class))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(lkmasg2Class); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "lkmasg2: device class registered correctly\n");

	// Register the device driver
	lkmasg2Device = device_create(lkmasg2Class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(lkmasg2Device))
	{								 // Clean up if there is an error
		class_destroy(lkmasg2Class); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(lkmasg2Device);
	}
	printk(KERN_INFO "lkmasg2: device class created correctly\n"); // Made it! device was initialized

    g_buffer = 
    (word_buffer){
        .start = 0,
        .end = 0,
        .full = 0
    };

	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "lkmasg2: removing module.\n");
	device_destroy(lkmasg2Class, MKDEV(major_number, 0)); // remove the device
	class_unregister(lkmasg2Class);						  // unregister the device class
	class_destroy(lkmasg2Class);						  // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "lkmasg2: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
      
    // initialize the buffer
	printk(KERN_INFO "lkmasg2: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
    // didn't allocate any memory so no need to do anything
	printk(KERN_INFO "lkmasg2: device closed.\n");
	return 0;
}

/*
 * Reads from device, displays in userspace, and deletes the read data
 */
static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    ssize_t ind = 0;

    char message[MESSAGE_SIZE];
    unsigned long err;

    // read as long as it's not empty and string doesn't reach end
    while((g_buffer.start != g_buffer.end || g_buffer.full) && ind < len-1){
        message[ind] = g_buffer.buffer[g_buffer.start++];
        g_buffer.start %= BUFFER_SIZE;
        ind++;
        //we have read a character so it can't be full
        g_buffer.full = 0;
    }
    // add end to buffer
    message[ind] = '\0';

    err = copy_to_user(buffer, message, ind+1);

    if(err != 0){
        printk(KERN_INFO "lkmasg2: could not write to buffer");
    }

    // in case while loop stopped because it became empty
    // if(g_buffer.start != g_buffer.end){
    //     g_buffer.start++;
    //     g_buffer.start %= BUFFER_SIZE;
    //     ind++;
    // }


	printk(KERN_INFO "lkmasg2: read stub. start: %d, end: %d", g_buffer.start, g_buffer.end);
	return ind;
}

/*
 * Writes to the device
 */
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    // this catastrophe checks if there is space in the buffer
    // if((g_buffer.start > g_buffer.end && g_buffer.start - g_buffer.end < len)
    //         || (g_buffer.start <= g_buffer.end && BUFFER_SIZE + g_buffer.start - g_buffer.end < len)){
    //
	   //  printk(KERN_INFO "error writing");
    //     return 0;
    // }
    //
    int i;
    char message[MESSAGE_SIZE];
    unsigned long err;
    err = copy_from_user(message, buffer, len); 

    if(err != 0){
        printk(KERN_INFO "lkmasg2: error copying from user");
        return 0;
    }



    // no more writing
    if(g_buffer.full)
        return 0;

       
    for(i = 0; i < len && message[i] != '\0'; i++){
        if((g_buffer.end + 1) % BUFFER_SIZE == g_buffer.start){ // when buffer is full, no more writing.
            i++;
            break;
        }
        g_buffer.buffer[g_buffer.end] = message[i]; 
        g_buffer.end+=1;
        g_buffer.end %= BUFFER_SIZE;

    }
    printk(KERN_INFO "lkmasg2: buffer full: %d", (g_buffer.end + 1) % BUFFER_SIZE == g_buffer.start);
    if((g_buffer.end + 1) % BUFFER_SIZE == g_buffer.start){ // this means we've filled it up
        printk(KERN_INFO "lkmasg2: buffer became full");
        g_buffer.full = 1;
    
    }
	printk(KERN_INFO "lkmasg2: write stub. start: %d, end: %d", g_buffer.start, g_buffer.end);
	return i;
}

