#pragma once
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define szDEF 2<<20

static int fd = -1;

class RandomBuffer
{
private:
	uint8_t* buffer;
	uint32_t szRNG;
	uint32_t szRDY;
	uint8_t* head;
	const char* source;
	int (*_backend)(void);
public:
	RandomBuffer(uint32_t szBuff=szDEF);
	~RandomBuffer();
	void LoadRandom(uint32_t bytes);
	void LoadMax();
	void LoadFromBuffer(uint8_t* buffer, uint32_t len);
	uint64_t GetQWORD();
	uint32_t GetDWORD();
	void GetBytes(void* to, size_t len);
	void print() {
		uint8_t* tmp = head;
		uint32_t ssz = szRDY;
			while (ssz--) {
				printf("%.2x", *tmp);
				tmp = (uint8_t*)(buffer + ((size_t)tmp - (size_t)buffer + 1) % szRNG);
		}
		putchar('\n');
	};
	uint32_t GetRdy() { return this->szRDY; }
	uint32_t GetSz() { return this->szRNG; }
};

