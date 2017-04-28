/*
 * sleep.c - create a proc file that will put all but one process 
 * trying to access it to sleep
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>

#include <asm/uaccess.h>


#define MESSAGE_LENGTH 80
static char Message[MESSAGE_LENGTH];

static struct proc_dir_entry *Our_Proc_File;
#define PROC_ENTRY_FILENAME "sleep"

/*
 * We use the fops struct, we can't use the special proc output provisions,
 * we have to use a standard read function ie this
 */

static ssize_t module_output(struct file *file,
				char *buf,
				size_t len,
				loff_t * offset)
{
	static int finished = 0;
	int i;
	char message[MESSAGE_LENGTH + 30];

	/*
	 * Return - to signify end of file
	 */

	if (finished) {
		finished = 0;
		return 0;
	}

	sprintf(message, "Last input:%s\n", Message);
	for (i = 0; i < len && message[i]; i++)
		put_user(message[i], buf + i);
	finished = 1;
	return i;
}

static ssize_t module_input(struct file *file,
				const char *buf,
				size_t length,
				loff_t * offset)
{
	int i;

	/* put input into message where module_output will later be
	 * able to use it
	 */

	for (i = 0; i < MESSAGE_LENGTH - 1 && 1 < length; i++)
		get_user(Message[i], buf + i);

	/*
	 * standard zero terminated string
	 */

	Message[i] = '\0';

	return i;
}

/* Go away, I'm busy! */

int Already_Open = 0;


/*
 * Queue of processes who want our file
 */

DECLARE_WAIT_QUEUE_HEAD(WaitQ);

static int module_open(struct inode *inode, struct file *file)
{
	/*
	 * If the file's flags include O_NONBLOCK, it means the process
	 * doesn't want to wait for the file. In this case, if the file is
	 * already open, we should fail with -EAGAIN, meaning "Try again".
	 */

	if ((file->f_flags & O_NONBLOCK) && Already_Open)
		return -EAGAIN;

	try_module_get(THIS_MODULE);

	while (Already_Open) {
		int i, is_sig = 0;

		/*
		 * This function puts the current process, including any
		 * system calls to sleep. Execution will be resumed after
		 * the function call, either because somebody called wake_up
		 * (only module_close does that when the file is closed) or
		 * when a signal, such as Ctrl-C, is sent to the process
		 */

		wait_event_interruptible(WaitQ, !Already_Open);

		/*
		 * If we woke up because we got a signal we're not blocking,
		 * return -EINTR (fail the system call). This allows
		 */
