# Default test file name
DEFTEST 	= six502_test

Q = @

# Compiler flags
CPPSTD 				= -std=c++17
CXXFLAGS 			= -g -Wall -Wextra $(CPPSTD) $(CFLAGS_PLATFORM)
LDFLAGS	 			= $(LDFLAGS_PLATFORM) -L/opt/local/lib -lSDL2 -lSDL2_image
AR					= ar
AROPTS				= rs
SIX502DBG			= -D__SIX502_DBG_ENABLED__

__IS_SHARED			:= $(filter shared,$(MAKECMDGOALS))
IS_SHARED			:= $(if $(__IS_SHARED),1,0)
__IS_SHARED_DBG		:= $(filter shared_dbg,$(MAKECMDGOALS))
IS_SHARED_DBG		:= $(if $(__IS_SHARED_DBG),1,0)
__IS_STATIC_DBG		:= $(filter static_dbg,$(MAKECMDGOALS))
IS_STATIC_DBG		:= $(if $(__IS_STATIC_DBG),1,0)

ifeq ($(IS_SHARED), 1)
	CXXFLAGS += -fPIC
endif

ifeq ($(IS_SHARED_DBG), 1)
	CXXFLAGS += -fPIC
	CXXFLAGS += $(SIX502DBG)
endif

ifeq ($(IS_STATIC_DBG), 1)
	CXXFLAGS += -fPIC
	CXXFLAGS += $(SIX502DBG)
endif

# Architecture related stuff
TARGET_SYSTEM = ""

TARGET_SYSTEM := $(shell uname -s)
ifeq ($(OS), Windows_NT)
	TARGET_SYSTEM = "WIN_32"
	CXXFLAGS += -D__WIN_32__
else
	ifeq ($(TARGET_SYSTEM), Darwin)
			OSX_SDL_FRAMEWORKS = -framework SDL2	\
							-framework SDL2_ttf		\
							-framework SDL2_image
			OSX_F = -F/Library/Frameworks
			CXXFLAGS += -D__OSX__
	endif
	ifeq ($(TARGET_SYSTEM), Linux)
			CXXFLAGS += -D__LINUX__
	endif
endif

# Library version
LIBVER		= 0

# Output library file names
LIBSTNAME	= six502.a
LIBSHNAME	= libsix502.so.$(LIBVER)

# Common sources folder
SRCDIR		:= src
SRCOBJDIR	:= $(SRCDIR)/obj

# Folder for test files
TESTDIR		:= tests
TESTOBJDIR	:= $(TESTDIR)/obj
TESTBINDIR 	:= $(TESTDIR)/bin

# 6502 Instruction implementation folder
OPSDIR		:= $(SRCDIR)/ops
OPSOBJDIR	:= $(OPSDIR)/obj

# six502 Debugger folder
DBGDIR		:= $(SRCDIR)/debugger
DBGOBJDIR	:= $(DBGDIR)/obj

# SDL (video output) stuff for six502 debugger
VIDEODIR	:= $(SRCDIR)/video
VIDEOOBJDIR	:= $(VIDEODIR)/obj

# Default test file sources and objects
DEFTEST_SRC	:= $(TESTDIR)/$(DEFTEST).cpp
DEFTEST_OBJ := $(TESTOBJDIR)/$(DEFTEST).o

# Common source files and objects
SOURCES		:= $(wildcard $(SRCDIR)/*.cpp)
OBJECTS		:= $(SOURCES:$(SRCDIR)/%.cpp=$(SRCOBJDIR)/%.o)

# 6502 instructions source files and objects
OP_SOURCES	:= $(wildcard $(OPSDIR)/*.cpp)
OP_OBJECTS	:= $(OP_SOURCES:$(OPSDIR)/%.cpp=$(OPSOBJDIR)/%.o)

# SDL (video output) source files and objects
SDL_SOURCES	:= $(wildcard $(VIDEODIR)/*.cpp)
SDL_OBJECTS := $(SDL_SOURCES:$(VIDEODIR)/%.cpp=$(VIDEOOBJDIR)/%.o)

# six502 debugger source files and objects
DBG_SOURCES	:= $(wildcard $(DBGDIR)/*.cpp)
DBG_OBJECTS := $(DBG_SOURCES:$(DBGDIR)/%.cpp=$(DBGOBJDIR)/%.o)

# All objects
ALLOBJ			:= $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS) $(DBG_OBJECTS)
ALLOBJ_NODBG	:= $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS)

# Include directories
INCLUDES	= -I$(SRCDIR) -I$(VIDEODIR) -I$(DBGDIR) -I.

# Colored output to console
ANSI_RESET	= \e[0m
ANSI_BOLD	= \e[1m
ANSI_ULINE	= \e[4m
ANSI_RED	= \e[31m
ANSI_GREEN	= \e[32m

# ---------------- Main make targets ----------------

static_dbg: info_static $(ALLOBJ)
	$(Q)printf "Archiving objects into a static library $(ANSI_GREEN)$(LIBSTNAME)$(ANSI_RESET)\n"
	$(Q)$(AR) $(AROPTS) $(LIBSTNAME) $(ALLOBJ)
	$(Q)printf "\nBuild finished.\n"

static: info_static $(ALLOBJ_NODBG)
	$(Q)printf "Archiving objects into a static library $(ANSI_GREEN)$(LIBSTNAME)$(ANSI_RESET)\n"
	$(Q)$(AR) $(AROPTS) $(LIBSTNAME) $(ALLOBJ_NODBG)
	$(Q)printf "\nBuild finished.\n"

shared_dbg: info_shared $(ALLOBJ)
	$(Q)printf "Linking into a shared library $(ANSI_GREEN)$(LIBSHNAME)$(ANSI_RESET)\n"
	$(Q)$(CXX) -shared $(INCLUDES) $(ALLOBJ) -o $(LIBSHNAME) $(LDFLAGS)
	$(Q)printf "\nBuild finished.\n"

shared: info_shared $(ALLOBJ_NODBG)
	$(Q)printf "Linking into a shared library $(ANSI_GREEN)$(LIBSHNAME)$(ANSI_RESET)\n"
	$(Q)$(CXX) -shared $(INCLUDES) $(ALLOBJ_NODBG) -o $(LIBSHNAME) $(LDFLAGS)
	$(Q)printf "\nBuild finished.\n"

# Default six502 test target
test: $(DEFTEST)
$(DEFTEST): info_test $(DEFTEST_OBJ) $(ALLOBJ)
	$(Q)printf "Linking into an executable $(ANSI_GREEN)$(TESTBINDIR)/$(DEFTEST)$(ANSI_RESET)\n"
	$(Q)$(CXX) $(INCLUDES) $(DEFTEST_OBJ) $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS) $(DBG_OBJECTS) -o $(TESTBINDIR)/$(DEFTEST) $(LDFLAGS)
	$(Q)printf "\nBuild finished.\n"
$(DEFTEST_OBJ):
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $(DEFTEST_SRC) -o $@
	$(Q)printf "Compiled $(DEFTEST_OBJ)\n"

.PHONY: clean
clean:
	$(Q)printf "Removing all $(ANSI_BOLD)SIX502$(ANSI_RESET) objects and executables..."
	$(Q)rm -f $(TESTBINDIR)/$(DEFTEST) $(DEFTEST_OBJ) $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS) $(DBG_OBJECTS)
	$(Q)rm -f $(LIBSTNAME) $(LIBSHNAME)
	$(Q)printf " $(ANSI_GREEN)Done$(ANSI_RESET)!\n"

# ---------------- Make "subtargets" ----------------

# Main sources
$(OBJECTS): $(SRCOBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(Q)mkdir -p $(SRCOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

# 6502 instruction set
$(OP_OBJECTS): $(OPSOBJDIR)/%.o : $(OPSDIR)/%.cpp
	$(Q)mkdir -p $(OPSOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

# SDL video stuff for debugger
$(SDL_OBJECTS): $(VIDEOOBJDIR)/%.o : $(VIDEODIR)/%.cpp
	$(Q)mkdir -p $(VIDEOOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

# Debugger logic
$(DBG_OBJECTS): $(DBGOBJDIR)/%.o : $(DBGDIR)/%.cpp
	$(Q)mkdir -p $(DBGOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

# Aux info targets
.PHONY: info_test
info_test:
	$(Q)printf "\nBuilding $(ANSI_BOLD)SIX502 Test \"$(DEFTEST)\"$(ANSI_RESET)...\n\n"

.PHONY: info_static
info_static:
	$(Q)printf "\nBuilding $(ANSI_BOLD)SIX502 static library$(ANSI_RESET)...\n\n"

.PHONY: info_shared
info_shared:
	$(Q)printf "\nBuilding $(ANSI_BOLD)SIX502 shared library$(ANSI_RESET)...\n\n"
