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
			Params: nil})

	beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
		beego.ControllerComments{
			Method: "AggregationWithType",
			Router: `/aggr/type`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Params: nil})

	beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
		beego.ControllerComments{
			Method: "AggregationWithUserAgent",
			Router: `/aggr/ua`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Params: nil})

	beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:AttackAlarmController"],
		beego.ControllerComments{
			Method: "Search",
			Router: `/search`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Params: nil})

	beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:PolicyAlarmController"] = append(beego.GlobalControllerRouter["rasp-cloud/controllers/api/fore_logs:PolicyAlarmController"],
		beego.ControllerComments{
			Method: "Search",
			Router: `/search`,
			AllowHTTPMethods: []string{"post"},
			MethodParams: param.Make(),
			Params: nil})

}
