all: ip_addrs

ip_addrs: main.c 
	gcc -Wall -Werror main.c -o ip_addrs
run: ip_addrs 
	./ip_addrs
clean: 
	rm -f ./ip_addrs
