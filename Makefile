CXXFLAGS=-std=c++11 -O3 -Wall -IMandelbrot/ -Wno-deprecated-declarations
MANDEL_OBJS=main.o OffsceenSurface.o GLUTWrapper.o
TARGET=mandel
FRAMEWORKS=OpenGL GLUT CoreFoundation ImageIO CoreServices CoreGraphics
LDFLAGS=$(foreach fw,$(FRAMEWORKS), -framework $(fw))


all: $(TARGET)

clean:
	rm $(TARGET) $(MANDEL_OBJS)

$(TARGET): $(MANDEL_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: Mandelbrot/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

