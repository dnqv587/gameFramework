#pragma once
#include <string>

class Base64
{
public:
	Base64()=default;

	~Base64();

	std::string encode(const std::string& data);//±àÂë

	std::string decode( std::string& data);//½âÂë

private:
	int b64index(uint8_t c);


};