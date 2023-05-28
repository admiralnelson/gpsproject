#include "pages_controller.h"
#include "filesystem.h"

PagesController& PagesController::Get()
{
	static PagesController instance;
	return instance;
}

void PagesController::Register(HTTPServer& Server)
{
	Server.RegisterURI("/", std::bind(&PagesController::Index_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/index", std::bind(&PagesController::Index_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/index.htm", std::bind(&PagesController::Index_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/style.css", std::bind(&PagesController::StyleCSS_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/script.js", std::bind(&PagesController::AppJS_GET, this, std::placeholders::_1), HTTP_GET);
}

void PagesController::Index_GET(HTTPRequest& Request)
{
	std::string Page;
	if (FileSystem::Get().LoadFile("/data/index.htm", Page))
	{
		Request.SendReply(Page);
	}
	else
	{
		Request.SendError("cannot load file index.htm, please send the device to the manufacturer for repair", HTTPD_404_NOT_FOUND);
	}
}

void PagesController::StyleCSS_GET(HTTPRequest& Request)
{
	std::string Page;
	if (FileSystem::Get().LoadFile("/data/style.css", Page))
	{
		Request.SetContentType("text/css;");
		Request.SendReply(Page);
	}
	else
	{
		Request.SendError("cannot load file style.css, please send the device to the manufacturer for repair", HTTPD_404_NOT_FOUND);
	}
}

void PagesController::AppJS_GET(HTTPRequest& Request)
{
	std::string Page;
	if (FileSystem::Get().LoadFile("/data/script.js", Page))
	{
		Request.SetContentType("application/javascript;");
		Request.SendReply(Page);
	}
	else
	{
		Request.SendError("cannot load file app.js, please send the device to the manufacturer for repair", HTTPD_404_NOT_FOUND);
	}
}

PagesController::PagesController()
{
}