package routers

import (
    "github.com/astaxie/beego"
    "github.com/astaxie/beego/context/param"
)

func init() {

    beego.GlobalControllerRouter["rasp-cloud/controllers/iast:IastController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/iast:IastController"],
        beego.ControllerComments{
            Method: "Post",
            Router: `/`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/iast:IastController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/iast:IastController"],
        beego.ControllerComments{
            Method: "Version",
            Router: `/version`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/iast:IastController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/iast:IastController"],
        beego.ControllerComments{
            Method: "Auth",
            Router: `/auth`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/iast:WebsocketController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/iast:WebsocketController"],
        beego.ControllerComments{
            Method: "Get",
            Router: `/`,
            AllowHTTPMethods: []string{"get"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

}
