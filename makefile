all: ip_addrs

ip_addrs: main.c 
	gcc -Wall -Werror -g main.c -o ip_addrs
run: ip_addrs 
	./ip_addrs
leak: ip_addrs
	valgrind -s --leak-check=full ./ip_addrs
clean: 
	rm -f ./ip_addrs
