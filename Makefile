
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

LIBDIR			= libs/$(OS)
BUILDDIR		= build/$(OS)/$(CFGDIR)
TARGETDIR	= bin/$(OS)/$(CFGDIR)
OBJDIR		= $(BUILDDIR)

INC				= -I$(INCDIR) 
LDFLAGS		=  -L$(LIBDIR) -L/usr/lib -L/usr/local/lib -lGLEW -lglfw3 -lfftw3f -lAntTweakBar -lGL -lX11 -lpthread -ldl

EXE				= MyFFTOcean
TARGET		= $(TARGETDIR)/$(EXE)

SRCS			= $(wildcard $(SRCDIR)/*.cpp)
OBJS			= $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))


$(TARGET): $(OBJS)
	@mkdir -p $(TARGETDIR)
	@echo $(OBJS)
	@echo " Linking $(TARGET)"; $(LD) $^ -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	@echo "Compiling $<..."; $(CXX) -c $(CXXFLAGS) $(INC) $< -o $@

clean:  
	@echo "Cleaning $(BUILDDIR)  $(TARGET)..."; rm -rf $(BUILDDIR) $(TARGET)
