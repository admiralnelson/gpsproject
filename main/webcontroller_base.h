#pragma once
#include "webserver.h"

class IControllerBase
{
public:
	virtual void Register(HTTPServer& Server) = 0;
};