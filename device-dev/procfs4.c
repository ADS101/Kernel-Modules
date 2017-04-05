/**
 * procfs4.c
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME	"iter"

MODULE_AUTHOR("James Doidge");
MODULE_LICENSE("GPL");

static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter = 0;

	if ( *pos == 0 )
	{
		return &counter;
	}
	else
	{
		*pos = 0;
		return NULL;
	}
}

static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	unsigned long *tmp_v = (unsigned long*)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}

static void my_seq_stop(struct seq_file *s, void *v)
{
	/* static value in start() */
}

static int my_seq_show(struct seq_file *s, void *v)
{
	loff_t *spos = (loff_t *) v;

	seq_printf(s, "%Ld\n", *spos);
	return 0;
}

static struct seq_operations my_seq_ops = {
	.start	= my_seq_start,
	.next	= my_seq_next,
	.stop	= my_seq_stop,
	.show	= my_seq_show
};

static int my_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
}

static struct file_operations my_file_ops = {
	.owner	= THIS_MODULE,
	.open	= my_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release	= seq_release
};

int init_module(void)
{
	struct proc_dir_entry *entry;

	entry = create_proc_entry(PROC_NAME, 0, NULL);
	if (entry) {
		entry->proc_fops = &my_file_ops;
	}

	return 0;
}

void cleanup_module(void)
{
	remove_proc_entry(PROC_NAME, NULL);
}
