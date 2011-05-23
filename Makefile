
CUDA_INSTALL_PATH ?= /usr

CCFLAGS    :=
CXXFLAGS   := `pkg-config OGRE OIS --cflags --silence-errors`
LDFLAGS    := -lcuda -lcudart `pkg-config OGRE OIS --libs --silence-errors`
NVCCFLAGS  := --compiler-options -fpermissive --compiler-bindir=/opt/gcc-4.4

NVCC       := $(CUDA_INSTALL_PATH)/bin/nvcc
CXX        := g++ -fPIC
CC         := gcc -fPIC
LINK       := g++ -fPIC


all: particles

particles: main.o particles.o
	$(LINK) $(LDFLAGS) $^ -o $@

main.o: main.cpp
	$(CXX) -c $(CXXFLAGS) $?

particles.o: particles.cu
	$(NVCC) -c $(NVCCFLAGS) $?

clean:
	rm -fr *.o *core*

.PHONY: clean

