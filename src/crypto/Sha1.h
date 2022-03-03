#pragma once
#include <string>

typedef struct {
	uint32_t state[5];
	uint32_t count[2];
	uint8_t  buffer[64];
} SHA1_CTX;

class Sha1
{
public:
	Sha1();
	~Sha1();

	void Update(std::string data);

	std::string Final();

public:
	SHA1_CTX m_sha1;
};