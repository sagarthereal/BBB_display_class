program: main.cpp display.cpp
	g++ main.cpp display.cpp -o program -ldrm -I /usr/include/libdrm/ -I /usr/include/cairo -lcairo
clean:
	rm -r *.o
