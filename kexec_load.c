#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/kexec.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/slab.h>

u32 __boot_cpu_mode[] = { 0xe11, 0xe11 };
int panic_on_oops = 0;

static void **sys_call_table = NULL;
static asmlinkage long (*real_sys_reboot)(int magic1, int magic2, unsigned int cmd, void __user *arg);
extern asmlinkage long sys_kexec_load(unsigned long entry, unsigned long nr_segments,
                                      struct kexec_segment __user *segments, unsigned long flags);

void smp_send_stop(void)
{
	((void (*)(void))kallsyms_lookup_name("smp_send_stop"))();
}

void kernel_restart_prepare(char *cmd)
{
	((void (*)(char *))kallsyms_lookup_name("kernel_restart_prepare"))(cmd);
}

void machine_shutdown(void)
{
	((void (*)(void))kallsyms_lookup_name("machine_shutdown"))();
}

void setup_mm_for_reboot(void)
{
	((void (*)(void))kallsyms_lookup_name("setup_mm_for_reboot"))();
}

int insert_resource(struct resource *parent, struct resource *new)
{
	return ((int (*)(struct resource *, struct resource *))kallsyms_lookup_name("insert_resource"))(parent, new);
}

void log_buf_kexec_setup(void) { }

asmlinkage long sys_reboot(int magic1, int magic2, unsigned int cmd, void __user *arg)
{
	return real_sys_reboot(magic1, magic2, cmd, arg);
}

static int __init kexec_init(void)
{
	void **compat_sys_call_table = NULL;
	void *sys_io_setup = NULL;
	void *sys_io_destroy = NULL;
	int i;

	printk(KERN_INFO "kexec: module loading...\n");

	printk(KERN_INFO "kexec: looking up symbols...\n");
	compat_sys_call_table = (void **)kallsyms_lookup_name("compat_sys_call_table");
	sys_io_setup = (void *)kallsyms_lookup_name("sys_io_setup");
	sys_io_destroy = (void *)kallsyms_lookup_name("sys_io_destroy");

	printk(KERN_INFO "kexec: searching for sys_call_table...\n");
	for (i = 0; i < 0xFFFFFF; i++)
	{
		if (compat_sys_call_table[i] == sys_io_setup && compat_sys_call_table[i+1] == sys_io_destroy)
		{
			sys_call_table = (void **)&compat_sys_call_table[i];
		}
	}

	if (sys_call_table != NULL)
	{
		printk(KERN_INFO "kexec: found the sys_call_table!\n");
		sys_call_table[__NR_kexec_load] = (void *)sys_kexec_load;
		real_sys_reboot = (void *)&sys_call_table[__NR_reboot];
		sys_call_table[__NR_reboot] = (void *)sys_reboot;

		return 0;
	}
	else
	{
		printk(KERN_INFO "kexec: sys_call_table not found!\n");

		return -1;
	}
}

static void __exit kexec_exit(void)
{
	printk(KERN_ALERT "kexec: module unloading...\n");
	sys_call_table[__NR_kexec_load] = (void *)kallsyms_lookup_name("sys_ni_syscall");
	sys_call_table[__NR_reboot] = (void *)real_sys_reboot;
}

module_init(kexec_init);
module_exit(kexec_exit);

MODULE_AUTHOR("rbox");
MODULE_DESCRIPTION("Add kexec syscall");
MODULE_LICENSE("GPL");
