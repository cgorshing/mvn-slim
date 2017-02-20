default:
	gcc -std=c11 main.c thread_serve.c -lpthread -o wrench

run:
	./wrench -r .
