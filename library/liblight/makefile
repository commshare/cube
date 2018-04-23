#makefile for liblight
#OBJECTS = Buffer.o transaction.o session.o server.o clientapi.o business_unit.o dispatcher.o business_unit_group.o light_service.o business_unit_declare.o
#SRCS = Buffer.cpp transaction.cpp session.cpp  server.cpp clientapi.cpp business_unit.cpp dispatcher.cpp business_unit_group.cpp light_service.cpp business_unit_declare.cpp

SRCDIRS := .
SRCEXTS := .cpp
SOURCES := $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
OBJS := $(patsubst %.cpp,./obj/%.o,$(notdir $(SOURCES)))

CFLAGS = -fPIC -Wall -O3 -std=c++0x
INC = -I../../thirdparty/boost_1_65_0 -I../../public/ -L../../public/lib/ -llog 
all : $(OBJS)  
	ar r liblight.a $(OBJS)
	g++ -shared -o liblight.so $(OBJS)
	-cp -p liblight.a ../../public/lib
#	-cp -p liblight.so ../../public/lib
	-cp -p *.h ../../public/liblight
$(OBJS) :
./obj/%.o : %.cpp 
	@mkdir -p $(dir $@)
#	@echo liblight compile begin...
	@echo \#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#
#	g++ -c $(CFLAGS) $(INC) $(SOURCES)  
	g++ -c $< -o $@ $(CFLAGS) $(INC)
	@echo \#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#\#
#	@echo liblight compile end...
.PHONY : clean

clean:
	rm -f liblight.a $(OBJS)
