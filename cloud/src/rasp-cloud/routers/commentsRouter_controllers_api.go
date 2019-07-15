package routers

import (
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/context/param"
)

func init() {

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "Post",
            Router: `/`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "ConfigAlarm",
            Router: `/alarm/config`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "ConfigApp",
            Router: `/config`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "Delete",
            Router: `/delete`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "TestDing",
            Router: `/ding/test`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(
				param.New("config"),
			),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "TestEmail",
            Router: `/email/test`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "UpdateAppGeneralConfig",
            Router: `/general/config`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "GetApp",
            Router: `/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "TestHttp",
            Router: `/http/test`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(
				param.New("config"),
			),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "GetPlugins",
            Router: `/plugin/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "CheckPluginLatest",
            Router: `/plugin/latest`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(
				param.New("config"),
			),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "SetSelectedPlugin",
            Router: `/plugin/select`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "GetSelectedPlugin",
            Router: `/plugin/select/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "GetRasps",
            Router: `/rasp/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "GetAppSecret",
            Router: `/secret/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "RegenerateAppSecret",
            Router: `/secret/regenerate`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:AppController"],
        beego.ControllerComments{
            Method: "UpdateAppWhiteListConfig",
            Router: `/whitelist/config`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:OperationController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:OperationController"],
        beego.ControllerComments{
            Method: "Search",
            Router: `/search`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"],
        beego.ControllerComments{
            Method: "Upload",
            Router: `/`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"],
        beego.ControllerComments{
            Method: "UpdateAppAlgorithmConfig",
            Router: `/algorithm/config`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"],
        beego.ControllerComments{
            Method: "RestoreAlgorithmConfig",
            Router: `/algorithm/restore`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"],
        beego.ControllerComments{
            Method: "Delete",
            Router: `/delete`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"],
        beego.ControllerComments{
            Method: "Download",
            Router: `/download`,
            AllowHTTPMethods: []string{"get"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:PluginController"],
        beego.ControllerComments{
            Method: "Get",
            Router: `/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"],
        beego.ControllerComments{
            Method: "BatchDelete",
            Router: `/batch_delete`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"],
        beego.ControllerComments{
            Method: "GeneralCsv",
            Router: `/csv`,
            AllowHTTPMethods: []string{"get"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"],
        beego.ControllerComments{
            Method: "Delete",
            Router: `/delete`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:RaspController"],
        beego.ControllerComments{
            Method: "Search",
            Router: `/search`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:ReportController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:ReportController"],
        beego.ControllerComments{
            Method: "Search",
            Router: `/dashboard`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:ServerController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:ServerController"],
        beego.ControllerComments{
            Method: "PutUrl",
            Router: `/url`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:ServerController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:ServerController"],
        beego.ControllerComments{
            Method: "GetUrl",
            Router: `/url/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:TokenController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:TokenController"],
        beego.ControllerComments{
            Method: "Post",
            Router: `/`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:TokenController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:TokenController"],
        beego.ControllerComments{
            Method: "Delete",
            Router: `/delete`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:TokenController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:TokenController"],
        beego.ControllerComments{
            Method: "Get",
            Router: `/get`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"],
        beego.ControllerComments{
            Method: "IsLogin",
            Router: `/islogin`,
            AllowHTTPMethods: []string{"get","post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"],
        beego.ControllerComments{
            Method: "Login",
            Router: `/login`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"],
        beego.ControllerComments{
            Method: "Logout",
            Router: `/logout`,
            AllowHTTPMethods: []string{"get","post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api:UserController"],
        beego.ControllerComments{
            Method: "Update",
            Router: `/update`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

}
