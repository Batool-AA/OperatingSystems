build:
	gcc paging.c -o paging

run:
	@echo "Enter Physical Memory Size in B, Logical Address Size in bits, Page Size, Process Files (separated by space)"
	@read PHY_MEM LOG_ADD PAGE_SIZE FILES; \
	./paging $$PHY_MEM $$LOG_ADD $$PAGE_SIZE $$FILES

clean:
	rm -f paging
