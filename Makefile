ASIO_PREFIX  := asio
ASIO_VERSION := 1.28.0

ASIO_PACKAGE = $(ASIO_PREFIX)-$(ASIO_VERSION).tar.bz2

TARGET := hasher-server

CXXFLAGS = -g -std=c++2a

CPPFLAGS = -I$(ASIO_PREFIX)-$(ASIO_VERSION)/include

SOURCES = hasher-main.cpp \
          hasher-server.cpp \
          hasher-stream.cpp \
          buffer.cpp \
          hasher-config.cpp

OBJS := $(SOURCES:.cpp=.o)

LIBS = -lcrypto -lrt

LDFLAGS = -pthread

all: $(TARGET)

clean:
	rm $(TARGET) $(OBJS)

deps: $(ASIO_PREFIX)-$(ASIO_VERSION)

$(ASIO_PREFIX)-$(ASIO_VERSION):
	tar -xvf 3rdparty/$(ASIO_PACKAGE)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@ $(LIBS)
