QEMU_DEBUG_ENABLE := false
QEMU_DEBUG_LOGS   := false
QEMU_FLAGS := -debugcon stdio -m 12M -drive format=raw,file=build/image.iso

ifeq ($(QEMU_DEBUG_ENABLE), true)
QEMU_FLAGS += -s -S
else ifeq ($(QEMU_DEBUG_LOGS), true) 
QEMU_FLAGS += -d int
endif

CC  := clang

CFLAGS := -Ilimine/ \
          -ggdb \
          -O0 \
          -Isrc \
          -o foundryos \
          -Wall \
		  -Werror \
          -Wextra \
	      -Wno-unused-variable \
	      -Wno-unused-but-set-variable \
		  -Wno-unused-parameter \
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
           -z max-page-size=0x1000 \
           -T linker.ld

MCFLAGS := -ggdb \
	       -O0 \
           -Wall \
		   -Werror \
           -Wextra \
		   -fsanitize=address \
	       -Imeta \
		   -std=gnu11 \
		   -Wno-unused-variable \
		   -Wno-unused-but-set-variable \
		   -Wno-int-to-pointer-cast \
		   -Wno-switch \
		   -Wno-unused-parameter \
		   -o meta_generator \
	       -masm=intel

ifeq ($(CC), clang) 
	CFLAGS += -target x86_64-unknown-none
else ifeq ($(CC), gcc)
	LDFLAGS += -no-pie
endif

INCOMPLETE_SRCFILE := $(filter %.c,$(shell cd src && find -L * -type f))
SRCFILES := $(addprefix src/,$(INCOMPLETE_SRCFILE))
OBJFILES := $(addprefix build/obj/,$(INCOMPLETE_SRCFILE:.c=.o))
OUT := build/foundryos

META_INCOMPLETE_SRCFILE := $(filter %.c,$(shell cd metagen && find -L * -type f))
META_SRCFILES := $(addprefix metagen/,$(META_INCOMPLETE_SRCFILE))
META_OBJFILES := $(addprefix build/metagen/obj/,$(META_INCOMPLETE_SRCFILE:.c=.o))
META_OUT := build/meta_generator

.PHONY: all clean
# all: dirs build_iso
all: dirs build_metaprogram

dirs:
	@mkdir -p build/obj build/metagen/obj

$(OBJFILES): $(SRCFILES)
	@mkdir -p $(patsubst src/%,build/obj/%,$(dir $<))
	@$(CC) $(CFLAGS) $(patsubst build/obj/%,src/%,$(patsubst %.o,%.c,$@)) -c -o $@

$(OUT): $(OBJFILES) 
	@$(CC) $(LDFLAGS) $(OBJFILES) -o $(OUT)

$(META_OBJFILES): $(META_SRCFILES)
	@mkdir -p $(patsubst metagen/%,build/metagen/obj/%,$(dir $<))
	@$(CC) $(MCFLAGS) $(patsubst build/metagen/obj/%,metagen/%,$(patsubst %.o,%.c,$@)) -c -o $@

build_metaprogram: $(META_OBJFILES) 
	@$(CC) $(META_OBJFILES) -lasan -o $(META_OUT)

run_metaprogram:
	@build/meta_generator

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
