#include "webcontroller_base.h"

class PagesController : public IControllerBase
{
public:
	static PagesController& Get();
	void Register(HTTPServer& Server) override;

private:
	// /index, /, /index.htm GET
	void Index_GET(HTTPRequest& Request);
	// /style.css GET
	void StyleCSS_GET(HTTPRequest& Request);
	// /app.js GET
	void AppJS_GET(HTTPRequest& Request);

private:
	PagesController();
	PagesController(PagesController const&) = delete;
	void operator=(PagesController const&) = delete;
};