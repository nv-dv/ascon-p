# Targets
TARGET_X86 := main_x86
TARGET_X64 := main_x64
TARGET_ARMV7 := main_armv7
TARGET_AARCH64 := main_aarch64

# Build directories
BUILD_X86 := bin_x86
BUILD_X64 := bin_x64
BUILD_ARMV7 := bin_armv7
BUILD_AARCH64 := bin_aarch64

SRC_DIR := src
SRCS := $(shell find $(SRC_DIR) -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.s' \))

# Include flags
INC_FLAGS := -I$(SRC_DIR)


# M4RI library paths
M4RI_X64 := /usr/local/lib/m4ri/x64
M4RI_X86 := /usr/local/lib/m4ri/x86
M4RI_ARMV7 := /usr/local/lib/m4ri/armv7
M4RI_AARCH64 := /usr/local/lib/m4ri/arm64

# Compiler flags
CPPFLAGS_INTEL :=$(INC_FLAGS) -O3 -mavx2 -fno-builtin -masm=intel
CPPFLAGS_ARM := $(INC_FLAGS) -O3 -fno-builtin 


# Link flags
LDFLAGS_X64 := -L$(M4RI_X64) -lm4ri -Wl,-rpath,$(M4RI_X64) 
LDFLAGS_X86 := -L$(M4RI_X86) -lm4ri -Wl,-rpath,$(M4RI_X86)
LDFLAGS_ARMV7 := -L$(M4RI_ARMV7) -lm4ri -Wl,-rpath,$(M4RI_ARMV7)
LDFLAGS_AARCH64 := -L$(M4RI_AARCH64) -lm4ri -Wl,-rpath,$(M4RI_AARCH64)


.PHONY: all
all: x86 x64 armv7 aarch64

# -------------------------------
# x86 build
# -------------------------------
x86: $(BUILD_X86)/$(TARGET_X86)

$(BUILD_X86)/$(TARGET_X86): $(SRCS)
	mkdir -p $(BUILD_X86)
	g++ -m32 $(CPPFLAGS_INTEL) $(SRCS) -o $@ $(LDFLAGS_X86)

# -------------------------------
# x64 build
# -------------------------------
x64: $(BUILD_X64)/$(TARGET_X64)

$(BUILD_X64)/$(TARGET_X64): $(SRCS)
	mkdir -p $(BUILD_X64)
	g++ -m64 $(CPPFLAGS_INTEL) $(SRCS) -o $@ $(LDFLAGS_X64)

# -------------------------------
# ARMv7 build
# -------------------------------
armv7: $(BUILD_ARMV7)/$(TARGET_ARMV7)

$(BUILD_ARMV7)/$(TARGET_ARMV7): $(SRCS)
	mkdir -p $(BUILD_ARMV7)
	arm-linux-gnueabihf-g++ $(CPPFLAGS_ARMV7) $(SRCS) -o $@ $(LDFLAGS_ARMV7) -static

# -------------------------------
# AArch64 build
# -------------------------------
aarch64: $(BUILD_AARCH64)/$(TARGET_AARCH64)

$(BUILD_AARCH64)/$(TARGET_AARCH64): $(SRCS)
	mkdir -p $(BUILD_AARCH64)
	aarch64-linux-gnu-g++ $(CPPFLAGS_AARCH64) $(SRCS) -o $@ $(LDFLAGS_AARCH64) -static

# -------------------------------
# Clean
# -------------------------------
.PHONY: clean
clean:
	rm -rf $(BUILD_X86) $(BUILD_X64) $(BUILD_ARMV7) $(BUILD_AARCH64)
