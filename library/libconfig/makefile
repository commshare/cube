SRCS = tinystr.cpp \
       tinyxml.cpp \
       tinyxmlerror.cpp\
       tinyxmlparser.cpp\
       tixml_helper.cpp\
       config_meta.cpp

OBJS = tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o tixml_helper.o config_meta.o

CXXFLAGS = -Wall -std=c++0x -fpic -O3

TARGET_OUT_PATH:=../../public

PHONY : all 
all : $(OBJS) 
	ar r $(TARGET_OUT_PATH)/lib/libesunnyconf.a $(OBJS)
	g++ -shared -o $(TARGET_OUT_PATH)/lib/libesunnyconf.so $(OBJS)
	-mkdir -p $(TARGET_OUT_PATH)/esunnyconf
	-cp -p *.h $(TARGET_OUT_PATH)/esunnyconf
#	mv libesunnyconf.a $(NAUT_ROOTDIR)/public/lib
#	mv libesunnyconf.so $(NAUT_ROOTDIR)/public/lib
	
$(OBJS):	
	g++ -c $(CXXFLAGS) $(SRCS)

.PHONY : clean
clean:
	rm -f *.a *.o *.so
