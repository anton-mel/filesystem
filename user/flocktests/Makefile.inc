# -*-Makefile-*-

OBJDIRS		+= $(USER_OBJDIR)/flocktests

# make sure the compiler can find flocktests_common.h
USER_CFLAGS  += -I$(USER_DIR)/flocktests

# flocktests main test driver
USER_FLOCKTESTS_SRC += $(USER_DIR)/flocktests/flocktests.c
USER_FLOCKTESTS_OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKTESTS_SRC))
USER_FLOCKTESTS_OBJ := $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKTESTS_OBJ))
KERN_BINFILES += $(USER_OBJDIR)/flocktests/flocktests

# reader_exp_block
USER_READER_EXP_BLOCK_SRC += $(USER_DIR)/flocktests/helpers/reader_exp_block.c
USER_READER_EXP_BLOCK_OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(USER_READER_EXP_BLOCK_SRC))
USER_READER_EXP_BLOCK_OBJ := $(patsubst %.S, $(OBJDIR)/%.o, $(USER_READER_EXP_BLOCK_OBJ))
KERN_BINFILES += $(USER_OBJDIR)/flocktests/reader_exp_block

# reader_exp_no_block
USER_READER_EXP_NB_SRC += $(USER_DIR)/flocktests/helpers/reader_exp_no_block.c
USER_READER_EXP_NB_OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(USER_READER_EXP_NB_SRC))
USER_READER_EXP_NB_OBJ := $(patsubst %.S, $(OBJDIR)/%.o, $(USER_READER_EXP_NB_OBJ))
KERN_BINFILES += $(USER_OBJDIR)/flocktests/reader_exp_no_block

# writer_does_block
USER_WRITER_DOES_BLOCK_SRC += $(USER_DIR)/flocktests/helpers/writer_does_block.c
USER_WRITER_DOES_BLOCK_OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(USER_WRITER_DOES_BLOCK_SRC))
USER_WRITER_DOES_BLOCK_OBJ := $(patsubst %.S, $(OBJDIR)/%.o, $(USER_WRITER_DOES_BLOCK_OBJ))
KERN_BINFILES += $(USER_OBJDIR)/flocktests/writer_does_block

# writer_exp_block
USER_WRITER_EXP_BLOCK_SRC += $(USER_DIR)/flocktests/helpers/writer_exp_block.c
USER_WRITER_EXP_BLOCK_OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(USER_WRITER_EXP_BLOCK_SRC))
USER_WRITER_EXP_BLOCK_OBJ := $(patsubst %.S, $(OBJDIR)/%.o, $(USER_WRITER_EXP_BLOCK_OBJ))
KERN_BINFILES += $(USER_OBJDIR)/flocktests/writer_exp_block

# writer_exp_no_block
USER_WRITER_EXP_NB_SRC += $(USER_DIR)/flocktests/helpers/writer_exp_no_block.c
USER_WRITER_EXP_NB_OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(USER_WRITER_EXP_NB_SRC))
USER_WRITER_EXP_NB_OBJ := $(patsubst %.S, $(OBJDIR)/%.o, $(USER_WRITER_EXP_NB_OBJ))
KERN_BINFILES += $(USER_OBJDIR)/flocktests/writer_exp_no_block

# Build rules for each binary
flocktests: \
	$(USER_OBJDIR)/flocktests/flocktests \
	$(USER_OBJDIR)/flocktests/reader_exp_block \
	$(USER_OBJDIR)/flocktests/reader_exp_no_block \
	$(USER_OBJDIR)/flocktests/writer_does_block \
	$(USER_OBJDIR)/flocktests/writer_exp_block \
	$(USER_OBJDIR)/flocktests/writer_exp_no_block

$(USER_OBJDIR)/flocktests/flocktests: $(USER_LIB_OBJ) $(USER_FLOCKTESTS_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKTESTS_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktests/reader_exp_block: $(USER_LIB_OBJ) $(USER_READER_EXP_BLOCK_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_READER_EXP_BLOCK_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktests/reader_exp_no_block: $(USER_LIB_OBJ) $(USER_READER_EXP_NB_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_READER_EXP_NB_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktests/writer_does_block: $(USER_LIB_OBJ) $(USER_WRITER_DOES_BLOCK_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_WRITER_DOES_BLOCK_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktests/writer_exp_block: $(USER_LIB_OBJ) $(USER_WRITER_EXP_BLOCK_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_WRITER_EXP_BLOCK_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktests/writer_exp_no_block: $(USER_LIB_OBJ) $(USER_WRITER_EXP_NB_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_WRITER_EXP_NB_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

# Generic compile rules
$(USER_OBJDIR)/flocktests/%.o: $(USER_DIR)/flocktests/%.c
	@echo + cc[USER/flocktests] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<

$(USER_OBJDIR)/flocktests/%.o: $(USER_DIR)/flocktests/%.S
	@echo + as[USER/flocktests] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<
