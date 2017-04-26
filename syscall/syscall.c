/*
 *	syscall.c
 *
 *	System cell "stealing" sample
 */

/*
 * The necessary header files
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>

/*
 * For the current process structure we need to know the current user
 */

#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/cred.h>

/*
 * The system call table (a table of functions). We
 * just define this as external, and the kernel will
 * fill it up for us when we insmod
 *
 * sys_call_table is no longer exported in 2.6.x kernels. If you want to try
 * this DANGEROUS module yo uwill have to apply the supplied patch
 * against your current kernel and recompile it
 */

extern void *sys_call_table[];

/*
 * UID we want to spy on - sssshhhhh!
 */

static int uid;
module_param(uid, int, 0644);

/*
 * A pointer to the origional system call. We dont call the origional function
 * because the system call may have been replaced. This is not safe since if
 * another module replaced sys_open before us, we'll call the ufnction in that
 * module, may be removed before us
 *
 * Also we can't sys_open. Its a static variable that is not exported
 */

asmlinkage int(*original_call) (const char *, int, int);

/*
 *
 * Replacing sys_open with this. 
 */

asmlinkage int our_sys_open(const char *filename, int flags, int mode)
{
	int i = 0;
	char ch;

	/*
	 * Is this the user we're spying on?
	 */

	if (uid == current_uid().val) {
		printk("Opened file by %d: ", uid);
		do {
			get_user(ch, filename + i);
			i++;
			printk("%c", ch);
		} while (ch != 0);
		printk("\n");

	}

	return original_call(filename, flags, mode);
}

int init_module()
{
	/*
	 * Too late now, but...
	 */

	printk(KERN_ALERT "I'm dangerous, I hope you did a ");
	printk(KERN_ALERT "sync before you insmodded me\n");
	printk(KERN_ALERT "My counterpart, cleanup_module, is even");
	printk(KERN_ALERT "more dangerous. If\n");
	printk(KERN_ALERT "you value your file system, it will ");
	printk(KERN_ALERT "be \"sync; rmmod\" \n");
	printk(KERN_ALERT "when you remove this module.\n");

	/*
	 * Keep a pointer to the original function in
	 * origional call, and the replace system call
	 * in the system call table with our_sys_open
	 */
	original_call = sys_call_table[__NR_open];
	sys_call_table[__NR_open] = our_sys_open;

	printk(KERN_INFO "Spying on UID:%d\n", uid);

	return 0;
}

void cleanup_module()
{
	if (sys_call_table[__NR_open] != our_sys_open) {
		printk(KERN_ALERT "Somebody else also played with the ");
		printk(KERN_ALERT "open system call\n");
		printk(KERN_ALERT "The system may be left in ");
		printk(KERN_ALERT "an unstable state");

	}

	sys_call_table[__NR_open] = original_call;
}

