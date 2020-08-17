package logs

import (
	"github.com/astaxie/beego"
	"crypto/md5"
	"errors"
	"fmt"
	"strings"
	"rasp-cloud/conf"
)

var (
	CrashAlarmInfo = AlarmLogInfo{
		EsType:       "crash-alarm",
		EsIndex:      "openrasp-crash-alarm",
		EsAliasIndex: "real-openrasp-crash-alarm",
		AlarmBuffer:  make(chan map[string]interface{}, conf.AppConfig.AlarmBufferSize),
		FileLogger:   initAlarmFileLogger("openrasp-logs/crash-alarm", "crash.log"),
	}
)

func init() {
	registerAlarmInfo(&CrashAlarmInfo)
}

func AddCrashAlarm(alarm map[string]interface{}) error {
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to add crash alarm: ", r)
		}
	}()
	if alarm["language"].(string) == "java" {
		alarm = parseJavaStack(alarm)
	} else if alarm["language"].(string) == "php" {
		alarm = parsePhpStack(alarm)
	} else {
		return errors.New("unknown language:" +  alarm["language"].(string))
	}

	err := AddLogWithKafka(AttackAlarmInfo.EsType, alarm)
	if err != nil {
		return err
	}
	return AddAlarmFunc(CrashAlarmInfo.EsType, alarm)
}

func parseJavaStack(alarm map[string]interface{}) map[string]interface{}{
	// java
	var alarmMessage string
	var idContent string
	crashLogContent := fmt.Sprint(alarm["crash_log"])
	splitCrashLogContent := strings.Split(crashLogContent, "\n")
	for idx, content := range splitCrashLogContent {
		if strings.Contains(content, "Problematic frame:") && (idx + 1 < len(splitCrashLogContent)) {
			nextLine := splitCrashLogContent[idx + 1]
			alarmMessage = nextLine[strings.Index(nextLine, "]") + 1:]
		}
	}
	alarm["crash_message"] = strings.TrimSpace(alarmMessage)
	// 去重
	idContent += fmt.Sprint(alarm["crash_message"])
	idContent += fmt.Sprint(alarm["rasp_id"])
	alarm["upsert_id"] = fmt.Sprintf("%x", md5.Sum([]byte(idContent)))
	alarm["stack_md5"] = alarm["upsert_id"]
	return alarm
}

func parsePhpStack(alarm map[string]interface{}) map[string]interface{}{
	// php
	var alarmMessage string
	var idContent string
	var cnt int
	findStack := false
	crashLogContent := fmt.Sprint(alarm["crash_log"])
	splitCrashLogContent := strings.Split(crashLogContent, "\n")
	for idx, content := range splitCrashLogContent {
		// 去掉[0x7f0e258d86ee]类似的内存地址
		if strings.Index(content, "Native stacks:") != -1 {
			findStack = true
			continue
		}
		if strings.Index(content, "[0x") != -1 {
			content = content[:strings.Index(content, "[0x")]
			if strings.Index(content, "openrasp.so") != -1 && findStack {
				cnt += 1
				if cnt == 3 {
					alarmMessage = content
					findStack = false
				}
			}
			splitCrashLogContent[idx] = content
		}
	}
	alarm["crash_message"] = alarmMessage
	crashLogContent = strings.Join(splitCrashLogContent, "\n")
	alarm["crash_log"] = crashLogContent
	// 去重
	idContent += fmt.Sprint(alarm["crash_log"])
	idContent += fmt.Sprint(alarm["rasp_id"])
	alarm["upsert_id"] = fmt.Sprintf("%x", md5.Sum([]byte(idContent)))
	alarm["stack_md5"] = alarm["upsert_id"]
	return alarm
}