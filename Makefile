# Makefile created with assistance from ChatGPT
# =========================
# Simple OpenGL Makefile
# Linux + Windows (MinGW/MSYS2)
# Uses: GLFW + GLEW
# =========================

APP       := app
SRC_DIR   := src
BUILD_DIR := build

SRCS      := $(wildcard $(SRC_DIR)/*.cpp)
OBJS      := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

CPP       := g++
CPPFLAGS  := -std=c++17 -Wall -Wextra -Wpedantic

# --- Platform detection ---
ifeq ($(OS),Windows_NT)
  PLATFORM := windows
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
  else
    PLATFORM := unknown
  endif
endif

# --- Libraries ---
LDLIBS := ./libs/libportaudio.a -lrt -lm -lasound -lfftw3
LDFLAGS :=

ifeq ($(PLATFORM),linux)
  # Try pkg-config
  PKG_CFLAGS := $(shell pkg-config --cflags  2>/dev/null)
  PKG_LIBS   := $(shell pkg-config --libs  2>/dev/null)
  LDLIBS     += -static-libstdc++ -static-libgcc

  ifneq ($(strip $(PKG_LIBS)),)
    CPPFLAGS += $(PKG_CFLAGS)
    LDLIBS   += $(PKG_LIBS)
  else
    LDLIBS   += -lm
  endif

else ifeq ($(PLATFORM),windows)
  CPPFLAGS +=
  LDLIBS   += -lportaudio -lwinmm -lgdi32 -luser32 -lshell32 -lkernel32

else
  $(warning Unknown platform; using Linux defaults.)
  LDLIBS   += -lm -lfftw3
endif

# --- Targets ---
.PHONY: all clean run

all: clean $(APP)

$(APP): $(OBJS)
	@echo "Linking $(APP)..."
	$(CPP) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CPP) $(CPPFLAGS) -c $< -o $@

run: $(APP)
	./$(APP)

clean:
	rm -rf $(BUILD_DIR) $(APP)
