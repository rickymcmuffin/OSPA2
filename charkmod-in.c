/**
 * File:	charkmod_in.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 */

#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#include "struct.h"
#define DEVICE_NAME "charkmod-in" // Device name.
#define CLASS_NAME "charw"	  ///< The device class -- this is a character device drive
#define BUFFER_SIZE 1024      // The max size of the buffer
#define MESSAGE_SIZE 256      // The max size of the message

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("John Aedo");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("charkmod_in Kernel Module"); ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *charkmod_inClass = NULL;	///< The device-driver class struct pointer
static struct device *charkmod_inDevice = NULL; ///< The device-driver device struct pointer

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.write = write,
};

extern word_buffer g_buffer;

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "charkmod_in: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "charkmod_in could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "charkmod_in: registered correctly with major number %d\n", major_number);

	// Register the device class
	charkmod_inClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charkmod_inClass))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(charkmod_inClass); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "charkmod_in: device class registered correctly\n");

	// Register the device driver
	charkmod_inDevice = device_create(charkmod_inClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(charkmod_inDevice))
	{								 // Clean up if there is an error
		class_destroy(charkmod_inClass); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(charkmod_inDevice);
	}
	printk(KERN_INFO "charkmod_in: device class created correctly\n"); // Made it! device was initialized

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
	printk(KERN_INFO "charkmod_in: removing module.\n");
	device_destroy(charkmod_inClass, MKDEV(major_number, 0)); // remove the device
	class_unregister(charkmod_inClass);						  // unregister the device class
	class_destroy(charkmod_inClass);						  // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "charkmod_in: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
      
    // initialize the buffer
	printk(KERN_INFO "charkmod_in: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
    // didn't allocate any memory so no need to do anything
	printk(KERN_INFO "charkmod_in: device closed.\n");
	return 0;
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
        printk(KERN_INFO "charkmod_in: error copying from user");
        return 0;
    }
    printk(KERN_INFO "charkmod_in: g_buffer %d %d", g_buffer.start, g_buffer.end);



    // no more writing
    if(g_buffer.full)
        return 0;

       
    for(i = 0; i < len ; i++){
        printk(KERN_INFO "charkmod_in: adding char %d", i);
        if((g_buffer.end + 1) % BUFFER_SIZE == g_buffer.start){ // when buffer is full, no more writing.
            break;
        }
        g_buffer.buffer[g_buffer.end] = message[i]; 
        g_buffer.end+=1;
        g_buffer.end %= BUFFER_SIZE;

    }
    if((g_buffer.end + 1) % BUFFER_SIZE == g_buffer.start){ // this means we've filled it up
        g_buffer.full = 1;
    
    }
    // g_buffer.buffer[g_buffer.end] = '\0';
    // g_buffer.end+=1;
    // g_buffer.end %= BUFFER_SIZE;
	printk(KERN_INFO "charkmod_in: write stub. start: %d, end: %d", g_buffer.start, g_buffer.end);
	return i;
}

