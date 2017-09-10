/*
 * KVM API Sample.
 * author: Xu He Jie xuhj@cn.ibm.com
 */
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/kvm.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <kvm_init.h>

#define KVM_DEVICE "/dev/kvm"
#define RAM_SIZE 512000000

#define BINARY_FILE "test.bin"

struct kvm {
   int dev_fd;
   int vm_fd;
   __u64 ram_start;
   __u64 ram_size;
   struct kvm_userspace_memory_region mem;

   struct vcpu *vcpus;
   int vcpu_number;
};

void *kvm_cpu_thread(void *data) {
	struct kvm *kvm = (struct kvm *)data;
	int ret = 0;
	kvm_reset_vcpu(kvm->vcpus);

	while (1) {
		printf("KVM start run\n");
		ret = ioctl(kvm->vcpus->vcpu_fd, KVM_RUN, 0);

		if (ret < 0) {
			fprintf(stderr, "KVM_RUN failed\n");
			exit(1);
		}

		if (kvm_handle_exit(kvm->vcpus))
			return 0;
	}
}

void load_binary(struct kvm *kvm) {
    int fd = open(BINARY_FILE, O_RDONLY);

    if (fd < 0) {
        fprintf(stderr, "can not open binary file\n");
        exit(1);
    }

    int ret = 0;
    char *p = (char *)kvm->ram_start;

    while(1) {
        ret = read(fd, p, 4096);
        if (ret <= 0) {
            break;
        }
        printf("read size: %d", ret);
        p += ret;
    }
}

struct kvm *kvm_init(void) {
    struct kvm *kvm = malloc(sizeof(struct kvm));
    kvm->dev_fd = open(KVM_DEVICE, O_RDWR);

    if (kvm->dev_fd < 0) {
        perror("open kvm device fault: ");
        return NULL;
    }

    return kvm;
}

void kvm_clean(struct kvm *kvm) {
    assert (kvm != NULL);
    close(kvm->dev_fd);
    free(kvm);
}

int kvm_create_vm(struct kvm *kvm, int ram_size) {
    int ret = 0;
    kvm->vm_fd = ioctl(kvm->dev_fd, KVM_CREATE_VM, 0);

    if (kvm->vm_fd < 0) {
        perror("can not create vm");
        return -1;
    }

    kvm->ram_size = ram_size;
    kvm->ram_start =  (__u64)mmap(NULL, kvm->ram_size,
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                -1, 0);

    if ((void *)kvm->ram_start == MAP_FAILED) {
        perror("can not mmap ram");
        return -1;
    }

    kvm->mem.slot = 0;
    kvm->mem.guest_phys_addr = 0;
    kvm->mem.memory_size = kvm->ram_size;
    kvm->mem.userspace_addr = kvm->ram_start;

    ret = ioctl(kvm->vm_fd, KVM_SET_USER_MEMORY_REGION, &(kvm->mem));

    if (ret < 0) {
        perror("can not set user memory region");
        return ret;
    }

    return ret;
}

void kvm_clean_vm(struct kvm *kvm) {
    close(kvm->vm_fd);
    munmap((void *)kvm->ram_start, kvm->ram_size);
}

struct vcpu *kvm_init_vcpu(struct kvm *kvm, int vcpu_id, void *(*fn)(void *)) {
    struct vcpu *vcpu = malloc(sizeof(struct vcpu));
    vcpu->vcpu_id = 0;
    vcpu->vcpu_fd = ioctl(kvm->vm_fd, KVM_CREATE_VCPU, vcpu->vcpu_id);

    if (vcpu->vcpu_fd < 0) {
        perror("can not create vcpu");
        return NULL;
    }

    vcpu->kvm_run_mmap_size = ioctl(kvm->dev_fd, KVM_GET_VCPU_MMAP_SIZE, 0);

    if (vcpu->kvm_run_mmap_size < 0) {
        perror("can not get vcpu mmsize");
        return NULL;
    }

    printf("%d\n", vcpu->kvm_run_mmap_size);
    vcpu->kvm_run = mmap(NULL, vcpu->kvm_run_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu->vcpu_fd, 0);

    if (vcpu->kvm_run == MAP_FAILED) {
        perror("can not mmap kvm_run");
        return NULL;
    }

    vcpu->vcpu_thread_func = fn;
    return vcpu;
}

void kvm_clean_vcpu(struct vcpu *vcpu) {
    munmap(vcpu->kvm_run, vcpu->kvm_run_mmap_size);
    close(vcpu->vcpu_fd);
}

void kvm_run_vm(struct kvm *kvm) {
    int i = 0;

    for (i = 0; i < kvm->vcpu_number; i++) {
        if (pthread_create(&(kvm->vcpus->vcpu_thread), (const pthread_attr_t *)NULL, kvm->vcpus[i].vcpu_thread_func, kvm) != 0) {
            perror("can not create kvm thread");
            exit(1);
        }
    }

    pthread_join(kvm->vcpus->vcpu_thread, NULL);
}

int main(int argc, char **argv) {
    int ret = 0;
    struct kvm *kvm = kvm_init();

    if (kvm == NULL) {
        fprintf(stderr, "kvm init fauilt\n");
        return -1;
    }

    if (kvm_create_vm(kvm, RAM_SIZE) < 0) {
        fprintf(stderr, "create vm fault\n");
        return -1;
    }

    load_binary(kvm);

    // only support one vcpu now
    kvm->vcpu_number = 1;
    kvm->vcpus = kvm_init_vcpu(kvm, 0, kvm_cpu_thread);

    kvm_run_vm(kvm);

    kvm_clean_vm(kvm);
    kvm_clean_vcpu(kvm->vcpus);
    kvm_clean(kvm);
}
