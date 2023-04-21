#pragma once
#include <inttypes.h>
#include "string"



class Persistence {
public:
	struct IntReturn {
		int Result = 0;
		bool Existed = false;
	};
public:
	static Persistence& Get();

	void WriteInt(const std::string& name, int integer);
	IntReturn GetInt(const std::string& name);

private:
	Persistence();
	Persistence(Persistence const&) = delete;
	void operator=(Persistence const&) = delete;

	uint32_t NVSHandleNr = 0;
};