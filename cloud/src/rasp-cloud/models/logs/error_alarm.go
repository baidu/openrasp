package logs

import (
	"github.com/astaxie/beego"
	"rasp-cloud/conf"
	"fmt"
	"crypto/md5"
)

var (
	ErrorAlarmInfo = AlarmLogInfo{
		EsType:       "error-alarm",
		EsIndex:      "openrasp-error-alarm",
		EsAliasIndex: "real-openrasp-error-alarm",
		AlarmBuffer:  make(chan map[string]interface{}, conf.AppConfig.AlarmBufferSize),
		FileLogger:   initAlarmFileLogger("openrasp-logs/error-alarm", "error.log"),
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
	idContent := ""
	idContent += fmt.Sprint(alarm["rasp_id"])
	idContent += fmt.Sprint(alarm["error_code"])
	idContent += fmt.Sprint(alarm["message"])
	idContent += fmt.Sprint(alarm["stack_trace"])
	alarm["upsert_id"] = fmt.Sprintf("%x", md5.Sum([]byte(idContent)))
	return AddAlarmFunc(ErrorAlarmInfo.EsType, alarm)
}
