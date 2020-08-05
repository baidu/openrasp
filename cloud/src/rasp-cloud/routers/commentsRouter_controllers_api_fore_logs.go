package routers

import (
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/context/param"
)

func init() {

    beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
        beego.ControllerComments{
            Method: "AggregationWithTime",
            Router: `/aggr/time`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
        beego.ControllerComments{
            Method: "AggregationWithType",
            Router: `/aggr/type`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
        beego.ControllerComments{
            Method: "AggregationWithUserAgent",
            Router: `/aggr/ua`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
        beego.ControllerComments{
            Method: "AggregationVuln",
            Router: `/aggr/vuln`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
        beego.ControllerComments{
            Method: "Search",
            Router: `/search`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:ErrorController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:ErrorController"],
        beego.ControllerComments{
            Method: "Search",
            Router: `/search`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

    beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:PolicyAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:PolicyAlarmController"],
        beego.ControllerComments{
            Method: "Search",
            Router: `/search`,
            AllowHTTPMethods: []string{"post"},
            MethodParams: param.Make(),
            Filters: nil,
            Params: nil})

}
