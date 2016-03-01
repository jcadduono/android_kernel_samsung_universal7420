LIBFDT_DIR=../dtc/libfdt

include $(LIBFDT_DIR)/Makefile.libfdt

OBJ_FILES := dtbtool.o $(addprefix $(LIBFDT_DIR)/,$(LIBFDT_OBJS))
CFLAGS := -I$(LIBFDT_DIR) -Os -fomit-frame-pointer -Wall

all: dtbTool

dtbTool: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^
	strip --strip-all $@

clean:
	rm -f $(OBJ_FILES) dtbTool

