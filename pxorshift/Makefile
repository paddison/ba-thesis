BUILD_DIR := build
SRC_DIR := src

poly_ntl: poly_ntl.o xorshift64.o gray.o
	cd $(BUILD_DIR)
	clang++ poly_ntl.o gray.o xorshift64.o -o ../poly_ntl -lntl -lgmp -lgf2x -lc --verbose 

poly_ntl.o:
	clang++ -c -std=c++11 $(SRC_DIR)/poly_ntl.cpp -o $(BUILD_DIR)/poly_ntl.o

xorshift64.o:
	clang -c $(SRC_DIR)/xorshift64.c -o $(BUILD_DIR)/xorshift64.o

gray.o:
	clang -c $(SRC_DIR)/gray/gray.c -o $(BUILD_DIR)/gray.o

clean:
	cd build
	rm *.o
	cd ..
	rm -r build

