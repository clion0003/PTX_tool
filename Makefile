.PHONY : clean
LibAIGPUGen: src/main.cpp src/caffe.pb.cc src/PTX_tool.cpp
	g++ src/PTX_tool.cpp src/main.cpp src/caffe.pb.cc -I./include -I$(HOME)/local/include -std=c++11 -Wall -O2 -L$(HOME)/local/lib -lprotobuf -lmstch -lboost_filesystem -lboost_system -o PTXGen.bin 
clean:
	-rm *.bin
