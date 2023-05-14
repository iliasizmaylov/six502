PACKAGE 	= six502_test
VERSION		= 0.0001

CPPSTD 		= -std=c++17
CXXFLAGS 	= -g -Wall -Wextra $(CPPSTD) $(CFLAGS_PLATFORM)
LDFLAGS	 	= $(LDFLAGS_PLATFORM) -lsdl2

INCLUDES	= -I.

SOURCES		:= $(wildcard *.cpp)
OBJECTS		:= $(SOURCES:.cpp=.o)
OP_SOURCES	:= $(wildcard ops/*.cpp)
OP_OBJECTS	:= $(OP_SOURCES:.cpp=.o)

ANSI_RESET	= \e[0m
ANSI_BOLD	= \e[1m
ANSI_ULINE	= \e[4m
ANSI_RED	= \e[31m
ANSI_GREEN	= \e[32m

all: $(PACKAGE)

$(PACKAGE): info $(OBJECTS) $(OP_OBJECTS)
	@printf "Linking into an executable $(ANSI_GREEN)$(PACKAGE)$(ANSI_RESET)\n"
	@$(CXX) $(INCLUDES) $(OBJECTS) $(OP_OBJECTS) -o $(PACKAGE) $(LDFLAGS)
	@printf "\nBuild finished.\n"

%.o: %.cpp
	@$(CXX) $(INCLUDES) $(CXXFLAGS) $< -c -o $@
	@printf "Compiling $<...\n"

info:
	@printf "Building $(ANSI_BOLD)SIX502$(ANSI_RESET)...\n\n"

.PHONY: clean
clean:
	@printf "Removing all $(ANSI_BOLD)SIX502$(ANSI_RESET) objects and executables..."
	@rm -f $(PACKAGE) $(OBJECTS) $(OP_OBJECTS)
	@printf " $(ANSI_GREEN)Done$(ANSI_RESET)!\n"
