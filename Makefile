OS := $(shell uname)

ifeq ($(OS),Darwin)
    CFLAGS += -DMACOS
    CPPFLAGS += -I$(shell brew --prefix)/include
    OPENSSL_PREFIX = /opt/homebrew/Cellar/openssl@3/3.5.0
    OPENSSL_LIB = $(OPENSSL_PREFIX)/lib
    LIBS = -lcrypto 
else ifeq ($(OS),Linux)
    CFLAGS += -DLINUX
    OPENSSL_INC = /usr/include/
    OPENSSL_LIB = /usr/lib
    LIBS = -lcrypto -lrt
endif

ASIO_PREFIX  := asio
ASIO_VERSION := 1.28.0

ASIO_PACKAGE = $(ASIO_PREFIX)-$(ASIO_VERSION).tar.bz2


CXXFLAGS = -g -std=c++2a

CPPFLAGS += -I$(ASIO_PREFIX)-$(ASIO_VERSION)/include

LDFLAGS = -pthread -L$(OPENSSL_LIB)

TARGET := hasher-server

SOURCES = hasher-main.cpp \
          hasher-server.cpp \
          hasher-stream.cpp \
          buffer.cpp \
          hasher-config.cpp

OBJS := $(SOURCES:.cpp=.o)


all: $(TARGET)

clean:
	rm $(TARGET) $(OBJS)

deps: $(ASIO_PREFIX)-$(ASIO_VERSION)

$(ASIO_PREFIX)-$(ASIO_VERSION):
	tar -xvf 3rdparty/$(ASIO_PACKAGE)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@ $(LIBS)
