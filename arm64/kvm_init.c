#include <stdio.h>

#include <kvm_init.h>

void kvm_reset_vcpu (struct vcpu *vcpu) {
}

int kvm_handle_exit(struct vcpu *vcpu)
{
	printf("kvm exit reaso: %d\n", vcpu->kvm_run->exit_reason);
	return 0;
}
