package test

import (
	"testing"
	_ "rasp-cloud/tests/start"
	"rasp-cloud/models"
	"time"
	"rasp-cloud/tests/inits"
	. "github.com/smartystreets/goconvey/convey"
	"github.com/bouk/monkey"
	"github.com/pkg/errors"
	"rasp-cloud/tests/start"
	"rasp-cloud/mongo"
	"gopkg.in/mgo.v2"
	"rasp-cloud/models/logs"
)

func getValidApp() map[string]interface{} {
	return map[string]interface{}{
		"name":             time.Now().String(),
		"description":      "test app",
		"language":         "java",
		"general_config":   map[string]interface{}{},
		"whitelist_config": []map[string]interface{}{},

		"email_alarm_conf": map[string]interface{}{
			"enable":      true,
			"server_addr": "smtp.sina.com:456",
			"username":    "j524697@sina.cn",
			"password":    "123456789",
			"subject":     "openrasp",
			"recv_addr":   []string{"j524697@sina.cn"},
			"tls_enable":  true,
		},
		"ding_alarm_conf": map[string]interface{}{
			"enable":      true,
			"agent_id":    "manager6632",
			"corp_id":     "ding70235c2f4657eb6378f",
			"corp_secret": "123456789",
			"recv_user":   []string{"2263285838022"},
			"recv_party":  []string{"92843"},
		},
		"http_alarm_conf": map[string]interface{}{
			"enable":    true,
			"recv_addr": []string{"http://172.23.22.14:8088/upload"},
		},
	}
}

func TestHandleApp(t *testing.T) {

	Convey("Subject: Test App Post Api\n", t, func() {

		Convey("when app data valid", func() {
			app := getValidApp()
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			defer models.RemoveAppById(r.Data.(map[string]interface{})["id"].(string))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("app name can not be empty", func() {
			app := getValidApp()
			app["name"] = ""
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("the length of app name can not greater than 65", func() {
			app := getValidApp()
			app["name"] = inits.GetLongString(65)
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("language can not be empty", func() {
			app := getValidApp()
			app["language"] = ""
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when language is not supported", func() {
			app := getValidApp()
			app["language"] = "javh"
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of language is greater than 64", func() {
			app := getValidApp()
			app["language"] = inits.GetLongString(65)
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when description length is greater than 1024", func() {
			app := getValidApp()
			app["description"] = inits.GetLongString(1025)
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when selected plugin id is length greater than 1024", func() {
			app := getValidApp()
			app["selected_plugin_id"] = inits.GetLongString(1025)
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when selected plugin id length is greater than 1024", func() {
			app := getValidApp()
			app["selected_plugin_id"] = inits.GetLongString(1025)
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when mongodb has error", func() {
			monkey.Patch(mongo.FindOne, func(collection string, query interface{}, result interface{}) error {
				return nil
			})
			app := getValidApp()
			app["whitelist_config"] = nil
			r := inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(mongo.FindOne)

			monkey.Patch(mongo.FindOne, func(string, interface{}, interface{}) error {
				return errors.New("")
			})
			app = getValidApp()
			app["whitelist_config"] = nil
			r = inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(mongo.FindOne)

			monkey.Patch(logs.CreateAlarmEsIndex, func(string) (err error) {
				return errors.New("")
			})
			app = getValidApp()
			app["whitelist_config"] = nil
			r = inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(logs.CreateAlarmEsIndex)

			monkey.Patch(mongo.Insert, func(string, interface{}) error {
				return errors.New("")
			})
			app = getValidApp()
			app["whitelist_config"] = nil
			r = inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(mongo.Insert)

			monkey.Patch(models.AddApp, func(app *models.App) (result *models.App, err error) {
				return nil, errors.New("test error")
			})
			defer monkey.Unpatch(models.AddApp)
			app = getValidApp()
			app["whitelist_config"] = nil
			r = inits.GetResponse("POST", "/v1/api/app", inits.GetJson(app))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})

	Convey("Subject: Test App Get Api\n", t, func() {
		Convey("the app id must be exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/get", `{"id":"not exist app id"}`)
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/get", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("get all app with paging", func() {
			r := inits.GetResponse("POST", "/v1/api/app/get", `{"page":1,"perpage":10}`)
			So(r.Status, ShouldEqual, 0)
		})

		Convey("page param must be greater than 0", func() {
			r := inits.GetResponse("POST", "/v1/api/app/get", `{"page":-1,"perpage":10}`)
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("perpage param must be greater than 0", func() {
			r := inits.GetResponse("POST", "/v1/api/app/get", `{"page":10,"perpage":-10}`)
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when mongo has errors", func() {
			monkey.Patch(models.GetAllApp, func(int, int, bool) (int, []*models.App, error) {
				return 0, nil, errors.New("")
			})
			defer monkey.Unpatch(models.GetAllApp)
			r := inits.GetResponse("POST", "/v1/api/app/get", `{"page":1,"perpage":10}`)
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(models.GetAllApp, func(int, int, bool) (int, []*models.App, error) {
				return 0, nil, nil
			})
			defer monkey.Unpatch(models.GetAllApp)
			r = inits.GetResponse("POST", "/v1/api/app/get", `{"page":1,"perpage":10}`)
			So(r.Status, ShouldEqual, 0)

			monkey.Patch(models.GetAppById, func(string) (*models.App, error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(models.GetAppById)
			r = inits.GetResponse("POST", "/v1/api/app/get", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})

}

func TestGetRasp(t *testing.T) {
	Convey("Subject: Test App Get Rasp Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/rasp/get", inits.GetJson(map[string]interface{}{
				"app_id":  start.TestApp.Id,
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/rasp/get", inits.GetJson(map[string]interface{}{
				"app_id":  "000000000000000000000",
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.GetAppById, func(string) (*models.App, error) {
				return nil, nil
			})
			r := inits.GetResponse("POST", "/v1/api/app/rasp/get", inits.GetJson(map[string]interface{}{
				"app_id":  start.TestApp.Id,
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppById)

			monkey.Patch(models.GetRaspByAppId, func(string, int, int) (int, []*models.Rasp, error) {
				return 0, nil, errors.New("")
			})
			defer monkey.Unpatch(models.GetRaspByAppId)
			r = inits.GetResponse("POST", "/v1/api/app/rasp/get", inits.GetJson(map[string]interface{}{
				"app_id":  start.TestApp.Id,
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(models.GetRaspByAppId, func(string, int, int) (int, []*models.Rasp, error) {
				return 0, nil, nil
			})
			r = inits.GetResponse("POST", "/v1/api/app/rasp/get", inits.GetJson(map[string]interface{}{
				"app_id":  start.TestApp.Id,
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldEqual, 0)
		})

	})
}

func TestGetSecrete(t *testing.T) {
	Convey("Subject: Test App Get Secrete Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/secret/get", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/secret/get", inits.GetJson(map[string]interface{}{
				"app_id": "000000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/secret/get", inits.GetJson(map[string]interface{}{
				"app_id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func TestRegenerateSecrete(t *testing.T) {
	Convey("Subject: Test App Regenerate Secrete Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/secret/regenerate", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/secret/regenerate", inits.GetJson(map[string]interface{}{
				"app_id": "000000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/secret/regenerate", inits.GetJson(map[string]interface{}{
				"app_id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func TestConfigGenerate(t *testing.T) {
	Convey("Subject: Test App Generate Config Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": map[string]interface{}{
					"clientip.header": "ClientIP",
				},
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": "",
				"config": map[string]interface{}{
					"clientip.header": "ClientIP",
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": "000000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when config does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of config key is greater than 512", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": map[string]interface{}{
					inits.GetLongString(513): "ClientIP",
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when config key is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": map[string]interface{}{
					"": "ClientIP",
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of config value is greater than 2048", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": map[string]interface{}{
					"clientip.header": inits.GetLongString(2049),
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when one of config value is nil", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": map[string]interface{}{
					"clientip.header": nil,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when mongodb has error", func() {
			monkey.Patch(models.UpdateGeneralConfig, func(string, map[string]interface{}) (*models.App, error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(models.UpdateGeneralConfig)
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": map[string]interface{}{
					"clientip.header": "ClientIP",
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func TestConfigWhitelist(t *testing.T) {
	Convey("Subject: Test App Whitelist Config Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/whitelist/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": []map[string]interface{}{
					{
						"url": "http://127.0.0.1:8086/path",
						"hook": map[string]bool{
							"sql": true,
						},
					},
				},
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the white list base format is invalid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/whitelist/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": []map[string]interface{}{
					{
						"url": "http://127.0.0.1:8086/path",
						"hook": map[string]bool{
							inits.GetLongString(150): true,
						},
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			data := make([]map[string]interface{}, 201, 201)
			r = inits.GetResponse("POST", "/v1/api/app/whitelist/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": data,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/whitelist/config", inits.GetJson(map[string]interface{}{
				"app_id": "",
				"config": []map[string]interface{}{
					{
						"url": "http://127.0.0.1:8086/path",
						"hook": map[string]bool{
							"sql": true,
						},
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/whitelist/config", inits.GetJson(map[string]interface{}{
				"app_id": "000000000000000000000",
				"config": []map[string]interface{}{
					{
						"url": "http://127.0.0.1:8086/path",
						"hook": map[string]bool{
							"sql": true,
						},
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when config does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/whitelist/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of config url is greater than 200", func() {
			r := inits.GetResponse("POST", "/v1/api/app/whitelist/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": []map[string]interface{}{
					{
						"url": inits.GetLongString(201),
						"hook": map[string]bool{
							"sql": true,
						},
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of hook key is greater than 128", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": []map[string]interface{}{
					{
						"url": "http://127.0.0.1:8086/path",
						"hook": map[string]bool{
							inits.GetLongString(128): true,
						},
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when hook value is not boolean", func() {
			r := inits.GetResponse("POST", "/v1/api/app/general/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"config": []map[string]interface{}{
					{
						"url": "http://127.0.0.1:8086/path",
						"hook": map[string]interface{}{
							inits.GetLongString(128): "true",
						},
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func TestConfigAlarm(t *testing.T) {
	Convey("Subject: Test App Alarm Config Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    "j524697@sina.cn",
					"password":    "123456789",
					"subject":     "openrasp",
					"recv_addr":   []string{"j524697@sina.cn"},
					"tls_enable":  true,
					"from":        "openrasp",
				},
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "123456789",
					"recv_user":   []string{"2263285838022"},
					"recv_party":  []string{"92843"},
				},
				"http_alarm_conf": map[string]interface{}{
					"enable":    true,
					"recv_addr": []string{"http://172.23.22.14:8088/upload"},
				},
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.UpdateAppById, func(string, interface{}) (*models.App, error) {
				return nil, errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"http_alarm_conf": map[string]interface{}{
					"enable":    true,
					"recv_addr": []string{"http://172.23.232.144:8088/upload"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.UpdateAppById)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": "",
				"http_alarm_conf": map[string]interface{}{
					"enable":    true,
					"recv_addr": []string{"http://172.23.232.144:8088/upload"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": "000000000000000000000",
				"http_alarm_conf": map[string]interface{}{
					"enable":    true,
					"recv_addr": []string{"http://172.23.232.144:8088/upload"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when email server_addr is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "",
					"username":    "j524697@sina.cn",
					"password":    "************",
					"subject":     "openrasp",
					"recv_addr":   []string{"j524697@sina.cn"},
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of email server_addr is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": inits.GetLongString(257),
					"username":    "j524697@sina.cn",
					"password":    "************",
					"subject":     "openrasp",
					"recv_addr":   []string{"j524697@sina.cn"},
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of email subject is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    "j524697@sina.cn",
					"password":    "************",
					"subject":     inits.GetLongString(257),
					"recv_addr":   []string{"j524697@sina.cn"},
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of email username is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    inits.GetLongString(257),
					"password":    "************",
					"subject":     "alarm",
					"recv_addr":   []string{"j524697@sina.cn"},
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of email password is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    "j524697@sina.cn",
					"password":    inits.GetLongString(257),
					"subject":     "alarm",
					"recv_addr":   []string{"j524697@sina.cn"},
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when email recv_addr is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    "j524697@sina.cn",
					"password":    "************",
					"subject":     "alarm",
					"recv_addr":   nil,
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when count of email recv_addr array is greater than 128", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    "j524697@sina.cn",
					"password":    "************",
					"subject":     "alarm",
					"recv_addr":   inits.GetLongStringArray(129),
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when receive email format error", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    "j524697@sina.cn",
					"password":    "************",
					"subject":     "alarm",
					"recv_addr":   []string{"asd.com"},
					"tls_enable":  true,
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of email from is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"email_alarm_conf": map[string]interface{}{
					"enable":      true,
					"server_addr": "smtp.sina.com:456",
					"username":    "j524697@sina.cn",
					"password":    "************",
					"subject":     "alarm",
					"recv_addr":   []string{"j524697@sina.cn"},
					"tls_enable":  true,
					"from":        inits.GetLongString(257),
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of ding corp_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "",
					"corp_secret": "123456789",
					"recv_user":   []string{"2263285838022"},
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of ding corp_id is greter than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     inits.GetLongString(257),
					"corp_secret": "123456789",
					"recv_user":   []string{"2263285838022"},
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when count of ding corp_secret array is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": inits.GetLongString(257),
					"recv_user":   []string{"2263285838022"},
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when count of ding corp_secret array is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "",
					"recv_user":   []string{"2263285838022"},
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when count of ding recv_user and recv_party is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "************",
					"recv_user":   []string{},
					"recv_party":  []string{},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when one of ding recv_user and recv_party is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "************",
					"recv_user":   nil,
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when count of ding recv_user is greater than 128", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "123456789",
					"recv_user":   inits.GetLongStringArray(129),
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when count of ding recv_party array is greater than 128", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "manager6632",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "123456789",
					"recv_user":   []string{"2263285838022"},
					"recv_party":  inits.GetLongStringArray(129),
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when length of ding agent_id is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    inits.GetLongString(257),
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "123456789",
					"recv_user":   []string{"2263285838022"},
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when ding agent_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"ding_alarm_conf": map[string]interface{}{
					"enable":      true,
					"agent_id":    "",
					"corp_id":     "ding70235c2f4657eb6378f",
					"corp_secret": "123456789",
					"recv_user":   []string{"2263285838022"},
					"recv_party":  []string{"92843"},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the count of recv_addr is greater 128", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"http_alarm_conf": map[string]interface{}{
					"enable":    true,
					"recv_addr": inits.GetLongStringArray(129),
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when recv_addr is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"http_alarm_conf": map[string]interface{}{
					"enable":    true,
					"recv_addr": []string{},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of recv_addr is greater than 256", func() {
			r := inits.GetResponse("POST", "/v1/api/app/alarm/config", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
				"http_alarm_conf": map[string]interface{}{
					"enable":    true,
					"recv_addr": []string{inits.GetLongString(257)},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func TestConfigApp(t *testing.T) {
	Convey("Subject: Test App Config Api\n", t, func() {
		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    "java",
				"name":        "test_app",
				"description": "test app",
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      "",
				"language":    "java",
				"name":        "test_app",
				"description": "test app",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      "0000000000000000000",
				"language":    "java",
				"name":        "test_app",
				"description": "test app",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when language is not supported", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    "japhp",
				"name":        "test_app",
				"description": "test app",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of language is greater than 64", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    inits.GetLongString(65),
				"name":        "test_app",
				"description": "test app",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of language is 0", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    "",
				"name":        "test_app",
				"description": "test app",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app name is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    "java",
				"name":        "",
				"description": "test app",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app name is greater than 64", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    "java",
				"name":        inits.GetLongString(65),
				"description": "test app",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app description is greater than 1024", func() {
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    "java",
				"name":        "test_app",
				"description": inits.GetLongString(1025),
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.UpdateAppById, func(string, interface{}) (*models.App, error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(models.UpdateAppById)
			r := inits.GetResponse("POST", "/v1/api/app/config", inits.GetJson(map[string]interface{}{
				"app_id":      start.TestApp.Id,
				"language":    "java",
				"name":        "test_app",
				"description": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestDeleteApp(t *testing.T) {
	Convey("Subject: Test Delete App Api\n", t, func() {

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": "0000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when param is valid", func() {
			var deleteAppId = "1111111111111111111111"
			mongo.UpsertId("app", deleteAppId, map[string]interface{}{
				"name":        "test_delete",
				"language":    "java",
				"description": "test delete",
			})
			r := inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": deleteAppId,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongodb has errors", func() {

			var deleteAppId = "1111111111111111111111"
			mongo.UpsertId("app", deleteAppId, map[string]interface{}{
				"name":        "test_delete",
				"language":    "java",
				"description": "test delete",
			})

			monkey.Patch(models.RemoveAppById, func(id string) (app *models.App, err error) {
				return &models.App{Id: deleteAppId}, nil
			})
			defer monkey.Unpatch(models.RemoveAppById)

			monkey.Patch(models.GetAppCount, func() (count int, err error) {
				return 0, errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": deleteAppId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppCount)

			monkey.Patch(models.GetAppCount, func() (count int, err error) {
				return 10, nil
			})
			monkey.Patch(models.RemoveRaspByAppId, func(id string) (err error) {
				return errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": deleteAppId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.RemoveRaspByAppId)

			monkey.Patch(models.RemovePluginByAppId, func(appId string) error {
				return errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": deleteAppId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.RemovePluginByAppId)

			monkey.Patch(models.GetAppCount, func() (count int, err error) {
				return 1, nil
			})
			r = inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": deleteAppId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppCount)

			monkey.Patch(models.RemoveAppById, func(id string) (app *models.App, err error) {
				return nil, errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/api/app/delete", inits.GetJson(map[string]interface{}{
				"id": deleteAppId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.RemoveAppById)

		})

	})
}

func TestGetPlugins(t *testing.T) {
	Convey("Subject: Test App Get Plugins Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/get", inits.GetJson(map[string]interface{}{
				"app_id":  start.TestApp.Id,
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/get", inits.GetJson(map[string]interface{}{
				"app_id":  "0000000000000000000",
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/get", inits.GetJson(map[string]interface{}{
				"app_id":  "",
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.GetAppById, func(id string) (app *models.App, err error) {
				return nil, nil
			})
			r := inits.GetResponse("POST", "/v1/api/app/plugin/get", inits.GetJson(map[string]interface{}{
				"app_id":  start.TestApp.Id,
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppById)

			monkey.Patch(models.GetPluginsByApp, func(string, int, int, string) (int, []models.Plugin, error) {
				return 0, nil, errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/api/app/plugin/get", inits.GetJson(map[string]interface{}{
				"app_id":  start.TestApp.Id,
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetPluginsByApp)
		})

	})
}

func TestGetSelectedPlugin(t *testing.T) {
	Convey("Subject: Test App Get Selected Plugin Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select/get", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select/get", inits.GetJson(map[string]interface{}{
				"app_id": "0000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select/get", inits.GetJson(map[string]interface{}{
				"app_id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.GetSelectedPlugin, func(string, bool) (*models.Plugin, error) {
				return nil, mgo.ErrNotFound
			})
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select/get", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
			monkey.Unpatch(models.GetSelectedPlugin)
		})

	})
}

func TestSelectPlugin(t *testing.T) {
	Convey("Subject: Test App Select Plugin Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select", inits.GetJson(map[string]interface{}{
				"app_id":    start.TestApp.Id,
				"plugin_id": start.TestApp.SelectedPluginId,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select", inits.GetJson(map[string]interface{}{
				"app_id":    "0000000000000000000",
				"plugin_id": start.TestApp.SelectedPluginId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select", inits.GetJson(map[string]interface{}{
				"app_id":    "",
				"plugin_id": start.TestApp.SelectedPluginId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when plugin_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/plugin/select", inits.GetJson(map[string]interface{}{
				"app_id":    start.TestApp.Id,
				"plugin_id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func TestTestEmail(t *testing.T) {
	Convey("Subject: Test App Email Test Api\n", t, func() {
		monkey.Patch(models.PushEmailAttackAlarm, func(*models.App, int64, []map[string]interface{}, bool) error {
			return nil
		})
		defer monkey.Unpatch(models.PushEmailAttackAlarm)
		start.TestApp.EmailAlarmConf.Enable = true

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/email/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when email alarm is not enable", func() {
			monkey.Patch(models.GetAppByIdWithoutMask, func(id string) (app *models.App, err error) {
				app, _ = models.GetAppById(start.TestApp.Id)
				app.EmailAlarmConf.Enable = false
				return
			})
			r := inits.GetResponse("POST", "/v1/api/app/email/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppByIdWithoutMask)
		})

		Convey("when the email has errors", func() {
			monkey.Patch(models.PushEmailAttackAlarm, func(*models.App, int64, []map[string]interface{}, bool) error {
				return errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/app/email/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.PushEmailAttackAlarm)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/email/test", inits.GetJson(map[string]interface{}{
				"app_id": "0000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/email/test", inits.GetJson(map[string]interface{}{
				"app_id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}

func TestTestDing(t *testing.T) {
	Convey("Subject: Test App Ding Test Api\n", t, func() {
		monkey.Patch(models.PushDingAttackAlarm, func(*models.App, int64, []map[string]interface{}, bool) error {
			return nil
		})
		defer monkey.Unpatch(models.PushDingAttackAlarm)
		start.TestApp.EmailAlarmConf.Enable = true

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/ding/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/ding/test", inits.GetJson(map[string]interface{}{
				"app_id": "0000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/ding/test", inits.GetJson(map[string]interface{}{
				"app_id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when ding alarm is not enable", func() {
			monkey.Patch(models.GetAppByIdWithoutMask, func(id string) (app *models.App, err error) {
				app, _ = models.GetAppById(start.TestApp.Id)
				app.DingAlarmConf.Enable = false
				return
			})
			r := inits.GetResponse("POST", "/v1/api/app/ding/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppByIdWithoutMask)
		})

		Convey("when the ding ding has errors", func() {
			monkey.Patch(models.PushDingAttackAlarm, func(*models.App, int64, []map[string]interface{}, bool) error {
				return errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/app/ding/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.PushDingAttackAlarm)
		})

	})
}

func TestTestHttp(t *testing.T) {
	Convey("Subject: Test App HTTP Test Api\n", t, func() {
		monkey.Patch(models.PushHttpAttackAlarm, func(*models.App, int64, []map[string]interface{}, bool) error {
			return nil
		})
		defer monkey.Unpatch(models.PushHttpAttackAlarm)
		start.TestApp.EmailAlarmConf.Enable = true

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/app/http/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/app/http/test", inits.GetJson(map[string]interface{}{
				"app_id": "0000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/app/http/test", inits.GetJson(map[string]interface{}{
				"app_id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when ding alarm is not enable", func() {
			monkey.Patch(models.GetAppByIdWithoutMask, func(id string) (app *models.App, err error) {
				app, _ = models.GetAppById(start.TestApp.Id)
				app.HttpAlarmConf.Enable = false
				return
			})
			r := inits.GetResponse("POST", "/v1/api/app/http/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppByIdWithoutMask)
		})

		Convey("when the http has errors", func() {
			monkey.Patch(models.PushHttpAttackAlarm, func(*models.App, int64, []map[string]interface{}, bool) error {
				return errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/app/http/test", inits.GetJson(map[string]interface{}{
				"app_id": start.TestApp.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.PushHttpAttackAlarm)
		})

	})
}
