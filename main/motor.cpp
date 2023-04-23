#include "motor.h"
#include "common.h"
#include "esp_log.h"
#include "persistence.h"


const char* PERSISTENCE_MOTOR_A_DEG = "MOTORA_DEG";
const char* PERSISTENCE_MOTOR_B_DEG = "MOTORB_DEG";
const char* MOTOR_TAG = "MotorController";

MotorController& MotorController::Get()
{
    static MotorController instance;
    return instance;
}



void MotorController::SetAToDeg(int degrees)
{
    if (degrees == 0 && this->CurrentMotorADegree == 0)
    {
        ESP_LOGI(MOTOR_TAG, "motor a already at 0");
        return;
    }

    if (degrees >= 0 && degrees >= A_MAX_FORWARD_RANGE_DEG)
    {
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "set degree to %d, will move by this delta %d", degrees, delta);
        this->StepAByDeg(delta);
        return;
    }
    
    if (degrees <= 0 && degrees <= -A_MAX_BACKWARD_RANGE_DEG)
    {
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "set degree to %d, will move by this delta %d", degrees, delta);
        this->StepAByDeg(delta);
        return;
    }

    if (degrees > 0 && this->CurrentMotorADegree < 0)
    {
        ESP_LOGI(MOTOR_TAG, "reset to 0 by this many degrees %d (forward)", this->CurrentMotorADegree);
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "set degree to %d, will move by this delta %d", degrees, delta);
        this->StepAByDeg(delta);
        ESP_LOGI(MOTOR_TAG, "step to %d degree", degrees);
        this->StepAByDeg(degrees);
        this->CurrentMotorADegree = degrees;
        return;
    }

    if (degrees < 0 && this->CurrentMotorADegree > 0)
    {
        ESP_LOGI(MOTOR_TAG, "reset to 0 by this many degrees %d (backward)", this->CurrentMotorADegree);
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "set degree to %d, will move by this delta %d", degrees, delta);
        this->StepAByDeg(delta);
        ESP_LOGI(MOTOR_TAG, "step to %d degree", degrees);
        this->StepAByDeg(degrees);
        this->CurrentMotorADegree = degrees;
        return;
    }

    if (degrees < this->CurrentMotorADegree)
    {
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "going backward this many degrees %d", delta);
        ESP_LOGI(MOTOR_TAG, "set degree to %d, will move by this delta %d", degrees, delta);
        this->StepAByDeg(delta);
        this->CurrentMotorADegree = degrees;
        return;
    }

    if (degrees > this->CurrentMotorADegree)
    {
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "going forward this many degrees %d", delta);
        ESP_LOGI(MOTOR_TAG, "set degree to %d, will move by this delta %d", degrees, delta);
        this->StepAByDeg(delta);
        this->CurrentMotorADegree = degrees;
        return;
    }

    if (degrees > 0 && this->CurrentMotorADegree == 0)
    {
        ESP_LOGI(MOTOR_TAG, "step to %d degree", degrees);
        this->StepAByDeg(degrees);
        this->CurrentMotorADegree = degrees;
        return;
    }

    if (degrees < 0 && this->CurrentMotorADegree == 0)
    {
        ESP_LOGI(MOTOR_TAG, "step to %d degree", degrees);
        this->StepAByDeg(degrees);
        this->CurrentMotorADegree = degrees;
        return;
    }

    if (degrees == 0 && this->CurrentMotorADegree < 0)
    {
        ESP_LOGI(MOTOR_TAG, "reset to 0 by this many degrees %d (forward)", this->CurrentMotorADegree);
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "set degree to ZERO, will move by this delta %d", delta);
        this->StepAByDeg(delta);
        this->CurrentMotorADegree = 0;
        return;
    }

    if (degrees == 0 && this->CurrentMotorADegree > 0)
    {
        ESP_LOGI(MOTOR_TAG, "reset to 0 by this many degrees %d (backward)", this->CurrentMotorADegree);
        int delta = std::clamp(degrees - this->CurrentMotorADegree, -A_MAX_BACKWARD_RANGE_DEG, A_MAX_FORWARD_RANGE_DEG);
        ESP_LOGI(MOTOR_TAG, "set degree to ZERO, will move by this delta %d", delta);
        this->StepAByDeg(delta);
        this->CurrentMotorADegree = 0;
        return;
    }
}

void MotorController::StepAByDeg(int howManyDegrees)
{
    bool isMinus = howManyDegrees < 0;
    if (howManyDegrees < 0 && this->CurrentMotorADegree == -A_MAX_BACKWARD_RANGE_DEG)
    {
        ESP_LOGI(MOTOR_TAG, "can't step backward already reached %d", -A_MAX_BACKWARD_RANGE_DEG);
        return;
    }
    if (howManyDegrees > 0 && this->CurrentMotorADegree == A_MAX_FORWARD_RANGE_DEG)
    {
        ESP_LOGI(MOTOR_TAG, "can't step forward already reached %d", A_MAX_FORWARD_RANGE_DEG);
        return;
    }
    this->CurrentMotorADegree = this->CurrentMotorADegree + howManyDegrees;
    ESP_LOGI(MOTOR_TAG, "stepping motor by this many degree %d, current motor a degree is %d", howManyDegrees, this->CurrentMotorADegree);

    if (isMinus)
    {
        for (int i = 1; i <= std::abs(howManyDegrees); i++)
        {
            this->StepABackward();
        }
    }
    else
    {
        for (int i = 1; i <= std::abs(howManyDegrees); i++)
        {
            this->StepAForward();
        }
    }
}

void MotorController::StepAForward()
{
    if (this->MotorMovementStatus != MotorMovement::Stopped)
    {
        ESP_LOGW(MOTOR_TAG, "%s: motor is currently busy", __func__);
        return;
    }
    this->MotorMovementStatus = MotorMovement::MotorAMovingPosDeg;
    this->MotorAForward->Pause(false);
    Delay(2000);
    this->MotorAForward->Pause(true);
    this->MotorMovementStatus = MotorMovement::Stopped;
}

void MotorController::StepABackward()
{
    if (this->MotorMovementStatus != MotorMovement::Stopped)
    {
        ESP_LOGW(MOTOR_TAG, "%s: motor is currently busy", __func__);
        return;
    }
    this->MotorMovementStatus = MotorMovement::MotorAMovingNegDeg;
    this->MotorABackward->Pause(false);
    Delay(2000);
    this->MotorABackward->Pause(true);
    this->MotorMovementStatus = MotorMovement::Stopped;
}

void MotorController::StepBForward()
{
    if (this->MotorMovementStatus != MotorMovement::Stopped)
    {
        ESP_LOGW(MOTOR_TAG, "%s: motor is currently busy", __func__);
        return;
    }
    this->MotorMovementStatus = MotorMovement::MotorBMovingPosDeg;
    this->MotorBForward->Pause(false);
    Delay(750);
    this->MotorBForward->Pause(true);
    this->MotorMovementStatus = MotorMovement::Stopped;
}

void MotorController::StepBBackward()
{
    if (this->MotorMovementStatus != MotorMovement::Stopped)
    {
        ESP_LOGW(MOTOR_TAG, "%s: motor is currently busy", __func__);
        return;
    }
    this->MotorMovementStatus = MotorMovement::MotorBMovingNegDeg;
    this->MotorBBackward->Pause(false);
    Delay(1000);
    this->MotorBBackward->Pause(true);
    this->MotorMovementStatus = MotorMovement::Stopped;
}

void MotorController::DebugClearMemory()
{
    ESP_LOGW(MOTOR_TAG, "clear CurrentMotorADegree & CurrentMotorBDegree to 0");

    this->CurrentMotorADegree = 0;
    this->CurrentMotorBDegree = 0;

    Persistence::Get().WriteInt(PERSISTENCE_MOTOR_A_DEG, 0);
    Persistence::Get().WriteInt(PERSISTENCE_MOTOR_B_DEG, 0);
}

void MotorController::ResetA(int newDegree)
{
    
}

MotorController::MotorController()
{
    auto prevADeg = Persistence::Get().GetInt(PERSISTENCE_MOTOR_A_DEG);
    auto prevBDeg = Persistence::Get().GetInt(PERSISTENCE_MOTOR_B_DEG);

    if (!prevADeg.Existed)
    {
        Persistence::Get().WriteInt(PERSISTENCE_MOTOR_A_DEG, 0);
        Persistence::Get().WriteInt(PERSISTENCE_MOTOR_B_DEG, 0);
    }
    else
    {
        this->CurrentMotorADegree = prevADeg.Result;
        this->CurrentMotorBDegree = prevBDeg.Result;
    }

    this->MotorAForward.reset(new PWMPin(PIN_CHANNEL_A_FORWARD, FREQUENCY));
    this->MotorABackward.reset(new PWMPin(PIN_CHANNEL_A_BACKWARD, FREQUENCY));
    this->MotorBForward.reset(new PWMPin(PIN_CHANNEL_B_FORWARD, FREQUENCY));
    this->MotorBBackward.reset(new PWMPin(PIN_CHANNEL_B_BACKWARD, FREQUENCY));

    this->MotorAForward->SetDutyCycle(100);
    this->MotorABackward->SetDutyCycle(100);
    this->MotorBForward->SetDutyCycle(100);
    this->MotorBBackward->SetDutyCycle(100);

    this->MotorAForward->Pause(true);
    this->MotorABackward->Pause(true);
    this->MotorBForward->Pause(true);
    this->MotorBBackward->Pause(true);

    PinFunctions::EnablePin(PIN_ENABLE_FORWARD);
    PinFunctions::EnablePin(PIN_ENABLE_BACKWARD);

    this->MotorMovementStatus = MotorMovement::Stopped;

    ESP_LOGI(MOTOR_TAG, "motors have been initialised");
}

