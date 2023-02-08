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
  cflags = $(OPENSSL_SUPPORT) $(CXXFLAGS) 
  ldlibs = -lssl -pthread 

else ifeq (Darwin,$(findstring Darwin,$(uname)))
  # compile with clang++
  CXX = clang++
  PREFIX = /usr/local
  OPENSSL_DIR = $(PREFIX)/opt/openssl@1.1
  OPENSSL_SUPPORT = -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_DIR)/include
  cflags = $(OPENSSL_SUPPORT) -O2 -std=c++11 -I.. -Wall -Wextra 
  ldlibs = -lssl -pthread -libssl


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

