# library name
lib.name = server

# -O2 -std=c++11 -I.. -Wall -Wextra

uname := $(shell uname -s)

ifeq (MINGW,$(findstring MINGW,$(uname)))
  cflags = -O2 -std=c++11 -Wall -Wextra -DCPPHTTPLIB_OPENSSL_SUPPORT -Wno-cast-function-type 
  ldlibs =  -lwinpthread -lssl -lcrypto


else ifeq (Linux,$(findstring Linux,$(uname)))
  cflags = -O2 -Winline  -Wall -Wextra -DCPPHTTPLIB_OPENSSL_SUPPORT -Wno-cast-function-type 
  ldlibs = -lssl -lcrypto 

else ifeq (Darwin,$(findstring Darwin,$(uname)))
  cflags = -Wno-cast-function-type 
  ldlibs = -lssl -lcrypto -libpthread

else
  $(error "Unknown system type: $(uname)")
  $(shell exit 1)

endif

# =================================== Sources ===================================

server.class.sources = src/server.cpp

# =================================== Data ======================================
datafiles = 
# =================================== Pd Lib Builder =============================

PDLIBBUILDER_DIR=./pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

