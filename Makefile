default:
	gcc main.c thread_serve.c -lpthread -o wrench

run:
	./wrench -r .
