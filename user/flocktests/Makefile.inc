# -*-Makefile-*-

OBJDIRS		+= $(USER_OBJDIR)/flocktests

USER_FLOCKTESTS_SRC	+= $(USER_DIR)/flocktests/flocktests.c
USER_FLOCKTESTS_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKTESTS_SRC))
USER_FLOCKTESTS_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKTESTS_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/flocktests/flocktests

USER_FLOCKSTALL_SRC	+= $(USER_DIR)/flocktests/flockstall.c
USER_FLOCKSTALL_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKSTALL_SRC))
USER_FLOCKSTALL_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKSTALL_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/flocktests/flockstall

flocktests: $(USER_OBJDIR)/flocktests/flocktests $(USER_OBJDIR)/flocktests/flockstall

$(USER_OBJDIR)/flocktests/flocktests: $(USER_LIB_OBJ) $(USER_FLOCKTESTS_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKTESTS_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktests/flockstall: $(USER_LIB_OBJ) $(USER_FLOCKSTALL_OBJ)
	@echo + ld[USER/flocktests] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKSTALL_OBJ) $(GCC_LIBS)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktests/%.o: $(USER_DIR)/flocktests/%.c
	@echo + cc[USER/flocktests] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<

$(USER_OBJDIR)/flocktests/%.o: $(USER_DIR)/flocktests/%.S
	@echo + as[USER/flocktests] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<
