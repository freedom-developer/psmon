KERNEL_DIR := kernel
USERSPACE_DIR := userspace

.PHONY: all kernel userspace compile-commands test clean

all: kernel userspace

kernel:
	$(MAKE) -C $(KERNEL_DIR)

userspace:
	$(MAKE) -C $(USERSPACE_DIR)

compile-commands:
	./scripts/gen_compile_commands.sh

test: userspace
	./tests/userspace/smoke.sh

clean:
	$(MAKE) -C $(KERNEL_DIR) clean
	$(MAKE) -C $(USERSPACE_DIR) clean
