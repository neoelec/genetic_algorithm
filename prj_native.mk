PRJ_NATIVE_MK_FILE	:= $(realpath $(lastword $(MAKEFILE_LIST)))
PRJ_NATIVE_MK_DIR	:= $(shell dirname $(PRJ_NATIVE_MK_FILE))

I_HDD00			:= $(shell \
if [ -d $(HOME)/iHDD00 ]; then \
    echo "$(HOME)/iHDD00"; \
else \
    echo "$(HOME)/eHDD00"; \
fi)
MK_RACCOON_DIR		:= $(I_HDD00)/08.PROJECT/mk-raccoon

include $(MK_RACCOON_DIR)/gcc_native.mk
