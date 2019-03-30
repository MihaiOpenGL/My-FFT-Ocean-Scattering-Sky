
CFG ?= release
ifeq ($(CFG), release)
    CFGDIR = release
	CXXCFG = -O3
else
    CFGDIR = debug
	CXXCFG = -g -D_DEBUG
endif

CXX 				= g++
CXXFLAGS 	= $(CXXCFG) -Wall -std=c++11
LD				= g++

OS				= linux
INCDIR			= include
SRCDIR		= source
SRCDIREXT   = glad

LIBDIR			= libs/$(OS)
BUILDDIR		= build/$(OS)/$(CFGDIR)
TARGETDIR	= bin/$(OS)/$(CFGDIR)
OBJDIR		= $(BUILDDIR)

INC				= -I$(INCDIR) 
LDFLAGS		=  -L$(LIBDIR) -L/usr/lib -L/usr/local/lib -lSDL2 -lSDL2main -lfftw3f -lAntTweakBar -lGL -lX11 -lpthread -ldl

EXE				= MyFFTOcean
TARGET		= $(TARGETDIR)/$(EXE)

SRCS			= $(wildcard $(SRCDIR)/*.cpp)
OBJS			= $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
SRCSEXT            = $(wildcard $(SRCDIREXT)/*.cpp)
OBJSEXT            = $(patsubst $(SRCDIREXT)/%.cpp, $(OBJDIR)/%.o, $(SRCSEXT))

$(TARGET): $(OBJS) $(OBJSEXT)
	@mkdir -p $(TARGETDIR)
	@echo $(OBJS)
	@echo $(OBJSEXT)
	@echo " Linking $@"; $(LD) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	@echo "Compiling $<..."; $(CXX) -c $(CXXFLAGS) $(INC) $< -o $@

$(OBJDIR)/%.o : $(SRCDIREXT)/%.cpp
	@mkdir -p $(BUILDDIR)
	@echo "Compiling $<..."; $(CXX) -c $(CXXFLAGS) $(INC) $< -o $@

clean:  
	@echo "Cleaning $(BUILDDIR)  $(TARGET)..."; rm -rf $(BUILDDIR) $(TARGET)
