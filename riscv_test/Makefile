XLEN ?=64
default: all

bmarks = \
		hello \
		dispatch \
		llm		\
		#softmax

src_dir = .
incs = -I$(src_dir)/common $(addprefix -I$(src_dir)/, $(bmarks))
objs := 

RISCV_PREFIX	 ?= riscv$(XLEN)-unknown-elf-
RISCV_GCC		 ?= $(RISCV_PREFIX)gcc
RISCV_GCC_OPTS	 ?= -DPREALLOCATE=1 -mcmodel=medany -static -std=gnu99 -O2 -ffast-math -fno-builtin-printf -fno-common -fno-tree-loop-distribute-patterns -march=rv$(XLEN)imafdc -mabi=lp$(XLEN)d 
RISCV_LINK		 ?= $(RISCV_GCC) -T $(src_dir)/common/test.ld $(incs) 
# RISCV_LINK_OPTS	 ?= -static -nostartfiles -nostdlib -lm -lgcc -T $(src_dir)/common/test.ld #-Wl,-Map,$(src_dir)/common/test.map
RISCV_LINK_OPTS	 ?= -static -nostartfiles -nostdlib -lm -lgcc -T $(src_dir)/common/test.ld -Wl,-Map,$(src_dir)/common/test.map
RISCV_OBJDUMP	 ?= $(RISCV_PREFIX)objdump --disassemble-all 
RISCV_SIM 		 ?= spike --isa=rv$(XLEN)imafdcv #--pc=0x80000000
#RISCV_SIM 		 ?= spike --disable-dtb --isa=rv32imafc

define compile_template
$(1).riscv: $(wildcard $(src_dir)/$(1)/*) $(wildcard $(src_dir)/common/*)
	$$(RISCV_GCC) $$(RISCV_GCC_OPTS) $$(incs) -o $$@ $(wildcard $(src_dir)/$(1)/*.c) $(wildcard $(src_dir)/common/*.c) $(wildcard $(src_dir)/common/*.S) $$(RISCV_LINK_OPTS)
endef

$(foreach bmark, $(bmarks), $(eval $(call compile_template,$(bmark))))

bmarks_riscv_bin 	= $(addsuffix .riscv, $(bmarks))
bmarks_riscv_dump 	= $(addsuffix .riscv.dump, $(bmarks))
bmarks_riscv_out 	= $(addsuffix .riscv.out, $(bmarks))

$(bmarks_riscv_dump): %.riscv.dump: %.riscv
	$(RISCV_OBJDUMP) $< > $@

.PHONY: $(bmarks_riscv_out)
$(bmarks_riscv_out): %.riscv.out: %.riscv.dump
#  $(RISCV_SIM) $< > $@
	@$(RISCV_SIM) $< 
#| grep -v "spike: " | grep -v "spike: unimplemented" | grep -v "spike: unimplemented instruction" | grep -v "spike: unimplemented system call" > 

riscv: $(bmarks_riscv_dump)
run: $(bmarks_riscv_out)

junk += $(bmarks_riscv_bin) $(bmarks_riscv_dump) $(bmarks_riscv_out) $(bmarks_riscv_hex)

all: riscv

.PHONY:  clean 
clean:
	rm -f $(junk) $(objs)
	rm -f $(addsuffix .riscv, $(bmarks))
	rm -f $(addsuffix .riscv.dump, $(bmarks))
	rm -f $(addsuffix .riscv.out, $(bmarks))
	rm -f $(addsuffix .riscv.hex, $(bmarks))