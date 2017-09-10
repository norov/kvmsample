#include <linux/kvm.h>
#include <stdlib.h>

#define CODE_START 0x1000

struct vcpu {
    int vcpu_id;
    int vcpu_fd;
    pthread_t vcpu_thread;
    struct kvm_run *kvm_run;
    int kvm_run_mmap_size;
    struct kvm_regs regs;
    struct kvm_sregs sregs;
    void *(*vcpu_thread_func)(void *);
};

void kvm_reset_vcpu (struct vcpu *vcpu);
int kvm_handle_exit(struct vcpu *vcpu);
