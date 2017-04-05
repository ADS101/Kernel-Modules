/*
 * procfs1.c - create a "file" in proc
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#define procfs_name "helloworld"

/**
 * This structure holds information about the /rpoc file
 *
 */

struct proc_dir_entry *Our_Proc_File;

/*
 * Put data into the proc fs file.
 *
 * Arguments
 *
 * 1. The buffer where the data is to be inserted, if you decide to use it.
 * 2. A pointer to a pointer to charactersThis is useful if you dont want to use the buffer allocated by the kernel
 * 3. The file index
 * 4. The size of the buffer in the first argument.
 * 5. Boolean (0, 1) value to indicate EOF.
 * 6. A pointer to data
 *
 * Usage and Return Value
 * ===========
 * A return value of zero means you have no further information at the time (end of file). A negative return value is an error condition
 */

int procfile_read(char *buffer,
		char **buffer_location,
		off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s) called \n", procfs_name);

	/*
	 *
	 * We give all of our information in one go so if the user asks us if we have more information the
	 * answr should always be no
	 *
	 * This is important because the standard read function from the library would continue to issue te read system call until the kernel replies that there is no more.
	 */

	if(offset > 0) {
		/* we have finished read, return 0 */
		ret = 0;
	} else {
		/* fill the buffr, return the buffer size */
		ret = sprintf(buffer, "HelloWorld!\n");
	}
	
	return ret;
}

int init_module()
{
	Our_Proc_File = proc_create(procfs_name, 0644, NULL);

	if (Our_Proc_File == NULL) {
		remove_proc_entry(procfs_name, &proc_root);
		printk(KERN_ALERT "Error: Could not initalize /proc/%s\n",
				procfs_name);
		return -ENOMEM;
	}

	Our_Proc_File->read_proc	= procfile_read;
	Our_Proc_File->owner		= THIS_MODULE;
	Our_Proc_File->mode		= S_IFREG | S_IRUGO;
	Our_Proc_File->uid		= 0;
	Our_Proc_File->gid		= 0;
	Our_Proc_File->size		= 37;

	printk(KERN_INFO "/proc/%s created\n", procfs_name);
	return 0;
}

void cleanup_module()
{
	remove_proc_entry(procfs_name, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}
