//Copyright 2017-2019 Baidu Inc.
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http: //www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

package agent_logs

import (
	"rasp-cloud/controllers"
	"rasp-cloud/models/logs"
	"time"
)

// Operations about policy alarm message
type PolicyAlarmController struct {
	controllers.BaseController
}

// @router / [post]
func (o *PolicyAlarmController) Post() {
	var alarms []map[string]interface{}
	o.UnmarshalJson(&alarms)
	count := 0
	for _, alarm := range alarms {
		alarm["@timestamp"] = time.Now().UnixNano() / 1000000
		err := logs.AddPolicyAlarm(alarm)
		if err == nil {
			count++
		}
	}
	o.Serve(map[string]uint64{"count": uint64(count)})
}
