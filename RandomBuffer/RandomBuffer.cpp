#include "RandomBuffer.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


int _internal() {
	int tmp;
	if (fd == -1)
		return -1;
	if (read(fd, &tmp, 2))  // can read up to 4 bytes
		return tmp;
	else
		return -1;
}

RandomBuffer::RandomBuffer(uint32_t szBuff)
{
	this->buffer = (uint8_t*)malloc(szBuff);
	if (!this->buffer) {
		perror("ERROR! malloc failed\n");
		exit(2);
	}
	this->head = buffer+szBuff;
	this->szRNG = szBuff;
	this->szRDY = 0;
	// random-related stuff
	//srand(time(NULL));
	//_backend = rand;
	source = "/dev/urandom";
	if (fd == -1)
	{
		fd = open("/dev/urandom", O_RDONLY);
	}
	if (fd == -1)
	{
		printf("Failed to open backend source %s\n", source);
		exit(1);
	}
	_backend = _internal;
}


RandomBuffer::~RandomBuffer()
{
	if (buffer) {
		free(buffer);
		buffer = nullptr;
	}
	this->head = nullptr;
}

void RandomBuffer::LoadRandom(uint32_t bytes)
{
	if (szRDY + bytes > szRNG) {
		// can't load so many random bytes
		bytes = szRNG - szRDY;
	}
	head = (uint8_t*)(buffer + ((size_t)head - (size_t)buffer - bytes) % szRNG);
	if (read(fd, head, bytes) == -1)
		exit(12);
	szRDY += bytes;
}
void RandomBuffer::LoadMax() {
	this->LoadRandom(this->szRNG);
}
void RandomBuffer::LoadFromBuffer(uint8_t* extBuffer, uint32_t len)
{
	if (szRDY + len > szRNG) {
		len = szRNG - szRDY;
	}
	for (uint32_t i = 0; i < len; i++)
	{
		head = (uint8_t*)(buffer + ((size_t)head - (size_t)buffer - 1) % szRNG);
		*head = *(extBuffer++);
		
	}
	szRDY += len;
}

uint64_t RandomBuffer::GetQWORD()
{
	uint64_t tmp;
	if (szRDY < sizeof(tmp))
		this->LoadRandom(sizeof(tmp) - szRDY);
	tmp = *((uint64_t*)head);
	head = (uint8_t*)(buffer + ((size_t)head - (size_t)buffer + sizeof(tmp)) % szRNG);
	szRDY -= sizeof(tmp);
	return tmp;
}

uint32_t RandomBuffer::GetDWORD()
{
	uint32_t tmp;
	if (szRDY < sizeof(tmp))
		this->LoadRandom(sizeof(tmp) - szRDY);
	tmp = *((uint32_t*)head);
	head = (uint8_t*)(buffer + ((size_t)head - (size_t)buffer + sizeof(tmp)) % szRNG);
	szRDY -= sizeof(tmp);
	return tmp;
}

void RandomBuffer::GetBytes(void* to, size_t len)
{
	if (len > szRDY) {
		// not enough random bytes ready - get more
		this->LoadRandom(len - szRDY);
	}
	memcpy(to, head, len);
	head = (uint8_t*)(buffer + ((size_t)head - (size_t)buffer + len) % szRNG);
	szRDY -= len;
}
