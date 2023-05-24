#include "memory"
#include "common.h"
#include "sunpath.h"

class SolarTrackingOpenLoop
{
public:
	static SolarTrackingOpenLoop& Get();
	bool Start();
	bool Stop();
	bool IsRunning();

	void Update();

private:
	bool blsRunning = false;

	SolarTrackingOpenLoop();
	SolarTrackingOpenLoop(SolarTrackingOpenLoop const&) = delete;

};


class SolarTrackingClosedLoop
{
public:
	static SolarTrackingClosedLoop& Get();
	bool Start();
	bool Stop();
	bool IsRunning();

};

