# usbd_mini.irx built from the ps2sdk USBD sources from before the "USBD feature update" (ps2sdk
# b1f7ff96), which download_usbd.sh fetches into ps2sdk/. It is built the way ps2sdk built it then:
# the sources of iop/usb/usbd with -DMINI_DRIVER (iop/usb/usbd_mini/Makefile of that tree). See CONTRIBUTING.md.
IOP_BIN = usbd_mini.irx
IOP_OBJS = hcd.o hub.o interface.o mem.o usbd.o usbio.o driver.o metrics.o imports.o exports.o

USBD_DIR = ps2sdk/iop/usb/usbd/
IOP_CFLAGS += -DMINI_DRIVER
IOP_INCS += -I$(USBD_DIR)src/ -I$(USBD_DIR)include/

include $(PS2SDK)/Defs.make
include ../../Rules.bin.make
IOP_SRC_DIR = $(USBD_DIR)src/
include $(PS2SDK)/samples/Makefile.iopglobal
