#
# Windows 11 Core OS Source Reconstruction
# Build & Syntax Verification Makefile
#

CC ?= gcc
INCLUDES = -I include
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -fsyntax-only $(INCLUDES)

BOOT_SRCS = $(wildcard boot/*/*.c)
HAL_SRCS  = $(wildcard hal/*.c)
KE_SRCS   = $(wildcard ntoskrnl/ke/*.c ntoskrnl/ke/*/*.c)
MM_SRCS   = $(wildcard ntoskrnl/mm/*.c)
PS_SRCS   = $(wildcard ntoskrnl/ps/*.c)
OB_SRCS   = $(wildcard ntoskrnl/ob/*.c)
SE_SRCS   = $(wildcard ntoskrnl/se/*.c)
IO_SRCS   = $(wildcard ntoskrnl/io/*.c ntoskrnl/io/*/*.c)
CM_SRCS   = $(wildcard ntoskrnl/cm/*.c ntoskrnl/cm/*/*.c)
EX_SRCS   = $(wildcard ntoskrnl/ex/*.c)
CC_SRCS   = $(wildcard ntoskrnl/cc/*.c)
KD_SRCS   = $(wildcard ntoskrnl/kd/*.c)
PO_SRCS   = $(wildcard ntoskrnl/po/*.c)
ALPC_SRCS = $(wildcard ntoskrnl/alpc/*.c)
ETW_SRCS  = $(wildcard ntoskrnl/etw/*.c)
WHEA_SRCS = $(wildcard ntoskrnl/whea/*.c)
CI_SRCS   = $(wildcard security/ci/*.c)
DRV_SRCS  = $(wildcard drivers/*/*/*.c)
USER_SRCS = $(wildcard usermode/*/*.c)

ALL_SRCS = $(BOOT_SRCS) $(HAL_SRCS) $(KE_SRCS) $(MM_SRCS) $(PS_SRCS) $(OB_SRCS) $(SE_SRCS) $(IO_SRCS) $(CM_SRCS) $(EX_SRCS) $(CC_SRCS) $(KD_SRCS) $(PO_SRCS) $(ALPC_SRCS) $(ETW_SRCS) $(WHEA_SRCS) $(CI_SRCS) $(DRV_SRCS) $(USER_SRCS)

.PHONY: all check abi_test behavioral_test test clean

all: check abi_test behavioral_test

check:
	@echo "Checking syntax across all reconstructed Windows 11 core OS components..."
	$(CC) $(CFLAGS) $(ALL_SRCS)
	@echo "All reconstructed components compile cleanly with exit code 0."

abi_test:
	@echo "Executing Phase 1 binary & ABI structure offset verification test..."
	@$(CC) $(INCLUDES) tools/abi_verify.c -o /tmp/abi_verify && /tmp/abi_verify

behavioral_test:
	@echo "Executing Phase 2 Level 2 behavioral invariant verification test suite..."
	@$(CC) $(INCLUDES) tools/behavioral_test.c -o /tmp/behavioral_test && /tmp/behavioral_test

test: all

clean:
	@echo "Cleaning temporary build files..."
	@rm -f *.o *.bin
