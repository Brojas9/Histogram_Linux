#
# This Makefile will compile all source found in the "The Histogram System" directory.
#
# ====================================================================================================================
# 							The Histogram System
# ====================================================================================================================
# Other Targets

all : ./DataProducer1/bin/dp1 ./DataProducer2/bin/dp2 ./DataConsumer/bin/dc

# ====================================================================================================================
# 							Final Binary Targets
# ====================================================================================================================

./DataProducer1/bin/dp1 : ./DataProducer1/obj/dp1.o 
	cc ./DataProducer1/obj/dp1.o -o ./DataProducer1/bin/dp1 -lpthread

./DataProducer2/bin/dp2 : ./DataProducer2/obj/dp2.o 
	cc ./DataProducer2/obj/dp2.o -o ./DataProducer2/bin/dp2 -lpthread

./DataConsumer/bin/dc : ./DataConsumer/obj/dc.o 
	cc ./DataConsumer/obj/dc.o -o ./DataConsumer/bin/dc -lpthread

# ====================================================================================================================
# 							Dependencies
# ====================================================================================================================

./DataProducer1/obj/dp1.o : ./DataProducer1/src/dp1.c ./DataProducer1/inc/dp1.h ./Common/shared_defs.h
	cc -c ./DataProducer1/src/dp1.c -o ./DataProducer1/obj/dp1.o -I./Common -I./DataProducer1/inc

./DataProducer2/obj/dp2.o : ./DataProducer2/src/dp2.c ./DataProducer2/inc/dp2.h ./Common/shared_defs.h
	cc -c ./DataProducer2/src/dp2.c -o ./DataProducer2/obj/dp2.o -I./Common -I./DataProducer2/inc

./DataConsumer/obj/dc.o : ./DataConsumer/src/dc.c ./DataConsumer/inc/dc.h ./Common/shared_defs.h
	cc -c ./DataConsumer/src/dc.c -o ./DataConsumer/obj/dc.o -I./Common -I./DataConsumer/inc

# ====================================================================================================================
# Other Targets
# ====================================================================================================================

clean:
	rm -f ./DataProducer1/bin/*
	rm -f ./DataProducer1/obj/*.o
	rm -f ./DataProducer1/inc/*.h~

	rm -f ./DataProducer2/bin/*
	rm -f ./DataProducer2/obj/*.o
	rm -f ./DataProducer2/inc/*.h~

	rm -f ./DataConsumer/bin/*
	rm -f ./DataConsumer/obj/*.o
	rm -f ./DataConsumer/inc/*.h~

	rm -f ./Common/*.h~
