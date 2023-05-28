#include "webserver.h"
#include <esp_http_server.h>
#include <memory>
#include <string>
#include <algorithm> 
#include "esp_err.h"
#include "esp_log.h"
#include <sys/socket.h>


#define TAG "webserver"

std::set<uint> HTTPServer::RegisteredPorts;

void SetRemoteIP(HTTPRequest &Request)
{
    char RemoteIp[IP6ADDR_STRLEN_MAX];
    memset(RemoteIp, 0, sizeof(RemoteIp));
    int sockfd = httpd_req_to_sockfd(Request.HttpdRequestInternal);
    socklen_t socklen;
    
    struct sockaddr_storage client_addr;
    socklen = sizeof(client_addr);
    int port = 0;
    if (getpeername(sockfd, (struct sockaddr*)&client_addr, &socklen) < 0) {
        ESP_LOGE(TAG, "Error getting client IP");
        memset(RemoteIp, 0, sizeof(RemoteIp));
        return;
    }

    // Convert to IPv6 string
    //inet_ntop(AF_INET, &addr.sin6_addr, ipstr, sizeof(ipstr));
    if (client_addr.ss_family == AF_INET) 
    {
        struct sockaddr_in* s = (struct sockaddr_in*)&client_addr;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, RemoteIp, sizeof(RemoteIp));
    }
    else if (client_addr.ss_family == AF_INET6) 
    {
        struct sockaddr_in6* s = (struct sockaddr_in6*)&client_addr;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, RemoteIp, sizeof(RemoteIp));
    }
    Request.RemoteIp = RemoteIp;
    
}


void ExtractQueryFromURI(char* QueryStrings, MapQueryValue &Results)
{
    ESP_LOGI(TAG, "Found URL query => %s", QueryStrings);

    Results.clear();
    std::istringstream iss(QueryStrings);

    std::string keyval, key, val;

    while (std::getline(iss, keyval, '&')) // split each term
    {
        std::istringstream iss(keyval);

        // split key/value pairs
        if (std::getline(std::getline(iss, key, '='), val))
        {
            Results[key] = val;
        }
    }

    ESP_LOGI(TAG, "processed %d query KVs", Results.size());
}

static esp_err_t HTTPHandler(httpd_req_t* req)
{
    std::shared_ptr<char> buffer;
    int bufferLength;
    HTTPRequest Request;

    assert(req->user_ctx != nullptr);
    HTTPServer* Server = (HTTPServer*)req->user_ctx;

    ESP_LOGI(TAG, "handling request: %s", req->uri);

    std::string uri = req->uri;
    
    size_t pos = uri.find('?');
    if (pos != std::string::npos)
    {
        uri = uri.substr(0, pos);
    }

    /* Read URL query string length and allocate memory for length + 1,
    * extra byte for null termination */
    bufferLength = httpd_req_get_url_query_len(req) + 1;
    
    if (bufferLength < MAX_URL_LENGTH)
    {
        if (bufferLength > 1) 
        {
            buffer.reset(new char[bufferLength]);
            if (httpd_req_get_url_query_str(req, buffer.get(), bufferLength) == ESP_OK)
            {
                ExtractQueryFromURI(buffer.get(), Request.Queries);
            }
        }
    }
    else
    {
        ESP_LOGW(TAG, "url request too long, skipping parsing");
    }

    if (req->content_len > MAX_BODY_LENGTH)
    {
        ESP_LOGE(TAG, "fatal, content too long");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "content too long");
        return ESP_ERR_HTTPD_ALLOC_MEM;
    }

    int received = 0;
    int remaining = req->content_len;
    bufferLength = remaining + 10;
    buffer.reset(new char[bufferLength]);
    while (remaining > 0) 
    {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        received = httpd_req_recv(req, buffer.get(), std::min(remaining, bufferLength));
        if (received <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry if timeout occurred */
                continue;
            }

            ESP_LOGE(TAG, "Content reception failed!");
        }

        /* Write buffer content to file on storage */
        if (received > 0) 
        {
            Request.Body.append(buffer.get(), received);
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
        memset(buffer.get(), 0, sizeof(char) * bufferLength);
    }


    Request.HttpdRequestInternal = req;
    Request.HttpServerInternal = Server;
    SetRemoteIP(Request);
    
    ESP_LOGI(TAG, "Client IP6 => %s", Request.RemoteIp.data());
    bufferLength = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (bufferLength < MAX_AUTH_LENGTH)
    {
        if (bufferLength > 1)
        {
            buffer.reset(new char[bufferLength]);
            if (httpd_req_get_hdr_value_str(req, "Authorization", buffer.get(), bufferLength) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found header => Authorization: %s", buffer.get());
            }
            Request.Authorisation = buffer.get();
        }
    }
    else
    {
        ESP_LOGW(TAG, "authorization header is too long");
    }
   
    Server->HandleRequest(uri.data(), Request);

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");


    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) 
    {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

HTTPServer::HTTPServer(uint Port, uint MaxHandler) : 
    Port(Port), MaxHandler(MaxHandler)
{
	if (RegisteredPorts.find(Port) != RegisteredPorts.end())
	{
		ESP_LOGE(TAG, "Port has been registered!");
		return;
	}

    httpd_config_t Config = HTTPD_DEFAULT_CONFIG();
    Config.lru_purge_enable = true;
    Config.server_port = Port;
    Config.max_uri_handlers = MaxHandler;

    ESP_LOGI(TAG, "Starting server on port: '%d'", Config.server_port);
    if (httpd_start(&Server, &Config) == ESP_OK) 
    {
        bIsServerStarted = true;
        RegisteredPorts.insert(Port);
        ESP_LOGI(TAG, "HTTP has been started");

    }
    else
    {
        ESP_LOGE(TAG, "Error starting server!");
    }

    HandlerCollections.reserve(Config.max_uri_handlers);

}

HTTPServer::~HTTPServer()
{
    if (bIsServerStarted)
    {
        httpd_stop(Server);
	    RegisteredPorts.erase(Port);
    }
}

void HTTPServer::RegisterURI(const std::string& Uri, HTTPRequestCallback Handler, HTTPMethod Method, bool bIsSecure)
{
    if (HandlerCollections.size() == MaxHandler)
    {
        ESP_LOGE(TAG, "Too many handlers! Max %d", MaxHandler);
        return;
    }

    if (!bIsServerStarted)
    {
        ESP_LOGE(TAG, "Server is not started!");
        return;
    }

    if (IsHandlerExisted(Uri.data()))
    {
        ESP_LOGE(TAG, "Uri %s has been registered!", Uri.data());
        return;
    }

    size_t pos = Uri.find('?');
    if (pos != std::string::npos)
    {
        ESP_LOGE(TAG, "Uri %s contains ilegal char ? (question mark)", Uri.data());
        return;
    }

    httpd_uri_t Request = {
        .uri = Uri.c_str(),
        .method = Method,
        .handler = HTTPHandler,
        /* Let's pass response string in user
         * context to demonstrate it's usage */
        .user_ctx = this
    };
    InternalHandler IHandler = {Uri, Handler, bIsSecure };
    HandlerCollections.push_back(IHandler);
    
    //ESP_LOGI(TAG, "registering handler with uri: %s, method: %d", Uri.data(), (int)Method);

    ESP_ERROR_CHECK(httpd_register_uri_handler(Server, &Request));
}

void HTTPServer::RegisterAuthorisedURI(const std::string& Uri, HTTPRequestCallback Handler, HTTPMethod Method)
{
    RegisterURI(Uri, Handler, Method, true);
}

void HTTPServer::SendReply(const std::stringstream& Content, const HTTPRequest& Request)
{
    assert(Request.HttpdRequestInternal != nullptr);
    if (Request.HttpdRequestInternal == nullptr)
    {
        ESP_LOGE(TAG, "Request.HttpdRequestInternal is null. this method uses httpd and requires pointer to the request");
        return;
    }
    const std::string& tmp = Content.str();
    httpd_resp_send(Request.HttpdRequestInternal, tmp.data(), HTTPD_RESP_USE_STRLEN);
}

void HTTPServer::SendReply(const std::string& Content, const HTTPRequest& Request)
{
    SendReplyByChunks(Content, Request);
    SendReplyByChunks(std::string(), Request);
}

void HTTPServer::SendReplyByChunks(const std::string& Content, const HTTPRequest& Request)
{
    assert(Request.HttpdRequestInternal != nullptr);
    if (Request.HttpdRequestInternal == nullptr)
    {
        ESP_LOGE(TAG, "Request.HttpdRequestInternal is null. this method uses httpd and requires pointer to the request");
        return;
    }
    if (Content.length() > 0)
    {
        httpd_resp_sendstr_chunk(Request.HttpdRequestInternal, Content.data());
    }
    else
    {
        httpd_resp_sendstr_chunk(Request.HttpdRequestInternal, nullptr);
    }
}

void HTTPServer::SendError(const std::string& Content, const HTTPRequest& Request, HTTPErrorCode Code)
{
    assert(Request.HttpdRequestInternal != nullptr);
    if (Request.HttpdRequestInternal == nullptr)
    {
        ESP_LOGE(TAG, "Request.HttpdRequestInternal is null. this method uses httpd and requires pointer to the request");
        return;
    }
    httpd_resp_send_err(Request.HttpdRequestInternal, Code, Content.data());
}

void HTTPServer::SetContentType(const char* Type, const HTTPRequest& Request)
{
    httpd_resp_set_type(Request.HttpdRequestInternal,  Type);
}


bool HTTPServer::IsHandlerExisted(const char* Uri)
{
    return GetHandlerIndex(Uri) != -1;
}

int HTTPServer::GetHandlerIndex(const char* Uri)
{
    for (int i = 0; i < HandlerCollections.size(); i++)
    {
        if (HandlerCollections[i].Uri == Uri)
        {
            return i;
        }
    }
    return -1;
}

void HTTPServer::HandleRequest(const char* Uri, const HTTPRequest& Request)
{
    std::string uri = Uri;
    int HandlerIndex = GetHandlerIndex(Uri);
    if (HandlerIndex != -1)
    {
        HTTPRequest Req = Request;
        if (HandlerCollections[HandlerIndex].bAuthorised)
        {
            if (true)
            {
                HandlerCollections[HandlerIndex].Callback(Req);
            }
            else
            {
                Request.SendError("access denied", HTTPD_401_UNAUTHORIZED);;
            }
        }
        else
        {
            HandlerCollections[HandlerIndex].Callback(Req);
        }
    }
    else
    {
        ESP_LOGW(TAG, "Unhandled request %s", Uri);
    }
}

void HTTPRequest::SendReply(const std::stringstream& Content) const
{
    HttpServerInternal->SendReply(Content, *this);
}

void HTTPRequest::SendReply(const std::string& Content) const
{
    HttpServerInternal->SendReply(Content, *this);
}

void HTTPRequest::SendReplyByChunks(const std::string& Content) const
{
    HttpServerInternal->SendReplyByChunks(Content, *this);
}

void HTTPRequest::SendError(const std::string& Content,  HTTPErrorCode Code) const
{
    HttpServerInternal->SendError(Content, *this, Code);
}

void HTTPRequest::SetContentType(const char* ContentType) const
{
    HttpServerInternal->SetContentType(ContentType, *this);
}