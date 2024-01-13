PACKAGE 	= six502_test
VERSION		= 0.0001

Q = @

TARGET_SYSTEM = ""

TARGET_SYSTEM := $(shell uname -s)
ifeq ($(OS), Windows_NT)
	TARGET_SYSTEM = "WIN_32"
endif

ifeq ($(TARGET_SYSTEM), Darwin)
	OSX_SDL_FRAMEWORKS = -framework SDL2	\
					-framework SDL2_ttf		\
					-framework SDL2_image
	OSX_F = -F/Library/Frameworks
endif

CPPSTD 		= -std=c++17
CXXFLAGS 	= -g -Wall -Wextra $(CPPSTD) $(CFLAGS_PLATFORM)
LDFLAGS	 	= $(LDFLAGS_PLATFORM) -L/opt/local/lib -lSDL2 -lSDL2_image

SRCDIR		:= src
TESTDIR		:= tests
OPSDIR		:= $(SRCDIR)/ops
DBGDIR		:= $(SRCDIR)/debugger
VIDEODIR	:= $(SRCDIR)/video
TESTOBJDIR	:= $(TESTDIR)/obj
TESTSRCDIR	:= $(TESTDIR)
TESTBINDIR 	:= $(TESTDIR)/bin
PACKAGE_SRC	:= $(TESTSRCDIR)/$(PACKAGE).cpp
PACKAGE_OBJ := $(TESTOBJDIR)/$(PACKAGE).o

INCLUDES	= -I$(SRCDIR) -I$(VIDEODIR) -I$(DBGDIR) -I.

SOURCES		:= $(wildcard $(SRCDIR)/*.cpp)
OBJECTS		:= $(SOURCES:$(SRCDIR)/%.cpp=$(TESTOBJDIR)/%.o)
OP_SOURCES	:= $(wildcard $(OPSDIR)/*.cpp)
OP_OBJECTS	:= $(OP_SOURCES:$(OPSDIR)/%.cpp=$(TESTOBJDIR)/%.o)
SDL_SOURCES	:= $(wildcard $(VIDEODIR)/*.cpp)
SDL_OBJECTS := $(SDL_SOURCES:$(VIDEODIR)/%.cpp=$(TESTOBJDIR)/%.o)
DBG_SOURCES	:= $(wildcard $(DBGDIR)/*.cpp)
DBG_OBJECTS := $(DBG_SOURCES:$(DBGDIR)/%.cpp=$(TESTOBJDIR)/%.o)

ANSI_RESET	= \e[0m
ANSI_BOLD	= \e[1m
ANSI_ULINE	= \e[4m
ANSI_RED	= \e[31m
ANSI_GREEN	= \e[32m

all: $(PACKAGE)

$(PACKAGE): info $(PACKAGE_OBJ) $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS) $(DBG_OBJECTS)
	$(Q)printf "Linking into an executable $(ANSI_GREEN)$(TESTBINDIR)/$(PACKAGE)$(ANSI_RESET)\n"
	$(Q)$(CXX) $(INCLUDES) $(PACKAGE_OBJ) $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS) $(DBG_OBJECTS) -o $(TESTBINDIR)/$(PACKAGE) $(LDFLAGS)
	$(Q)printf "\nBuild finished.\n"

$(PACKAGE_OBJ):
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $(PACKAGE_SRC) -o $@
	$(Q)printf "Compiled $(PACKAGE_OBJ)\n"

$(OBJECTS): $(TESTOBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

$(OP_OBJECTS): $(TESTOBJDIR)/%.o : $(OPSDIR)/%.cpp
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

$(SDL_OBJECTS): $(TESTOBJDIR)/%.o : $(VIDEODIR)/%.cpp
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

$(DBG_OBJECTS): $(TESTOBJDIR)/%.o : $(DBGDIR)/%.cpp
	$(Q)mkdir -p $(TESTOBJDIR)
	$(Q)$(CXX) $(INCLUDES) $(CXXFLAGS) $(OSX_F) -c $< -o $@
	$(Q)printf "Compiled $<\n"

info:
	$(Q)printf "\nBuilding $(ANSI_BOLD)SIX502 Test \"$(PACKAGE)\"$(ANSI_RESET)...\n\n"

.PHONY: clean
clean:
	$(Q)printf "Removing all $(ANSI_BOLD)SIX502$(ANSI_RESET) objects and executables..."
	$(Q)rm -f $(TESTBINDIR)/$(PACKAGE) $(PACKAGE_OBJ) $(OBJECTS) $(OP_OBJECTS) $(SDL_OBJECTS) $(DBG_OBJECTS)
	$(Q)printf " $(ANSI_GREEN)Done$(ANSI_RESET)!\n"
