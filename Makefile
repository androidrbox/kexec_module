obj-m += kexec_module.o
kexec_module-objs := kexec_load.o kexec.o machine_kexec.o relocate_kernel.o cpu-reset.o cache.o

ccflags-y := -DCONFIG_KEXEC -I$(PWD)
asflags-y := -I$(PWD)
CFLAGS_machine_kexec.o := -include $(PWD)/asm/proc-fns.h
AFLAGS_relocate_kernel.o := -include $(PWD)/asm/assembler.h -include $(PWD)/linux/kexec.h
AFLAGS_cpu-reset.o := -include $(PWD)/asm/virt.h

all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-android- -C $(KERNEL_LOC) M=$(PWD) modules

clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-android- -C $(KERNEL_LOC) M=$(PWD) clean
