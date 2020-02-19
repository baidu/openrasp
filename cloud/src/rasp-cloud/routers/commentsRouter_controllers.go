package routers

import (
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/context/param"
)

func init() {

    beego.GlobalControllerRouter["rasp-cloud/controllers:PingController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers:PingController"],
        beego.ControllerComments{
            Method: "Ping",
            Router: `/`,
            AllowHTTPMethods: []string{"get","post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

	beego.GlobalControllerRouter["rasp-cloud/controllers:GeneralController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers:GeneralController"],
		beego.ControllerComments{
			Method: "Version",
			Router: `/`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Filters: nil,
			Params: nil})
}
