/*
 * block_ram.c
 *
 *  Created on: Mar 30, 2014
 *      Author: DGronlund
 */

#include "block_ram.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

static int block_ram_created = -1;
static void *mapped_base, *mapped_dev_base;

int block_ram_init() {
	if (block_ram_created != -1) {
		printf("BLOCK RAM device already created\n");
		return BLOCK_RAM_FAILURE;
	}
	block_ram_created = 0;

	off_t dev_base = BLOCK_RAM_BASE_ADDRESS;

	int memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		return BLOCK_RAM_FAILURE;
	} else {
		printf("/dev/mem opened.\n");
	}

	// Map one page of memory into user space such that the device is in that page, but it may not
	// be at the start of the page.
	mapped_base = mmap(0, BLOCK_RAM_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base & ~BLOCK_RAM_MAP_MASK);
	if (mapped_base == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}
	printf("Memory mapped at address %p.\n", mapped_base);

	// get the address of the device in user space which will be an offset from the base
	// that was mapped as memory is mapped at the start of a page

	mapped_dev_base = mapped_base + (dev_base & BLOCK_RAM_MAP_MASK);

	close(memfd);

	return BLOCK_RAM_SUCCESS;
}

void write_register(int address, int value) {
	*((volatile unsigned long *) (mapped_dev_base + address)) = value;
}

int read_register(int address) {
	return *((volatile unsigned long *) (mapped_dev_base + address));
}

int block_ram_close() {
	if (munmap(mapped_base, BLOCK_RAM_MAP_SIZE) == -1) { // unmap the memory before exiting
		printf("Can't unmap memory from user space.\n");
		return BLOCK_RAM_FAILURE;
	}
	return BLOCK_RAM_SUCCESS;
}
