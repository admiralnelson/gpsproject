#include "webcontroller_base.h"

class ApiController : public IControllerBase
{
public:
	static ApiController& Get();
	virtual void Register(HTTPServer& Server) override;

private:
	void MoveAMax_GET(HTTPRequest& Request);
	void MoveAMin_GET(HTTPRequest& Request);
	void MoveBMax_GET(HTTPRequest& Request);
	void MoveBMin_GET(HTTPRequest& Request);

	void ResetAll_GET(HTTPRequest& Request);

	void StepAPos_GET(HTTPRequest& Request);
	void StepANeg_GET(HTTPRequest& Request);
	void StepBPos_GET(HTTPRequest& Request);
	void StepBNeg_GET(HTTPRequest& Request);

	void OverrideStepANeg_GET(HTTPRequest& Request);
	void OverrideStepAPos_GET(HTTPRequest& Request);
	void OverrideStepBNeg_GET(HTTPRequest& Request);
	void OverrideStepBPos_GET(HTTPRequest& Request);


	void StartOpenLoop_GET(HTTPRequest& Request);
	void StartClosedLoop_GET(HTTPRequest& Request);
	void StopLoop_GET(HTTPRequest& Request);
	void ResetCalibration_GET(HTTPRequest& Request);
	void RebootLoop_GET(HTTPRequest& Request);



	void Status_GET(HTTPRequest& Request);
	void SetDate_POST(HTTPRequest& Request);
	void SetCalibration_POST(HTTPRequest& Request);

	void DownloadPowerLog_GET(HTTPRequest& Request);
	void DownloadLdrLog_GET(HTTPRequest& Request);
	void ClearLog_GET(HTTPRequest& Request);

	
private:
	ApiController();
	ApiController(ApiController const&) = delete;
	void operator=(ApiController const&) = delete;
};