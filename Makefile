TARGET = markdown_reader
OBJS = main.o \
       core/theme_engine.o \
       core/md_parser.o \
       core/scroll_physics.o \
       core/app_state.o \
       ui/font.o \
       ui/graphics.o \
       ui/screens.o \
       platform/psp_fs.o \
       platform/psp_input.o \
       platform/psp_power.o

CFLAGS = -O2 -G0 -Wall -Wextra -I.
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspgu -lpsppower -lpsprtc -lpsputility -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP Go Markdown Reader
PSP_EBOOT_SFO = PARAM.SFO

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
