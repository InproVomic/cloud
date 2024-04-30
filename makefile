cloud:cloud.cpp
	g++ -std=c++14 $^ -o $@ -L./lib -lpthread -lstdc++fs -ljsoncpp -lbundle
.PHONY:clean
clean:
		rm -f cloud
