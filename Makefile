CROSS_COMPILE=arm-linux-gnueabi-
ZQSENDER_ENABLE=false

CC= $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar


LIVE=../live
LIVE_INC=-I$(LIVE)/BasicUsageEnvironment/include -I$(LIVE)/groupsock/include -I$(LIVE)/UsageEnvironment/include -I$(LIVE)/liveMedia/include
LIVE_LIBS=$(LIVE)/liveMedia/libliveMedia.a \
	  $(LIVE)/groupsock/libgroupsock.a \
	  $(LIVE)/BasicUsageEnvironment/libBasicUsageEnvironment.a \
	  $(LIVE)/UsageEnvironment/libUsageEnvironment.a

CFLAGS += -Wall -DOS_LINUX -O2 -g
CXXFLAGS += -Wall -DOS_LINUX -O2 -g $(LIVE_INC)
LIBS += ./lib/libcedarv.a ./lib/libh264enc.a ./lib/libcedarv_osal.a ./lib/libcedarxalloc.a \
	./lib/libccgnu2.a $(LIVE_LIBS) -lpthread

ifeq ($(ZQSENDER_ENABLE), true)
	$(CXXFLAGS) += -DZQSENDER
	$(LIBS) += ./lib/libzqsender.a
endif

SRC := $(wildcard *.c)
SRCC := $(wildcard *.cpp)
OBJ := $(SRC:%.c=%.o) $(SRCC:%.cpp=%.o)

TARGET := ct_webcam_rtspd
.PHONY : clean all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)

