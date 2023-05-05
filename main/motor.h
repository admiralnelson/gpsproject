#pragma once
#include "pins.h"
#include "stdint.h"
#include "memory"

const uint32_t PIN_ENABLE_FORWARD = 33;
const uint32_t PIN_ENABLE_BACKWARD = 25;
const uint32_t PIN_CHANNEL_A_FORWARD = 26;
const uint32_t PIN_CHANNEL_A_BACKWARD = 27;
const uint32_t PIN_CHANNEL_B_FORWARD = 14;
const uint32_t PIN_CHANNEL_B_BACKWARD = 12;
const int FREQUENCY = 25000;

const int B_FORWARD_TIME = 73;
const int B_BACKWARD_TIME = 70;

const int A_MAX_FORWARD_RANGE_DEG = 20;
const int A_MAX_BACKWARD_RANGE_DEG = 20;

const int B_MAX_FORWARD_RANGE_DEG = 20;
const int B_MAX_BACKWARD_RANGE_DEG = 25;




class MotorController {
	enum class MotorMovement {
		Stopped,
		MotorAMovingPosDeg,
		MotorAMovingNegDeg,
		MotorBMovingPosDeg,
		MotorBMovingNegDeg,
		Resetting
	};

public:
	static MotorController& Get();

	void SetAToDeg(int degrees);
	void SetBToDeg(int degrees);

	void StepAByDeg(int howManyDegrees);

	void StepAOneDeg();
	void StepAMinusOneDeg();
	void StepBOneDeg();
	void StepBMinusOneDeg();


	void StepAForward();
	void StepABackward();
	void StepBForward();
	void StepBBackward();

	void DebugClearMemory();


private:
	void ResetA(int newDegree);

	MotorController();
	MotorController(MotorController const&) = delete;
	void operator=(MotorController const&) = delete;
	
	std::unique_ptr<PWMPin> MotorAForward;
	std::unique_ptr<PWMPin> MotorBForward;

	std::unique_ptr<PWMPin> MotorABackward;
	std::unique_ptr<PWMPin> MotorBBackward;

	MotorMovement MotorMovementStatus = MotorMovement::Stopped;

	int CurrentMotorADegree = 0;
	int CurrentMotorBDegree = 0;
};