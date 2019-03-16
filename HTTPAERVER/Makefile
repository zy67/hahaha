.PHONY:all
all:HttpServer upload

HttpServer:HttpServer.cc
	g++ -o $@ $^ -pthread -std=c++11
upload:upload.cc
	g++ -o $@ $^ -pthread -std=c++11

.PHONY:clean
clean:
	rm -f HttpServer upload
