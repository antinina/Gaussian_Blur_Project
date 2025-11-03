.PHONY: all
all: final_app
final_app: final_app.o
	gcc -o final_app final_app.o -lm
final_app.o: final_app.c
	gcc -c final_app.c 
.PHONY: clean
clean:
	rm final_app.o final_app
.PHONY: run
run:
	./final_app
