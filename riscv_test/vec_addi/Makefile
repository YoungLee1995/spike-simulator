CROSS = riscv64-unknown-elf
GCC = $(CROSS)-gcc
LD = $(CROSS)-ld
OBJDUMP = $(CROSS)-objdump
OBJCOPY = $(CROSS)-objcopy
READELF = $(CROSS)-readelf
SPIKE = spike --isa=RV64GCV

CFLAGS = -march=rv64gcv -mabi=lp64d

all: vec_add.elf

vec_add.elf: crt.o vec_add.o
	$(LD) -T test.ld -o $@ $^

crt.o: crt.S
	$(GCC) $(CFLAGS) -c $< -o $@

vec_add.o: vec_add.S
	$(GCC) $(CFLAGS) -c $< -o $@

run: vec_add.elf
	$(SPIKE) -l $<

run-pk:
	spike --isa=RV64GCV /opt/riscv/riscv64-unknown-elf/bin/pk vec_add.elf

dump: vec_add.elf
	$(OBJDUMP) -d $< | less

hexdump: vec_add.elf
	$(OBJDUMP) -s -j .data $<

clean:
	rm -f *.o *.elf
