# -*-Makefile-*-

OBJDIRS += $(USER_OBJDIR)/idle

USER_IDLE_SRC += $(USER_DIR)/idle/idle.c
USER_IDLE_OBJ := $(patsubst %.c, $(OBJDIR)/%.o, $(USER_IDLE_SRC))
USER_IDLE_OBJ := $(patsubst %.S, $(OBJDIR)/%.o, $(USER_IDLE_OBJ))

KERN_BINFILES += $(USER_OBJDIR)/idle/idle
USER_LIB_SRC += $(USER_DIR)/lib/divdi3.c

idle: $(USER_OBJDIR)/idle/idle

# Hardcode the correct libgcc.a path
LIBGCC := /usr/lib/gcc/x86_64-linux-gnu/11/libgcc.a

$(USER_OBJDIR)/idle/idle: $(USER_LIB_OBJ) $(USER_IDLE_OBJ)
	@echo + LINK [USER/idle] $@
	@mkdir -p $(dir $@)
	/usr/bin/gcc -m32 -nostdlib -static -o $@ $(USER_LIB_OBJ) $(USER_IDLE_OBJ) /usr/lib/gcc/x86_64-linux-gnu/11/libgcc.a
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/idle/%.o: $(USER_DIR)/idle/%.c
	@echo + cc[USER/idle] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<

$(USER_OBJDIR)/idle/%.o: $(USER_DIR)/idle/%.S
	@echo + as[USER/idle] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<
