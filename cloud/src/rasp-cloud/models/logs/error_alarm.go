package logs

import (
	"github.com/astaxie/beego"
	"time"
	"rasp-cloud/conf"
)

var (
	ErrorAlarmInfo = AlarmLogInfo{
		EsType:       "error-alarm",
		EsIndex:      "openrasp-error-alarm",
		EsAliasIndex: "real-openrasp-error-alarm",
		TtlTime:      24 * 365 * time.Hour,
		AlarmBuffer:  make(chan map[string]interface{}, conf.AppConfig.AlarmBufferSize),
		FileLogger:   initAlarmFileLogger("/openrasp-logs/error-alarm", "error.log"),
	}
)

func init() {
	registerAlarmInfo(&ErrorAlarmInfo)
}

func AddErrorAlarm(alarm map[string]interface{}) error {
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to add error alarm: ", r)
		}
	}()
	return AddAlarmFunc(ErrorAlarmInfo.EsType, alarm)
}
