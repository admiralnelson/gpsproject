#include "api_controller.h"
#include "cjsoncpp.h"
#include "motor.h"
#include "solartrackingthread.h"
#include "gps.h"
#include "power_monitor.h"
#include "thread"

ApiController& ApiController::Get()
{
	static ApiController instance;
	return instance;
}

void ApiController::Register(HTTPServer& Server)
{
	Server.RegisterURI("/MoveAMax", std::bind(&ApiController::MoveAMax_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/MoveAMin", std::bind(&ApiController::MoveAMin_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/MoveBMax", std::bind(&ApiController::MoveBMax_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/MoveBMin", std::bind(&ApiController::MoveBMin_GET, this, std::placeholders::_1), HTTP_GET);

	Server.RegisterURI("/ResetAll", std::bind(&ApiController::ResetAll_GET, this, std::placeholders::_1), HTTP_GET);

	Server.RegisterURI("/StepAPos", std::bind(&ApiController::StepAPos_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/StepANeg", std::bind(&ApiController::StepANeg_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/StepBPos", std::bind(&ApiController::StepBPos_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/StepBNeg", std::bind(&ApiController::StepBNeg_GET, this, std::placeholders::_1), HTTP_GET);

	Server.RegisterURI("/OverrideStepANeg", std::bind(&ApiController::OverrideStepANeg_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/OverrideStepAPos", std::bind(&ApiController::OverrideStepAPos_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/OverrideStepBNeg", std::bind(&ApiController::OverrideStepBNeg_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/OverrideStepBPos", std::bind(&ApiController::OverrideStepBPos_GET, this, std::placeholders::_1), HTTP_GET);

	Server.RegisterURI("/OpenLoop", std::bind(&ApiController::StartOpenLoop_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/ClosedLoop", std::bind(&ApiController::StartClosedLoop_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/Stop", std::bind(&ApiController::StopLoop_GET, this, std::placeholders::_1), HTTP_GET);

	Server.RegisterURI("/Reboot", std::bind(&ApiController::RebootLoop_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/ResetCalibration", std::bind(&ApiController::ResetCalibration_GET, this, std::placeholders::_1), HTTP_GET);


	Server.RegisterURI("/Status", std::bind(&ApiController::Status_GET, this, std::placeholders::_1), HTTP_GET);
	Server.RegisterURI("/SetDate", std::bind(&ApiController::SetDate_POST, this, std::placeholders::_1), HTTP_POST);
	Server.RegisterURI("/SetCalibration", std::bind(&ApiController::SetCalibration_POST, this, std::placeholders::_1), HTTP_POST);


}

void ApiController::MoveAMax_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().SetAToDeg(20);
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::MoveAMin_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().SetAToDeg(-20);
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::MoveBMax_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().SetBToDeg(20);
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::MoveBMin_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().SetBToDeg(-20);
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::ResetAll_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().SetAToDeg(0);
		MotorController::Get().SetBToDeg(0);
	});
	t.detach();

	Request.SendReply("ok");
}

void ApiController::StepAPos_GET(HTTPRequest& Request)
{
	std::thread t([]() {	
		MotorController::Get().StepAOneDeg();
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::StepANeg_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().StepAMinusOneDeg();
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::StepBPos_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().StepBOneDeg();
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::StepBNeg_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().StepBMinusOneDeg();
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::OverrideStepANeg_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().StepABackward();
	});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::OverrideStepAPos_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().StepAForward();
		});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::OverrideStepBNeg_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().StepBBackward();
		});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::OverrideStepBPos_GET(HTTPRequest& Request)
{
	std::thread t([]() {
		MotorController::Get().StepBForward();
		});
	t.detach();
	Request.SendReply("ok");
}

void ApiController::StartOpenLoop_GET(HTTPRequest& Request)
{
	if (SolarTrackingClosedLoop::Get().IsRunning())
	{
		Request.SendError("Closed Loop still running", HTTPD_500_INTERNAL_SERVER_ERROR);
	}
	else 
	{
		SolarTrackingOpenLoop::Get().Start();
		Request.SendReply("Ok");
	}
}

void ApiController::StartClosedLoop_GET(HTTPRequest& Request)
{
	if (SolarTrackingOpenLoop::Get().IsRunning())
	{
		Request.SendError("Open Loop still running", HTTPD_500_INTERNAL_SERVER_ERROR);
	}
	else
	{
		SolarTrackingClosedLoop::Get().Start();
		Request.SendReply("Ok");
	}
}

void ApiController::StopLoop_GET(HTTPRequest& Request)
{
	SolarTrackingClosedLoop::Get().Stop();
	SolarTrackingOpenLoop::Get().Stop();
	Request.SendReply("Ok");
}

void ApiController::ResetCalibration_GET(HTTPRequest& Request)
{
	PowerMonitor::Get().ResetCalibration();
	Request.SendReply("Ok");
}

void ApiController::RebootLoop_GET(HTTPRequest& Request)
{
	Request.SendReply("Ok");
	esp_restart();
}

void ApiController::Status_GET(HTTPRequest& Request)
{
	cjsonpp::JSONObject object;
	object.set("time", GetCurrentSystemDateTimeAsString());

	object.set("degreeMotorA", MotorController::Get().GetRealTimeADeg());
	object.set("degreeMotorB", MotorController::Get().GetRealTimeBDeg());
	object.set("motorControllerStatus", (int) MotorController::Get().GetStatus());
	SolarTrackingClosedLoop::LdrArray LdrArrays = SolarTrackingClosedLoop::Get().GetArrayLdr();
	object.set("ldrn", LdrArrays.N);
	object.set("ldrw", LdrArrays.W);
	object.set("ldrs", LdrArrays.S);
	object.set("ldre", LdrArrays.E);
	object.set("long", Gps::Get().GetLon());
	object.set("lat", Gps::Get().GetLat());
	object.set("alt", Gps::Get().GetAltitute());
	object.set("azi", SolarTrackingOpenLoop::Get().GetAzimuth());
	object.set("elev", SolarTrackingOpenLoop::Get().GetElevation());
	object.set("volt", PowerMonitor::Get().Voltage());
	object.set("current", PowerMonitor::Get().Current());
	object.set("power", PowerMonitor::Get().Power());

	object.set("openlooprunning", SolarTrackingOpenLoop::Get().IsRunning());
	object.set("closedlooprunning", SolarTrackingClosedLoop::Get().IsRunning());

	Request.SetContentType("application/json");
	Request.SendReply(object.print());
}

void ApiController::SetDate_POST(HTTPRequest& Request)
{
	std::string data = Request.Body;
	SetDateAndTime(data);
	cjsonpp::JSONObject object;
	object.set("result", true);
	object.set("reason", "date time is set");
	object.set("dateTime", GetCurrentSystemDateTimeAsString());

	Request.SetContentType("application/json");
	Request.SendReply(object.print());
}

void ApiController::SetCalibration_POST(HTTPRequest& Request)
{
	std::string data = Request.Body;
	PowerMonitor::CalibrationData calibration;
	if (!calibration.FromJson(data))
	{
		Request.SendError("malformed json input", HTTPD_400_BAD_REQUEST);
	}
	else
	{
		Request.SendReply("calibration is set");
	}
}

ApiController::ApiController() : IControllerBase()
{

}
