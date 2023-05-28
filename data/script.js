const AutomaticModeStatus = document.getElementById("automaticModeStatus")
const MoveAMax = document.getElementById("MoveAMax")
const MoveAMin = document.getElementById("MoveAMin")
const MoveBMax = document.getElementById("MoveBMax") 
const MoveBMin = document.getElementById("MoveBMin")
const ResetAll = document.getElementById("ResetAll")
const StepAPos = document.getElementById("StepAPos")
const StepANeg = document.getElementById("StepANeg")
const StepBPos = document.getElementById("StepBPos")
const StepBNeg = document.getElementById("StepBNeg")
const OverrideStepAPos = document.getElementById("OverrideStepAPos")
const OverrideStepANeg = document.getElementById("OverrideStepANeg")
const OverrideStepBPos = document.getElementById("OverrideStepBPos")
const OverrideStepBNeg = document.getElementById("OverrideStepBNeg")
const SyncDate = document.getElementById("SyncDate")
const OpenLoop = document.getElementById("OpenLoop")
const ClosedLoop = document.getElementById("ClosedLoop")
const Stop = document.getElementById("Stop")
const Clear = document.getElementById("Clear")
const Download = document.getElementById("Download")
const Reboot = document.getElementById("Reboot")

const ClearDataLDR = document.getElementById("ClearDataLDR")
const DownloadDataLDR = document.getElementById("DownloadDataLDR")



const MotorControllerStatus = document.getElementById("motorControllerStatus")
const browserTime = document.getElementById("browserTime")
const statusText  = document.getElementById("status")
const DegreeMotorA = document.getElementById("degreeMotorA")
const DegreeMotorB = document.getElementById("degreeMotorB")
const LdrN = document.getElementById("ldrn")
const LdrW = document.getElementById("ldrw")
const LdrS = document.getElementById("ldrs")
const LdrE = document.getElementById("ldre")
const Long = document.getElementById("long")
const Lat  = document.getElementById("lat")
const Alt  = document.getElementById("alt")
const Azi  = document.getElementById("azi")
const Elev = document.getElementById("elev")
const Time = document.getElementById("time")
const OpenLoopStatus = document.getElementById("openloopstatus")
const ClosedloopStatus = document.getElementById("openloopstatus")
const Datalogs = document.getElementById("datalogs")
const DatalogsLDR = document.getElementById("datalogsLDR")

const Volt    = document.getElementById("volt")
const Current = document.getElementById("current")
const Power = document.getElementById("power")

/**
 * 	enum class MotorMovement {
		Stopped,
		MotorAMovingPosDeg,
		MotorAMovingNegDeg,
		MotorBMovingPosDeg,
		MotorBMovingNegDeg,
		Resetting
	};
 */
const MotorControllerStatusToString = {
    [0]: "Stopped",
    [1]: "MotorAMovingPosDeg",
    [2]: "MotorAMovingNegDeg",
    [3]: "MotorBMovingPosDeg",
    [4]: "MotorBMovingNegDeg",
    [5]: "Resetting",
}

const Columns = `Time;Current;Voltage;Power`
const Columns2 = `Time;LDRN;LDRW;LDRS;LDRE`

function ClearLogs() {
    Datalogs.value = `${Columns}`
}

function ClearLogsLDR() {
    DatalogsLDR.value = `${Columns2}`
}

function DownloadLogs() {
    const text = Datalogs.value;

    const blob = new Blob([text], { type: 'text/csv' });
    const url = URL.createObjectURL(blob);

    const link = document.createElement('a');
    link.href = url;
    link.download = 'data.csv';

    document.body.appendChild(link);

    link.click();

    document.body.removeChild(link);
}


function DownloadLogsLDR() {
    const text = DatalogsLDR.value;

    const blob = new Blob([text], { type: 'text/csv' });
    const url = URL.createObjectURL(blob);

    const link = document.createElement('a');
    link.href = url;
    link.download = 'data.csv';

    document.body.appendChild(link);

    link.click();

    document.body.removeChild(link);
}

ClearLogs()
ClearLogsLDR()

function TruncateDecimals(num, digits) {
    var numS = num.toString(),
        decPos = numS.indexOf('.'),
        substrLength = decPos == -1 ? numS.length : 1 + decPos + digits,
        trimmedResult = numS.substr(0, substrLength),
        finalResult = isNaN(trimmedResult) ? 0 : trimmedResult;

    return parseFloat(finalResult);
}

function GetFormattedTime() {
    const date = new Date();
    const hours = date.getHours().toString().padStart(2, '0')
    const minutes = date.getMinutes().toString().padStart(2, '0')
    const seconds = date.getSeconds().toString().padStart(2, '0')
    const day = date.getDate().toString().padStart(2, '0')
    const month = (date.getMonth() + 1).toString().padStart(2, '0')
    const year = date.getFullYear()
    return `${year}-${month}-${day}T${hours}:${minutes}:${seconds}`
}

function UpdateDataPeriodically() {
    function UpdateData() {
        const xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                const data = JSON.parse(this.responseText)
                DegreeMotorA.value = data.degreeMotorA
                DegreeMotorB.value = data.degreeMotorB
                MotorControllerStatus.value = MotorControllerStatusToString[data.motorControllerStatus]

                LdrN.value = TruncateDecimals(data.ldrn,3)
                LdrW.value = TruncateDecimals(data.ldrw,3)
                LdrS.value = TruncateDecimals(data.ldrs,3)
                LdrE.value = TruncateDecimals(data.ldre,3)

                Long.value = data.long
                Lat.value = data.lat
                Alt.value = data.alt
                Azi.value = data.azi
                Elev.value = data.elev

                Time.value = data.time

                OpenLoopStatus.checked = data.openlooprunning
                ClosedloopStatus.checked = data.closedlooprunning

                if (data.openlooprunning) {
                    AutomaticModeStatus.innerHTML = `Armed. Running Open Loop Mode`
                }
                else if (data.closedlooprunning) {
                    AutomaticModeStatus.innerHTML = `Armed. Running Closed Loop Mode`
                }
                else if (!data.openlooprunning && data.closedlooprunning && data.motorControllerStatus != 0) {
                    AutomaticModeStatus.innerHTML = `Armed. Manual adjustment mode`
                } else {
                    AutomaticModeStatus.innerHTML = `Stopped`
                }

                Volt.value = TruncateDecimals(data.volt, 3)
                Current.value = TruncateDecimals(data.current, 3)
                Power.value = TruncateDecimals(data.power, 3)

                Datalogs.value += `\n${Time.value};${Current.value};${Volt.value};${Power.value}`

                if (data.closedlooprunning)
                {
                    DatalogsLDR.value += `\n${Time.value};${LdrN.value};${LdrW.value};${LdrS.value};${LdrE.value}`
                }
                
            }
        };
        xhr.open("GET", "/Status", true)
        xhr.send()
    }

    var countdown = 3;
    var countdownInterval = setInterval(function () {
        statusText.innerText = `Updating in ${countdown} seconds`
        countdown--
        if (countdown < 0) {
            clearInterval(countdownInterval)
            UpdateData()
            countdown = 3
            countdownInterval = setInterval(arguments.callee, 1000)
        }
    }, 1000)
}

function UpdateDate() {
    function UpdateData() {
        const xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                const data = JSON.parse(this.responseText)
                statusText.value = `Date was changed to: ${data.dateTime}`
            }
            else if (this.readyState == 4 && this.status != 200) {
                statusText.value = `Error executing: SetDate`
            }
        };
        xhr.open("POST", `/SetDate`, true)
        const currentTime = GetFormattedTime()
        xhr.send(currentTime)
    }

    UpdateData()
}

function UpdateCalibration() {
    function UpdateData() {
        const xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                const data = this.responseText
                statusText.value = data
            }
            else if (this.readyState == 4 && this.status != 200) {
                statusText.value = `Error executing: SetCalibration`
            }
        };
        xhr.open("POST", `/SetCalibration`, true)
        const calibrationData = {
            maxCurrent: parseFloat(MaxCurrent.value),
            shuntResistance: parseFloat(shuntResistance.value)
        }
        xhr.send(JSON.stringify(calibrationData))
    }

    UpdateData()
}

function SendCommand(command) {
    function UpdateData(command) {
        const xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                statusText.value = `Command ${command} success`
            }
            else if (this.readyState == 4 && this.status != 200) {
                statusText.value = `Error executing: ${command}`
            }
        };
        xhr.open("GET", `/${command}`, true)
        xhr.send()
    }

    UpdateData(command)
}

MoveAMax.onclick = () => {
    SendCommand(`MoveAMax`)
}

MoveAMin.onclick = () => {
    SendCommand(`MoveAMin`)
}

MoveBMax.onclick = () => {
    SendCommand(`MoveBMax`)
}

MoveBMin.onclick = () => {
    SendCommand(`MoveBMin`)
}

ResetAll.onclick = () => {
    SendCommand(`ResetAll`)
}

StepAPos.onclick = () => {
    SendCommand(`StepAPos`)
}

StepANeg.onclick = () => {
    SendCommand(`StepANeg`)
}

StepBPos.onclick = () => {
    SendCommand(`StepBPos`)
}

StepBNeg.onclick = () => {
    SendCommand(`StepBNeg`)
}

OverrideStepAPos.onclick = () => {
    SendCommand(`OverrideStepAPos`)
}

OverrideStepBPos.onclick = () => {
    SendCommand(`OverrideStepBPos`)
}

OverrideStepANeg.onclick = () => {
    SendCommand(`OverrideStepANeg`)
}

OverrideStepBNeg.onclick = () => {
    SendCommand(`OverrideStepBNeg`)
}

SyncDate.onclick = () => {
    UpdateDate()
}

OpenLoop.onclick = () => {
    SendCommand(`OpenLoop`)
}

ClosedLoop.onclick = () => {
    SendCommand(`ClosedLoop`)
}

Stop.onclick = () => {
    SendCommand(`Stop`)
}

Reboot.onclick = () => {
    if(confirm("reboot yes/no?") == false) return
    SendCommand(`Reboot`)
}

Clear.onclick = () => {
    if (confirm("clear logs yes/no?") == false) return
    ClearLogs()
}
ClearDataLDR.onclick = () => {
    if (confirm("clear logs yes/no?") == false) return
    ClearLogsLDR()
}


Download.onclick = () => {
    DownloadLogs()
}

DownloadDataLDR.onclick = () => {
    DownloadLogsLDR()
}



setInterval(() => {
    browserTime.value = GetFormattedTime()
}, 500)

UpdateDataPeriodically()