#pragma once
#include "common.h"
#include <set>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include "esp_http_server.h"
#include "http_parser.h"

#define MAX_URL_LENGTH 75
#define MAX_BODY_LENGTH 500 * 2
#define MAX_BODY_SAVE_TO_FILE 1000 * 1000
#define MAX_AUTH_LENGTH 200
typedef std::map<std::string, std::string> MapQueryValue;
typedef httpd_method_t HTTPMethod;
typedef httpd_err_code_t HTTPErrorCode;
typedef httpd_handle_t HTTPHandle;

class HTTPServer;

struct HTTPRequest
{
	std::string RemoteIp;
	std::string Uri;
	std::string Body;
	std::string Authorisation;
	MapQueryValue Queries;
	HTTPMethod Method;
	void SendReply(const std::stringstream& Content) const;
	void SendReply(const std::string& Content) const;
	void SendReplyByChunks(const std::string& Content) const;
	void SendError(const std::string& Content, HTTPErrorCode Code) const;
	void SetContentType(const char* ContentType) const;
	httpd_req_t* HttpdRequestInternal;
	HTTPServer* HttpServerInternal;
};

typedef std::function<void(HTTPRequest&)> HTTPRequestCallback;


class HTTPServer
{
public:
	HTTPServer(uint Port = 80, uint MaxHandler = 15);
	~HTTPServer();

	void RegisterURI(const std::string& Uri, HTTPRequestCallback Handler, HTTPMethod Method = HTTP_GET, bool bIsSecure = false);
	void RegisterAuthorisedURI(const std::string& Uri, HTTPRequestCallback Handler, HTTPMethod Method = HTTP_GET);
	/*
		Call this only inside HTTPRequestCallback
		and only be called ONCE. this will finalise the HTTP request.
	*/
	void SendReply(const std::stringstream& Content, const HTTPRequest& Request);
	void SendReply(const std::string& Content, const HTTPRequest& Request);
	void SendReplyByChunks(const std::string& Content, const HTTPRequest& Request);
	void SendError(const std::string& Content, const HTTPRequest& Request, HTTPErrorCode Code);

	void SetContentType(const char* Type, const HTTPRequest& Request);

	bool IsHandlerExisted(const char* Uri);
	/*
		Returns -1 if not found;
	*/
	int GetHandlerIndex(const char* Uri);
	/*
	*   Handled internally!
		Do not use!
	*/
	void HandleRequest(const char* Uri, const HTTPRequest& Request);


private:

	struct InternalHandler
	{
		std::string Uri;
		HTTPRequestCallback Callback;
		bool bAuthorised;
	};

	std::vector<InternalHandler> HandlerCollections;
	httpd_handle_t Server = nullptr;
	uint Port;
	uint MaxHandler;
	bool bIsServerStarted = false;

private:
	HTTPServer() = delete;
	HTTPServer(HTTPServer const&) = delete;
	void operator=(HTTPServer const&) = delete;

	static std::set<uint> RegisteredPorts;
};