CC       ?= gcc
CFLAGS   ?= -Wall -Wextra -pedantic -std=c17 -O2
LDFLAGS  ?=
TARGET    = macroscope
SOURCES   = ethernet_frame.c

DOXYGEN  ?= doxygen
DOXYFILE ?= Doxyfile
DOC_HTML  = docs/html/index.html

FRAME    ?= random_frame.txt

.PHONY: all run docs clean docs-clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) $(FRAME)

docs: $(DOC_HTML)

$(DOC_HTML): $(SOURCES) $(DOXYFILE) README.md random_frame.txt
	$(DOXYGEN) $(DOXYFILE)
	@touch $(DOC_HTML)

clean:
	rm -f $(TARGET)

docs-clean:
	rm -rf docs
