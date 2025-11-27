CC       ?= gcc
CFLAGS   ?= -Wall -Wextra -pedantic -std=c17 -O2
LDFLAGS  ?=
TARGET    = macroscope
WIN_TARGET = macroscope.exe
SOURCES   = main.c eth_frame.c tables.c

# Cross-compilation for Windows (MinGW)
MINGW_CC ?= x86_64-w64-mingw32-gcc

DOXYGEN  ?= doxygen
DOXYFILE ?= Doxyfile
DOC_HTML  = docs/html/index.html

FRAME    ?= frames.txt
HEADERS   = eth_frame.h tables.h
ZIP_NAME  = macroscope-release.zip

.PHONY: all run docs clean docs-clean windows win-clean all-platforms zip

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

# Windows build using MinGW cross-compiler
windows: $(WIN_TARGET)

$(WIN_TARGET): $(SOURCES)
	$(MINGW_CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)
	@echo "Windows executable built: $(WIN_TARGET)"
	@echo "Test on Windows with: $(WIN_TARGET) $(FRAME)"

# Build for both Linux and Windows
all-platforms: $(TARGET) $(WIN_TARGET)
	@echo "Built for Linux: $(TARGET)"
	@echo "Built for Windows: $(WIN_TARGET)"

run: $(TARGET)
	./$(TARGET) $(FRAME)

docs: $(DOC_HTML)

$(DOC_HTML): $(SOURCES) $(DOXYFILE) README.md random_frame.txt
	$(DOXYGEN) $(DOXYFILE)
	@touch $(DOC_HTML)

clean:
	rm -f $(TARGET) $(WIN_TARGET) $(ZIP_NAME)

win-clean:
	rm -f $(WIN_TARGET)

# Create release zip with sources and executables
zip: all-platforms
	@echo "Creating release archive..."
	zip -9 $(ZIP_NAME) \
		$(SOURCES) $(HEADERS) \
		$(TARGET) $(WIN_TARGET) \
		$(FRAME) README.md Makefile
	@echo "Release archive created: $(ZIP_NAME)"
	@unzip -l $(ZIP_NAME)

docs-clean:
	rm -rf docs
