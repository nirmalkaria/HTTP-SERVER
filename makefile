all:
	gcc -pthread -o http http.c


clean:
	rm -r http
