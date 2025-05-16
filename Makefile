OUTPUT = nixmon
INSTALL_PATH = /usr/local/bin

SOURCE_FILES = main.c gen.c cpu.c mem.c storage.c i915.c amdgpu.c util/util.c

all:
	cc -Wall $(SOURCE_FILES) -o $(OUTPUT)

install:
	cp -f $(OUTPUT) $(INSTALL_PATH)

clean:
	rm -f $(OUTPUT)