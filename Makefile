myShell: step2.cpp
	g++ -o myShell -ggdb3 -Wall -Werror -pedantic -std=gnu++98 step2.cpp
test: step1.cpp
	g++ -o test -ggdb3 -Wall -Werror -pedantic -std=gnu++98 step1.cpp
.PHONY: clean
clean:
	rm -f myShell test *.o *~
