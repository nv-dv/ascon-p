TARGET_EXEC32 := main_x86
TARGET_EXEC64 := main_x64

BUILD_DIR32 := bin_x86
BUILD_DIR64 := bin_x64
SRC_DIRS := src

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
SRCS64 := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
SRCS_ARM := $(shell find $(SRC_DIRS) -name 'arm*.cpp' -or -name '*.c' -or -name '*.s')

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS32 := $(SRCS:%=$(BUILD_DIR32)/%.o)
OBJS64 := $(SRCS:%=$(BUILD_DIR64)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
# DEPS := $(OBJS:.o=.d)

# Every folder in src/ will need to be passed to GCC so that it can find header files
INC_DIRS := src # $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))


# Include M4RI library
LDFLAGS += -lm4ri

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -O3 -mavx2 -fno-builtin -masm=intel

.PHONY: all
all: $(BUILD_DIR32)/$(TARGET_EXEC32) $(BUILD_DIR64)/$(TARGET_EXEC64)

.PHONY: x86
x86: $(BUILD_DIR32)/$(TARGET_EXEC32)

.PHONY: x64
x64: $(BUILD_DIR64)/$(TARGET_EXEC64)

.PHONY: armv7
armv7:
	mkdir -p bin_armv7/
	arm-linux-gnueabihf-g++ -Isrc -fno-builtin -O3 -static -Wformat=0 arm_main.cpp src/random/*.cpp src/external/usuba_mask/masked_ascon_ua_vslice.cpp -o bin_armv7/main_armv7

.PHONY: aarch64
aarch64:
	mkdir -p bin_aarch64/
	aarch64-linux-gnu-g++ -Isrc -fno-builtin -O3 -static -Wformat=0 arm_main.cpp src/random/*.cpp src/external/usuba_mask/masked_ascon_ua_vslice.cpp -o bin_armv7/main_aarch64

# The final build step.
$(BUILD_DIR32)/$(TARGET_EXEC32): $(OBJS32)
	$(CXX) -m32 $^ -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR32)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -m32 $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR32)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -m32 $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# The final build step.
$(BUILD_DIR64)/$(TARGET_EXEC64): $(OBJS64)
	$(CXX) -m64 $^ -o $@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR64)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -m64 $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR64)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -m64 $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -r bin_*
