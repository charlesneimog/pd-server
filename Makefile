# library name
lib.name = server

# -O2 -std=c++11 -I.. -Wall -Wextra

uname := $(shell uname -s)

ifeq (MINGW,$(findstring MINGW,$(uname)))
  cflags = -O2 -Winline  -Wall -Wextra -DCPPHTTPLIB_OPENSSL_SUPPORT -Wno-cast-function-type 
  ldlibs =  -lssl -lcrypto -lwinpthread -lwsock32 -lws2_32 -lcrypt32


else ifeq (Linux,$(findstring Linux,$(uname)))
  PREFIX = /usr/local
  OPENSSL_DIR = $(PREFIX)/opt/openssl@1.
  OPENSSL_SUPPORT = -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_DIR)/include -L$(OPENSSL_DIR)/lib 
  cflags = $(OPENSSL_SUPPORT) $(CXXFLAGS) -std=c++11 
  ldlibs = -lssl -pthread 

else ifeq (Darwin,$(findstring Darwin,$(uname)))
  # compile with clang++
  PREFIX = /usr/local
  OPENSSL_DIR := $(shell pkg-config --variable=prefix openssl || echo $(PREFIX))
  OPENSSL_INCLUDE_DIR := $(shell pkg-config --cflags-only-I openssl | sed 's/^-I//')
  OPENSSL_LIB_DIR := $(shell pkg-config --variable=libdir openssl)
  OPENSSL_SUPPORT = -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_INCLUDE_DIR)
  cflags = -mmacosx-version-min=10.9 -std=c++11 $(OPENSSL_SUPPORT) -O2 -stdlib=libc++ -I.. -Wall -Wextra -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_INCLUDE_DIR) -DCPPHTTPLIB_ZLIB_SUPPORT 
  ldlibs = -pthread -L$(OPENSSL_LIB_DIR) -lssl


else
  $(error "Unknown system type: $(uname)")
  $(shell exit 1)

endif

# =================================== Sources ===================================

server.class.sources = src/server.cc

# =================================== Data ======================================
datafiles = 
# =================================== Pd Lib Builder =============================

PDLIBBUILDER_DIR=./pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

