QEMU_DEBUG_ENABLE := false
QEMU_DEBUG_LOGS   := false
QEMU_FLAGS := -debugcon stdio -m 12M -drive format=raw,file=build/image.iso

ifeq ($(QEMU_DEBUG_ENABLE), true)
QEMU_FLAGS += -s -S
else ifeq ($(QEMU_DEBUG_LOGS), true) 
QEMU_FLAGS += -d int
endif

CC := gcc
CFLAGS := -Ilimine/ \
          -ggdb \
          -O0 \
          -Isrc \
          -o foundryos \
          -Wall \
          -Wextra \
          -Wno-error=unused-variable \
          -std=gnu11 \
          -ffreestanding \
          -fno-stack-protector \
          -fno-stack-check \
          -fno-PIC \
          -m64 \
          -masm=intel \
          -march=x86-64 \
          -mno-80387 \
          -mno-mmx \
          -mno-sse \
          -mno-sse2 \
          -mno-red-zone \
          -DLIMINE_API_REVISION=3 \
          -mcmodel=kernel

LDFLAGS := -Wl,-m,elf_x86_64 \
           -Wl,--build-id=none \
           -nostdlib \
           -no-pie \
           -z max-page-size=0x1000 \
           -T linker.ld


INCOMPLETE_SRCFILE := $(filter %.c,$(shell cd src && find -L * -type f))
SRCFILES := $(addprefix src/,$(INCOMPLETE_SRCFILE))
OBJFILES := $(addprefix build/obj/,$(INCOMPLETE_SRCFILE:.c=.o))
OUT := build/foundryos

.PHONY: all clean
all: dirs build_iso

dirs:
	@mkdir -p build/obj

$(OBJFILES): $(SRCFILES)
	@mkdir -p $(patsubst src/%,build/obj/%,$(dir $<))
	@$(CC) $(CFLAGS) $(LDFLAGS) $(patsubst build/obj/%,src/%,$(patsubst %.o,%.c,$@)) -c -o $@

$(OUT): $(OBJFILES) 
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJFILES) -o $(OUT)

build_iso: $(OUT)
	@mkdir -p build/iso_root

	@mkdir -p build/iso_root/boot
	@cp -v build/foundryos build/iso_root/boot/
	@mkdir -p build/iso_root/boot/limine

	@cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin \
		limine/limine-uefi-cd.bin build/iso_root/boot/limine/

	@mkdir -p build/iso_root/EFI/BOOT
	@cp -v limine/BOOTX64.EFI build/iso_root/EFI/BOOT/
	@cp -v limine/BOOTIA32.EFI build/iso_root/EFI/BOOT/

	@xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		build/iso_root -o build/image.iso

	@./limine/limine bios-install build/image.iso

run:
	@qemu-system-x86_64 $(QEMU_FLAGS) 

clean:
	@rm -rf build
