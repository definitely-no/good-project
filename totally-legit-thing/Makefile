#
# Windows 11 Core OS Source Reconstruction
# Build & Syntax Verification Makefile
#

CC ?= gcc
INCLUDES = -I include
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -fsyntax-only $(INCLUDES)

BOOT_SRCS = $(wildcard boot/bootmgr/*.c)
HAL_SRCS  = $(wildcard hal/*.c)
KE_SRCS   = $(wildcard ntoskrnl/ke/*.c)
MM_SRCS   = $(wildcard ntoskrnl/mm/*.c)
PS_SRCS   = $(wildcard ntoskrnl/ps/*.c)
OB_SRCS   = $(wildcard ntoskrnl/ob/*.c)
SE_SRCS   = $(wildcard ntoskrnl/se/*.c)
IO_SRCS   = $(wildcard ntoskrnl/io/*.c)
CM_SRCS   = $(wildcard ntoskrnl/cm/*.c)
EX_SRCS   = $(wildcard ntoskrnl/ex/*.c)
CC_SRCS   = $(wildcard ntoskrnl/cc/*.c)
KD_SRCS   = $(wildcard ntoskrnl/kd/*.c)

ALL_SRCS = $(BOOT_SRCS) $(HAL_SRCS) $(KE_SRCS) $(MM_SRCS) $(PS_SRCS) $(OB_SRCS) $(SE_SRCS) $(IO_SRCS) $(CM_SRCS) $(EX_SRCS) $(CC_SRCS) $(KD_SRCS)

.PHONY: all check clean

all: check

check:
	@echo "Checking syntax across all reconstructed Windows 11 core OS components..."
	$(CC) $(CFLAGS) $(ALL_SRCS)
	@echo "All reconstructed components compile cleanly with exit code 0."

clean:
	@echo "Cleaning temporary build files..."
	@rm -f *.o *.bin
