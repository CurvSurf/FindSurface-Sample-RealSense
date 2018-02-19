# Compile & Link Parameters
CC = g++

ADDITIONAL_INCLUDE_PATH =
ADDITIONAL_LIB_PATH = -L/home/curvsurf/CurvSurf/linux_ubuntu/libFindSurface/lib_import/x86_64

CFLAGS = $(ADDITIONAL_INCLUDE_PATH) -std=c++11
LIBS = $(ADDITIONAL_LIB_PATH) -lm -lX11 -lGL -lglfw -lGLEW -lFindSurface -lrealsense

# Output Parameters
TARGET = RealSenseDemo
OBJDIR = obj

# SOURCE FILES
VPATH = src
SOURCES = \
main.cpp \
Application.cpp \
opengl_wrapper.cpp \
shader_resources.cpp \
sgeometry.cpp \
camera.cpp

OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SOURCES))

# Define make rules
all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/%.o: %.cpp
	@$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	@$(CC) -o $@ $^ $(LIBS)

clean:
	@rm -rf $(OBJDIR)/*.o $(TARGET)
