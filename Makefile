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
  cflags = -I ./src/websocketpp/ $(OPENSSL_SUPPORT) $(CXXFLAGS) -std=c++17 
  ldlibs = -lssl -pthread 

else ifeq (Darwin,$(findstring Darwin,$(uname)))
  PREFIX = /usr/local
  OPENSSL_DIR := $(shell dirname $(dirname $(dirname $(shell find / -name err.h -path "*/openssl/*" -print -quit 2>/dev/null))))
  OPENSSL_SUPPORT := -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_DIR)/include
  cflags := -mmacosx-version-min=10.9  -std=c++11 $(OPENSSL_SUPPORT) -O2 -stdlib=libc++ -I.. -Wall -Wextra -DCPPHTTPLIB_OPENSSL_SUPPORT -I/usr/local/opt/openssl@1.1/include -DCPPHTTPLIB_ZLIB_SUPPORT 
  ldlibs = -pthread -L/usr/local/opt/openssl@3/lib -lssl

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

