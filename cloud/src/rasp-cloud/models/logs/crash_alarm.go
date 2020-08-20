package logs

import (
	"github.com/astaxie/beego"
	"github.com/ianlancetaylor/demangle"
	"crypto/md5"
	"errors"
	"fmt"
	"math/rand"
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

func AddCrashAlarm(alarm map[string]interface{}) (bool, error) {
	sendEmail := true
	defer func() {
		if r := recover(); r != nil {
			beego.Error("failed to add crash alarm: ", r)
		}
	}()
	if alarm["language"].(string) == "java" {
		sendEmail, alarm = parseJavaStack(alarm)
	} else if alarm["language"].(string) == "php" {
		sendEmail, alarm = parsePhpStack(alarm)
	} else {
		return false, errors.New("unknown language:" +  alarm["language"].(string))
	}

	err := AddLogWithKafka(AttackAlarmInfo.EsType, alarm)
	if err != nil {
		return true, err
	}
	return sendEmail, AddAlarmFunc(CrashAlarmInfo.EsType, alarm)
}

func parseJavaStack(alarm map[string]interface{}) (bool, map[string]interface{}) {
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
	idContent += fmt.Sprint(rand.Int())
	alarm["upsert_id"] = fmt.Sprintf("%x", md5.Sum([]byte(idContent)))
	alarm["stack_md5"] = alarm["upsert_id"]
	return true, alarm
}

func parsePhpStack(alarm map[string]interface{}) (bool, map[string]interface{}){
	// php
	var alarmMessage string
	var idContent string
	var cnt int
	// 标记第一次出现openrasp.so的位置
	var firstIdx int
	findStack := false
	crashLogContent := fmt.Sprint(alarm["crash_log"])
	splitCrashLogContent := strings.Split(crashLogContent, "\n")
	for idx, content := range splitCrashLogContent {
		// 去掉[0x7f0e258d86ee]类似的内存地址
		if strings.Index(content, "Native stacks:") != -1 {
			findStack = true
			firstIdx = idx + 3
			continue
		}
		if strings.Index(content, "[0x") != -1 {
			content = content[:strings.Index(content, "[0x")]
			if strings.Index(content, "openrasp.so") != -1 {
				cnt += 1
				if cnt == 3 && findStack {
					tmp := strings.Split(content, "(")[1]
					ret := strings.Split(tmp, "+")
					retMessage, err := demangle.ToString(ret[0])
					if err != nil {
						alarmMessage = content
					} else {
						alarmMessage = retMessage + "+" + strings.Split(ret[1], ")")[0]
					}
					findStack = false
				}
			}
			splitCrashLogContent[idx] = content
		}
	}
	// 如果没有出现第三个openrasp.so, 此处需要附一个无关的值
	if alarmMessage == "" {
		content := splitCrashLogContent[firstIdx]
		alarm["crash_message"] = content[:strings.Index(content, "[0x")]
		alarm["relativity"] = false
	} else {
		alarm["crash_message"] = alarmMessage
		alarm["relativity"] = true
	}
	crashLogContent = strings.Join(splitCrashLogContent, "\n")
	alarm["crash_log"] = crashLogContent
	// 去重
	idContent += fmt.Sprint(alarm["crash_log"])
	idContent += fmt.Sprint(alarm["rasp_id"])
	idContent += fmt.Sprint(rand.Int())
	alarm["upsert_id"] = fmt.Sprintf("%x", md5.Sum([]byte(idContent)))
	alarm["stack_md5"] = alarm["upsert_id"]
	if cnt < 3 {
		return false, alarm
	}
	return true, alarm
}