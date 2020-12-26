EXECUTABLE=myapp

INCDIR=inc
SRCDIR=src
OBJDIR=obj
BINDIR=bin
RSCDIR=rsc
LIBDIR=lib

IMGUIDIR=imgui
IMGUISRCDIR=$(SRCDIR)/$(IMGUIDIR)
IMGUIINCDIR=$(SRCDIR)

OBJS = $(patsubst $(SRCDIR)%.cpp,$(OBJDIR)%.o,$(wildcard $(SRCDIR)/*.cpp))
OBJS += $(patsubst $(IMGUISRCDIR)/%.cpp,$(OBJDIR)/$(IMGUIDIR)/%.o,$(wildcard $(IMGUISRCDIR)/*.cpp))

CXX=g++
CXXFLAGS=-Wall -isystem $(INCDIR) -isystem $(IMGUIINCDIR) -c -std=c++17 -g
LIBS=$(shell pkg-config --static --libs glfw3 gl) -lassimp
#LIBS=-lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor
LDFLAGS=-L$(LIBDIR) $(LIBS) -Wl,-rpath,$(PWD)/$(LIBDIR)

.PHONY: all
all: $(BINDIR)/$(EXECUTABLE)

.PHONY: clean
clean:
	rm -vrf $(BINDIR) $(OBJDIR)

.PHONY: run
run: $(BINDIR)/$(EXECUTABLE)
	ln -sf $(PWD)/rsc/* $(PWD)/bin 
	# change the working directory to the bin folder so that
	# the path to symbolic links are relavtive to the executable.
	(cd $(BINDIR) && exec ./$(EXECUTABLE))

$(BINDIR)/$(EXECUTABLE): $(OBJS)
	mkdir -p $(BINDIR)
	ln -sf $(PWD)/rsc/* $(PWD)/bin 
	$(CXX) -o $@ $^ $(LDFLAGS) 

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(OBJDIR)/$(IMGUIDIR)/%.o: $(IMGUISRCDIR)/%.cpp
	mkdir -p $(OBJDIR)/imgui
	$(CXX) $(CXXFLAGS) $< -o $@
