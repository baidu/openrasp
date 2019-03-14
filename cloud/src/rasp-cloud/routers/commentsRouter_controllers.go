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

}
