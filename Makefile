# Default test file name
DEFTEST 	= six502_test
STTEST		= six502_test_st
SHTEST		= six502_test_sh

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

# Output library file names
# These values are valid for *unix systems
# and will be overritten in an "Architecture
# related stuff" section if the target system
# is win32
LIBNAME		= six502
LIBSTNAME	= lib$(LIBNAME).a
LIBSHNAME	= lib$(LIBNAME).so

# Architecture related stuff
TARGET_SYSTEM = ""

TARGET_SYSTEM := $(shell uname -s)
ifeq ($(OS), Windows_NT)
	TARGET_SYSTEM 	= "WIN_32"
	CXXFLAGS 		+= -D__WIN_32__
	LIBSTNAME		= $(LIBNAME).lib
	LIBSHNAME 		= lib$(LIBNAME).dll
	WIN32_IMPORTLIB	= lib$(LIBNAME).a
	LDFLAGS_SIX502_SHARED	= -Wl,-rpath=. -L. -lsix502
	LDFLAGS_SIX502_STATIC	= -L. -lsix502
	LDFLAGS_SHARED			= -Wl,--out-implib,$(WIN32_IMPORTLIB)
else
	LDFLAGS_SIX502_SHARED	= -L. -lsix502
	LDFLAGS_SIX502_STATIC	= -L. -lsix502

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
STTEST_SRC	:= $(TESTDIR)/$(STTEST).cpp
STTEST_OBJ 	:= $(TESTOBJDIR)/$(STTEST).o
SHTEST_SRC	:= $(TESTDIR)/$(SHTEST).cpp
SHTEST_OBJ 	:= $(TESTOBJDIR)/$(SHTEST).o

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

# Compile a static library with six502 debugger support
static_dbg: info_static $(ALLOBJ)
	$(Q)printf "Archiving objects into a static library $(ANSI_GREEN)$(LIBSTNAME)$(ANSI_RESET)\n"
	$(Q)$(AR) $(AROPTS) $(LIBSTNAME) $(ALLOBJ)
	$(Q)printf "\nBuild finished.\n"

# Compile a static library without six502 debugger support
static: info_static $(ALLOBJ_NODBG)
	$(Q)printf "Archiving objects into a static library $(ANSI_GREEN)$(LIBSTNAME)$(ANSI_RESET)\n"
	$(Q)$(AR) $(AROPTS) $(LIBSTNAME) $(ALLOBJ_NODBG)
	$(Q)printf "\nBuild finished.\n"

# Compile a shared library with six502 debugger support
shared_dbg: info_shared $(ALLOBJ)
	$(Q)printf "Linking into a shared library $(ANSI_GREEN)$(LIBSHNAME)$(ANSI_RESET)\n"
	$(Q)$(CXX) -shared $(INCLUDES) $(ALLOBJ) -o $(LIBSHNAME) $(LDFLAGS) $(LDFLAGS_SHARED)
	$(Q)printf "\nBuild finished.\n"

# Compile a shared library without six502 debugger support
shared: info_shared $(ALLOBJ_NODBG)
	$(Q)printf "Linking into a shared library $(ANSI_GREEN)$(LIBSHNAME)$(ANSI_RESET)\n"
	$(Q)$(CXX) -shared $(INCLUDES) $(ALLOBJ_NODBG) -o $(LIBSHNAME) $(LDFLAGS) $(LDFLAGS_SHARED)
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

# Test target that's compiled against a shared library
test_shared: $(SHTEST)
$(SHTEST): info_test $(SHTEST_OBJ)
ifeq (, $(wildcard $(LIBSHNAME)))
	$(error Can't link $(SHTEST): static library $(LIBSHNAME) has'n been built)
endif
	$(Q)printf "Linking into an executable $(ANSI_GREEN)$(TESTBINDIR)/$(SHTEST)$(ANSI_RESET)\n"
	$(Q)$(CXX) $(INCLUDES) $(SHTEST_OBJ) -o $(TESTBINDIR)/$(SHTEST) $(LDFLAGS) $(LDFLAGS_SIX502_SHARED)
	$(Q)printf "\nBuild finished.\n"
$(SHTEST_OBJ):
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $(SHTEST_SRC) -o $@
	$(Q)printf "Compiled $(SHTEST_OBJ)\n"

# Test target that's compiled statically against a static library
test_static: $(STTEST)
$(STTEST): info_test $(STTEST_OBJ)
ifeq (, $(wildcard $(LIBSTNAME)))
	$(error Can't link $(STTEST): static library $(LIBSTNAME) has'n been built)
endif
	$(Q)printf "Linking into an executable $(ANSI_GREEN)$(TESTBINDIR)/$(STTEST)$(ANSI_RESET)\n"
	$(Q)$(CXX) $(INCLUDES) $(STTEST_OBJ) -o $(TESTBINDIR)/$(STTEST) $(LDFLAGS) $(LDFLAGS_SIX502_STATIC)
	$(Q)printf "\nBuild finished.\n"
$(STTEST_OBJ):
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $(STTEST_SRC) -o $@
	$(Q)printf "Compiled $(STTEST_OBJ)\n"

.PHONY: clean
clean:
	$(Q)printf "Removing all $(ANSI_BOLD)SIX502$(ANSI_RESET) objects and executables..."
	$(Q)rm -f $(TESTBINDIR)/$(DEFTEST) $(DEFTEST_OBJ) $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS) $(DBG_OBJECTS)
	$(Q)rm -f $(LIBSTNAME) $(LIBSHNAME)
	$(Q)rm -f $(SHTEST) $(STTEST)
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
