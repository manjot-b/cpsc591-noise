EXECUTABLE=myapp

INCDIR=inc
SRCDIR=src
OBJDIR=obj
BINDIR=bin
RSCDIR=rsc
LIBDIR=lib
ASSIMP_LIB_DIR=dep/assimp/build/bin
ASSIMP_INC_DIR=dep/assimp/include
ASSIMP_CONF_DIR=dep/assimp/build/include
GLFW_LIB_DIR=dep/glfw/build/src
GLFW_INC_DIR=dep/glfw/include

IMGUIDIR=imgui
IMGUISRCDIR=$(SRCDIR)/$(IMGUIDIR)
IMGUIINCDIR=$(SRCDIR)

OBJS = $(patsubst $(SRCDIR)%.cpp,$(OBJDIR)%.o,$(wildcard $(SRCDIR)/*.cpp))
OBJS += $(patsubst $(IMGUISRCDIR)/%.cpp,$(OBJDIR)/$(IMGUIDIR)/%.o,$(wildcard $(IMGUISRCDIR)/*.cpp))

CXX=g++
CXXFLAGS=-Wall -I $(INCDIR) -I $(IMGUIINCDIR) -I$(ASSIMP_CONF_DIR) -I$(ASSIMP_INC_DIR) -I$(GLFW_INC_DIR)
CXXFLAGS+= -c -std=c++17 -g -DIMGUI_IMPL_OPENGL_LOADER_GLAD
LIBS= $(shell pkg-config --static --libs gl) -lglfw3 -lassimp -ldl -lpthread
#LIBS=-lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor
LDFLAGS= -L$(LIBDIR) -L$(ASSIMP_LIB_DIR) -L$(GLFW_LIB_DIR) $(LIBS)
LDFLAGS+= -Wl,-rpath=$(PWD)/$(LIBDIR) -Wl,-rpath=$(PWD)/$(ASSIMP_LIB_DIR)

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
