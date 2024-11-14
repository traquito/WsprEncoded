all:
	g++ -std=c++17 -g -Wall test.cpp -o test

test:
	chmod +x test
	./test

clean:
	$(RM) test