#NAME: Rohan Rao
#EMAIL: raokrohan@gmail.com
#ID: 305339928

lab4c: lab4c_tcp.c lab4c_tls.c
	@gcc -g lab4c_tcp.c -Wall -Wextra -lmraa -lm -o lab4c_tcp
	@chmod 777 lab4c_tcp

	@gcc -g lab4c_tls.c -Wall -Wextra -lmraa -lm -lssl -lcrypto -o lab4c_tls
	@chmod 777 lab4c_tls

dist: lab4c_tcp.c lab4c_tls.c README Makefile
	@tar -cvzf lab4c-305339928.tar.gz README Makefile lab4c_tcp.c lab4c_tls.c
	@chmod 777 lab4c-305339928.tar.gz

check:
	@make
	@{ echo "STOP"; echo "PERIOD=2"; echo "START"; sleep 6; echo "STOP"; echo "PERIOD=1"; echo "SCALE=F"; echo "START"; sleep 5; echo "OFF"; } | ./lab4c_tcp --log=tempfile.txt --period=1 --scale=C
	@rm -f tempfile.txt

sanity: lab4c-305339928.tar.gz
	@./sanity.sh 305339928 
.PHONY: clean
clean: 
	@rm -f lab4c-305339928.tar.gz lab4c_tcp lab4c_tls buf9928.txt