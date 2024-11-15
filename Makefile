all:
	g++ -std=c++17 -g -Werror -Wall -Wextra test.cpp -o test

test:
	chmod +x test
	./test

clean:
	$(RM) test