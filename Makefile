INCLUDE = -I/usr/include/
LIBDIR  = -L/usr/X11R6/lib 
CC = g++

ifeq ($(shell uname),Darwin)
	CFLAGS = -Wall  $(COMPILERFLAGS) $(INCLUDE) $(LIBDIR)
else
	CFLAGS = -Wall -g
endif

PROG = game

SRCS = Game.cpp

ifeq ($(shell uname),Darwin)
	LIBS = -lSOIL -framework OpenGL -framework GLUT -framework CoreFoundation -lX11 -lXi -lXmu 
else
	LIBS = -lglut -lGL -lSOIL -lGLU 
endif



all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)

# run for other codes as well , read README
