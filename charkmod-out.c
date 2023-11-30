/**
 * File:	charkmod_out.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 */

#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#include "struct.h"
#define DEVICE_NAME "charkmod-out" // Device name.
#define CLASS_NAME "charr"	  ///< The device class -- this is a character device drive
#define BUFFER_SIZE 1024      // The max size of the buffer
#define MESSAGE_SIZE 256      // The max size of the message

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("John Aedo");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("charkmod_out Kernel Module"); ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *charkmod_outClass = NULL;	///< The device-driver class struct pointer
static struct device *charkmod_outDevice = NULL; ///< The device-driver device struct pointer

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.read = read,
};

extern word_buffer g_buffer;

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "charkmod_out: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "charkmod_out could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "charkmod_out: registered correctly with major number %d\n", major_number);

	// Register the device class
	charkmod_outClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charkmod_outClass))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(charkmod_outClass); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "charkmod_out: device class registered correctly\n");

	// Register the device driver
	charkmod_outDevice = device_create(charkmod_outClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(charkmod_outDevice))
	{								 // Clean up if there is an error
		class_destroy(charkmod_outClass); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(charkmod_outDevice);
	}
	printk(KERN_INFO "charkmod_out: device class created correctly\n"); // Made it! device was initialized

	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "charkmod_out: removing module.\n");
	device_destroy(charkmod_outClass, MKDEV(major_number, 0)); // remove the device
	class_unregister(charkmod_outClass);						  // unregister the device class
	class_destroy(charkmod_outClass);						  // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "charkmod_out: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
      
    // initialize the buffer
    g_buffer = 
    (word_buffer){
        .start = 0,
        .end = 0,
        .full = 0
    };
	printk(KERN_INFO "charkmod_out: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
    // didn't allocate any memory so no need to do anything
	printk(KERN_INFO "charkmod_out: device closed.\n");
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
    while((g_buffer.start != g_buffer.end || g_buffer.full) && g_buffer.buffer[g_buffer.start] != '\0'){
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
        printk(KERN_INFO "charkmod_out: could not write to buffer");
    }

    // in case while loop stopped because it became empty
    if(g_buffer.start != g_buffer.end){
        g_buffer.start++;
        g_buffer.start %= BUFFER_SIZE;
        ind++;
    }


	printk(KERN_INFO "charkmod_out: read stub. start: %d, end: %d", g_buffer.start, g_buffer.end);
	return ind;
}

/*
 * Writes to the device
 */


