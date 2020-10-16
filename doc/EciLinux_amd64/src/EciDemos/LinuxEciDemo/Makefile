BUILD_SPEC  ?= release
CC       	?= gcc
CFLAGS   	+= 

LDFLAGS     += -leci002DriverLinux
LDFLAGS     += -leci003DriverLinux
LDFLAGS     += -leci005DriverLinux
LDFLAGS     += -leci101DriverLinux
LDFLAGS     += -leci102DriverLinux
LDFLAGS     += -leci105DriverLinux
LDFLAGS     += -leci109DriverLinux
LDFLAGS     += -leci10ADriverLinux
LDFLAGS     += -leci10EDriverLinux
LDFLAGS     += -leci111DriverLinux
LDFLAGS     += -leci113DriverLinux-usb-1.0
LDFLAGS     += -leci116DriverLinux-usb-1.0
LDFLAGS     += -lusb
LDFLAGS     += -lusb-1.0
LDFLAGS     += -L../../../bin/$(BUILD_SPEC)


OBJDIR   	:= ../../../obj/$(BUILD_SPEC)/LinuxEciDemo/
OUTDIR   	:= ../../../bin/$(BUILD_SPEC)
OUTPUT   	:= $(OUTDIR)/LinuxEciDemo

LOCAL_SRC_DIRS 	:= .
LOCAL_SRC_DIRS 	+= ..

LOCAL_INC_DIRS 	:= ..
LOCAL_INC_DIRS 	+= ../../../inc
LOCAL_INC_DIRS 	+= ../../EciDrivers/common/ECI
LOCAL_INC_DIRS 	+= ../../EciDrivers/OsDrvLinux


SRC     := $(foreach sdir,$(LOCAL_SRC_DIRS),$(wildcard $(sdir)/*.c))
INC	    := $(foreach idir,$(LOCAL_INC_DIRS),-I$(idir))
OBJ     := $(addprefix $(OBJDIR),$(notdir $(patsubst %.c,%.o,$(SRC))))

vpath %.c $(LOCAL_SRC_DIRS)

.PHONY: all clean dirtree

all: $(OUTPUT)
   
$(OUTPUT): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%o:    %c
	$(CC) $(CFLAGS) $< -o $@ $(INC)

$(OBJ): | dirtree

dirtree:
	mkdir -p $(OBJDIR)
	mkdir -p $(OUTDIR)

clean:
	rm -f $(OBJDIR)*.o *~ $(OUTPUT)

