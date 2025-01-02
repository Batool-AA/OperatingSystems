build-server:
	gcc server.c -o server -lssl -lcrypto

build-client:
	gcc client.c -o client -lssl -lcrypto

run-client:
	@echo "Enter File Name and Number of Segments (separated by space)"
	@read FILE SEG; \
	./client $$FILE $$SEG

run-server:
	./server

run-tests:
	bash tests.sh

clean:
	rm -f server client *_received.*


