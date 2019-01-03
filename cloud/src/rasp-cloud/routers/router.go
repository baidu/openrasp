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

package routers

import (
	"github.com/astaxie/beego"
	"rasp-cloud/controllers/agent"
	"rasp-cloud/controllers/agent/agent_logs"
	"rasp-cloud/controllers/api"
	"rasp-cloud/controllers/api/fore_logs"
	"rasp-cloud/tools"
	"rasp-cloud/environment"
)

func InitRouter() {
	agentNS := beego.NewNamespace("/agent",
		beego.NSNamespace("/heartbeat",
			beego.NSInclude(
				&agent.HeartbeatController{},
			),
		),
		beego.NSNamespace("/log",
			beego.NSNamespace("/attack",
				beego.NSInclude(
					&agent_logs.AttackAlarmController{},
				),
			),
			beego.NSNamespace("/policy",
				beego.NSInclude(
					&agent_logs.PolicyAlarmController{},
				),
			),
		),
		beego.NSNamespace("/rasp",
			beego.NSInclude(
				&agent.RaspController{},
			),
		),
		beego.NSNamespace("/report",
			beego.NSInclude(
				&agent.ReportController{},
			),
		),
	)
	foregroudNS := beego.NewNamespace("/api",

		beego.NSNamespace("/plugin",
			beego.NSInclude(
				&api.PluginController{},
			),
		),
		beego.NSNamespace("/log",
			beego.NSNamespace("/attack",
				beego.NSInclude(
					&fore_logs.AttackAlarmController{},
				),
			),
			beego.NSNamespace("/policy",
				beego.NSInclude(
					&fore_logs.PolicyAlarmController{},
				),
			),
		),
		beego.NSNamespace("/app",
			beego.NSInclude(
				&api.AppController{},
			),
		),
		beego.NSNamespace("/rasp",
			beego.NSInclude(
				&api.RaspController{},
			),
		),
		beego.NSNamespace("/token",
			beego.NSInclude(
				&api.TokenController{},
			),
		),
		beego.NSNamespace("/report",
			beego.NSInclude(
				&api.ReportController{},
			),
		),
		beego.NSNamespace("/operation",
			beego.NSInclude(
				&api.OperationController{},
			),
		),
		beego.NSNamespace("/agentdomain",
			beego.NSInclude(
				&api.AgentDomainController{},
			),
		),
	)
	userNS := beego.NewNamespace("/user", beego.NSInclude(&api.UserController{}))
	ns := beego.NewNamespace("/v1")
	startType := *environment.StartFlag.StartType
	if startType == environment.StartTypeForeground {
		ns.Namespace(foregroudNS, userNS)
	} else if startType == environment.StartTypeAgent {
		ns.Namespace(agentNS)
	} else if startType == environment.StartTypeDefault {
		ns.Namespace(foregroudNS, agentNS, userNS)
	} else {
		tools.Panic(tools.ErrCodeStartTypeNotSupport, "The start type is not supported: "+startType, nil)
	}
	if startType == environment.StartTypeForeground || startType == environment.StartTypeDefault {
		beego.SetStaticPath("//", "dist")
	}
	beego.AddNamespace(ns)
}
