#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <kvm_init.h>

void kvm_reset_vcpu (struct vcpu *vcpu) {
	if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &(vcpu->sregs)) < 0) {
		perror("can not get sregs\n");
		exit(1);
	}

	vcpu->sregs.cs.selector = CODE_START;
	vcpu->sregs.cs.base = CODE_START * 16;
	vcpu->sregs.ss.selector = CODE_START;
	vcpu->sregs.ss.base = CODE_START * 16;
	vcpu->sregs.ds.selector = CODE_START;
	vcpu->sregs.ds.base = CODE_START *16;
	vcpu->sregs.es.selector = CODE_START;
	vcpu->sregs.es.base = CODE_START * 16;
	vcpu->sregs.fs.selector = CODE_START;
	vcpu->sregs.fs.base = CODE_START * 16;
	vcpu->sregs.gs.selector = CODE_START;

	if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &vcpu->sregs) < 0) {
		perror("can not set sregs");
		exit(1);
	}

	vcpu->regs.rflags = 0x0000000000000002ULL;
	vcpu->regs.rip = 0;
	vcpu->regs.rsp = 0xffffffff;
	vcpu->regs.rbp= 0;

	if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &(vcpu->regs)) < 0) {
		perror("KVM SET REGS\n");
		exit(1);
	}
}

int kvm_handle_exit(struct vcpu *vcpu)
{
	switch (vcpu->kvm_run->exit_reason) {
	case KVM_EXIT_UNKNOWN:
		printf("KVM_EXIT_UNKNOWN\n");
		return 0;
	case KVM_EXIT_DEBUG:
		printf("KVM_EXIT_DEBUG\n");
		return 0;
	case KVM_EXIT_IO:
		printf("KVM_EXIT_IO\n");
		printf("out port: %d, data: %d\n",
			vcpu->kvm_run->io.port,
			*(int *)((char *)(vcpu->kvm_run)
				+ vcpu->kvm_run->io.data_offset));
		sleep(1);
		return 0;
	case KVM_EXIT_MMIO:
		printf("KVM_EXIT_MMIO\n");
		return 0;
	case KVM_EXIT_INTR:
		printf("KVM_EXIT_INTR\n");
		return 0;
	case KVM_EXIT_SHUTDOWN:
		printf("KVM_EXIT_SHUTDOWN\n");
		return -1;
	default:
		printf("KVM PANIC\n");
		return -1;
	}
}
