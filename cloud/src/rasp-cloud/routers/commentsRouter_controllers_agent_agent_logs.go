package routers

import (
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/context/param"
)

func init() {

    beego.GlobalControllerRouter["rasp-cloud/controllers/agent/agent_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/agent/agent_logs:AttackAlarmController"],
        beego.ControllerComments{
            Method: "Post",
            Router: `/`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/agent/agent_logs:ErrorController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/agent/agent_logs:ErrorController"],
        beego.ControllerComments{
            Method: "Post",
            Router: `/`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/agent/agent_logs:PolicyAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/agent/agent_logs:PolicyAlarmController"],
        beego.ControllerComments{
            Method: "Post",
            Router: `/`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

}
