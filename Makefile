
PLUGIN_NAME := geanyjedicomplete.so

LANG_SRCS := preferences.cpp completion_framework.cpp completion.cpp plugin_info.cpp
LANG_SRCS := $(addprefix src/, $(LANG_SRCS))

CXXFLAGS += -O2
LDFLAGS  += -lcurl

include geany-complete-core/Makefile.core

