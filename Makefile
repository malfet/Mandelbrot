OS=$(shell uname)

TARGET=mandel
MANDEL_OBJS=main.o OffsceenSurface.o GLUTWrapper.o

CXXFLAGS=-std=c++11 -O3 -Wall -IMandelbrot/ -Wno-deprecated-declarations

ifeq ($(OS),Darwin)
FRAMEWORKS=OpenGL GLUT CoreFoundation ImageIO CoreServices CoreGraphics
LDFLAGS=$(foreach fw,$(FRAMEWORKS), -framework $(fw))
endif

ifeq ($(OS),Linux)
LDFLAGS=-pthread -lGL -lglut
ifeq ($(shell uname -m),armvl7)
LDFLAGS += -L/usr/lib/arm-linux-gnueabihf/tegra/
endif
endif


all: $(TARGET)

clean:
	rm $(TARGET) $(MANDEL_OBJS)

$(TARGET): $(MANDEL_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: Mandelbrot/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

