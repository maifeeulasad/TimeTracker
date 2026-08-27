# ====================================================================
# File: Makefile
# TimeTracker — Persistent time tracking for Ubuntu
# Author: Maifee Ul Asad
# License: MIT
#
# Usage:
#   make              — build (Qt 5 by default)
#   make QT_VERSION=6 — build against Qt 6
#   make clean        — remove build artefacts
#   make install      — install to /usr/local (needs sudo)
# ====================================================================

# ---- toolchain ----
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
MOCFLAGS :=

# ---- Qt version (5 or 6) ----
QT_VERSION ?= 5

ifeq ($(QT_VERSION),6)
  QT_MODS    := Qt6Widgets Qt6Sql Qt6Core
  MOC_MOD    := Qt6Core
else
  QT_MODS    := Qt5Widgets Qt5Sql Qt5Core
  MOC_MOD    := Qt5Core
endif

CXXFLAGS += -fPIC $(shell pkg-config --cflags $(QT_MODS))
LDFLAGS  := $(shell pkg-config --libs   $(QT_MODS))

# ---- MOC ----
MOC := $(shell pkg-config --variable=host_bins $(MOC_MOD) 2>/dev/null)/moc
ifeq ($(MOC),/moc)
  MOC := moc
endif

# ---- directories ----
SRCDIR   := src
BUILDDIR := build

# ---- sources ----
SRCS      := $(wildcard $(SRCDIR)/*.cpp)
MOC_HDRS  := $(wildcard $(SRCDIR)/*.h)
MOC_SRCS  := $(patsubst $(SRCDIR)/%.h,$(BUILDDIR)/moc_%.cpp,$(MOC_HDRS))
OBJS      := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS)) \
             $(patsubst $(BUILDDIR)/%.cpp,$(BUILDDIR)/%.o,$(MOC_SRCS))

TARGET := timetracker

# ====================================================================
.PHONY: all clean rebuild install uninstall

all: $(BUILDDIR) $(TARGET)
	@echo "OK  Build complete: ./$(TARGET)"

$(BUILDDIR):
	mkdir -p $@

# ---- link ----
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# ---- compile src/*.cpp ----
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c $< -o $@

# ---- compile moc_*.cpp ----
$(BUILDDIR)/moc_%.o: $(BUILDDIR)/moc_%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c $< -o $@

# ---- generate moc ----
$(BUILDDIR)/moc_%.cpp: $(SRCDIR)/%.h | $(BUILDDIR)
	$(MOC) $(MOCFLAGS) $< -o $@

# ---- install / uninstall ----
PREFIX ?= /usr/local

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	install -Dm644 timetracker.desktop $(DESTDIR)$(PREFIX)/share/applications/timetracker.desktop

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -f $(DESTDIR)$(PREFIX)/share/applications/timetracker.desktop

# ---- housekeeping ----
clean:
	rm -rf $(BUILDDIR) $(TARGET)

rebuild: clean all
