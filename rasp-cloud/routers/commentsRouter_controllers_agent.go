package routers

import (
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/context/param"
)

func init() {

	beego.GlobalControllerRouter["rasp-cloud/controllers/agent:HeartbeatController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/agent:HeartbeatController"],
		beego.ControllerComments{
			Method: "Post",
			Router: `/`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Params: nil})

	beego.GlobalControllerRouter["rasp-cloud/controllers/agent:RaspController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/agent:RaspController"],
		beego.ControllerComments{
			Method: "Post",
			Router: `/`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Params: nil})

	beego.GlobalControllerRouter["rasp-cloud/controllers/agent:ReportController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/agent:ReportController"],
		beego.ControllerComments{
			Method: "Post",
			Router: `/`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Params: nil})

}
