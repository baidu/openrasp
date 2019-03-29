package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
	"rasp-cloud/tests/start"
	"github.com/bouk/monkey"
	"reflect"
	"github.com/astaxie/beego"
	"mime/multipart"
	"os"
	"rasp-cloud/tools"
	"github.com/pkg/errors"
	"io/ioutil"
	"io"
	"rasp-cloud/models"
	"rasp-cloud/mongo"
)

func TestUploadPlugin(t *testing.T) {
	Convey("Subject: Test Operation Search Api\n", t, func() {

		monkey.PatchInstanceMethod(reflect.TypeOf(&beego.Controller{}), "GetFile",
			func(*beego.Controller, string) (multipart.File, *multipart.FileHeader, error) {
				currentPath, _ := tools.GetCurrentPath()
				file, _ := os.Open(currentPath + "/resources/plugin.js")
				return file, &multipart.FileHeader{Size: 1}, nil
			},
		)

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id, "")
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when app_id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin?app_id=ssssssssssssss", "")
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when app_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin", "")
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when get file has error", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&beego.Controller{}), "GetFile",
				func(*beego.Controller, string) (multipart.File, *multipart.FileHeader, error) {
					currentPath, _ := tools.GetCurrentPath()
					file, _ := os.Open(currentPath + "/resources/plugin.js")
					return file, &multipart.FileHeader{Size: 1}, errors.New("")
				},
			)
			r := inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.PatchInstanceMethod(reflect.TypeOf(&beego.Controller{}), "GetFile",
				func(*beego.Controller, string) (multipart.File, *multipart.FileHeader, error) {
					currentPath, _ := tools.GetCurrentPath()
					file, _ := os.Open(currentPath + "/resources/plugin.js")
					return file, &multipart.FileHeader{Size: 0}, nil
				},
			)
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.PatchInstanceMethod(reflect.TypeOf(&beego.Controller{}), "GetFile",
				func(*beego.Controller, string) (multipart.File, *multipart.FileHeader, error) {
					return nil, &multipart.FileHeader{Size: 0}, nil
				},
			)
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.PatchInstanceMethod(reflect.TypeOf(&beego.Controller{}), "GetFile",
				func(*beego.Controller, string) (multipart.File, *multipart.FileHeader, error) {
					return nil, &multipart.FileHeader{Size: 1}, nil
				},
			)
			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte{}, errors.New("")
			})
			defer monkey.Unpatch(ioutil.ReadAll)
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.UnpatchInstanceMethod(reflect.TypeOf(&beego.Controller{}), "GetFile")

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return nil, nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when js plugin is invalid", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&beego.Controller{}), "GetFile",
				func(*beego.Controller, string) (multipart.File, *multipart.FileHeader, error) {
					currentPath, _ := tools.GetCurrentPath()
					file, _ := os.Open(currentPath + "/resources/plugin.js")
					return file, &multipart.FileHeader{Size: 1}, nil
				},
			)
			defer monkey.Unpatch(ioutil.ReadAll)
			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version = '2019-0225-1830'\n" +
					"const plugin_name    = 'official'\n" +
					"// BEGIN ALGORITHM CONFIG //\n" +
					"var algorithmConfig = {}\n" +
					"// END ALGORITHM CONFIG //"), nil
			})
			r := inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldEqual, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version = '2019-0225-1830'\n" +
					"const plugin_name    = 'official'\n" +
					"// BEGIN ALGORITHM CONFIG //\n" +
					"var algorithmConfig = {}\n" +
					"// END ALGORITHM CONFIG //"), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldEqual, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte(""), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version019-0225-1830'\n"), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version = '2019-0225-1830'\n"), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version = '2019-0225-1830'\n" +
					"const plugin_nfficial'\n" ), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version = '2019-0225-1830'\n" +
					"const plugin_name    = 'official'\n" +
					"var algorithmConfig = {}\n" +
					"// END ALGORITHM CONFIG //" ), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version = '2019-0225-1830'\n" +
					"const plugin_name    = 'official'\n" +
					"// BEGIN ALGORITHM CONFIG //\n" +
					"var algorithmConfig = {}\n" ), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id,
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Patch(ioutil.ReadAll, func(io.Reader) ([]byte, error) {
				return []byte("const plugin_version = '2019-0225-1830'\n" +
					"const plugin_name    = 'official'\n" +
					"// BEGIN ALGORITHM CONFIG //\n" +
					"var algorithmConfig = {''''}\n" +
					"// END ALGORITHM CONFIG //"), nil
			})
			r = inits.GetResponse("POST", "/v1/api/plugin?app_id="+start.TestApp.Id, "")
			So(r.Status, ShouldBeGreaterThan, 0)

		})
	})
}

func TestPluginGet(t *testing.T) {
	Convey("Subject: Test Plugin Get Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/get", inits.GetJson(map[string]interface{}{
				"id": start.TestApp.SelectedPluginId,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the plugin id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/get", inits.GetJson(map[string]interface{}{
				"id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("When the plugin id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/get", inits.GetJson(map[string]interface{}{
				"id": "000000000000000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestPluginDownload(t *testing.T) {
	Convey("Subject: Test Plugin Download Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponseRecorder("GET",
				"/v1/api/plugin/download?id="+start.TestApp.SelectedPluginId, "")
			So(r.Code, ShouldEqual, 200)
			So(r.Header().Get("Content-Disposition"), ShouldStartWith, "attachment;filename=")
		})

		Convey("when plugin name is empty", func() {
			monkey.Patch(models.GetPluginById, func(string, bool) (*models.Plugin, error) {
				return &models.Plugin{Name: ""}, nil
			})
			r := inits.GetResponseRecorder("GET",
				"/v1/api/plugin/download?id="+start.TestApp.SelectedPluginId, "")
			So(r.Code, ShouldEqual, 200)
			So(r.Header().Get("Content-Disposition"), ShouldStartWith, "attachment;filename=")
			monkey.Unpatch(models.GetPluginById)
		})

		Convey("when the plugin id is empty", func() {
			r := inits.GetResponse("GET",
				"/v1/api/plugin/download?id=", "")
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestUpdateAlgorithm(t *testing.T) {
	Convey("Subject: Test Plugin Update Algorithm Api\n", t, func() {

		plugin, _ := models.GetPluginById(start.TestApp.SelectedPluginId, false)
		algorithm := plugin.DefaultAlgorithmConfig

		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id":     start.TestApp.SelectedPluginId,
				"config": algorithm,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.UpdateAlgorithmConfig, func(string, map[string]interface{}) (string, error) {
				return "", errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id":     start.TestApp.SelectedPluginId,
				"config": algorithm,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.UpdateAlgorithmConfig)

			monkey.Patch(models.GetPluginById, func(id string, hasContent bool) (*models.Plugin, error) {
				return nil, errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id":     start.TestApp.SelectedPluginId,
				"config": algorithm,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetPluginById)

			r = inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id":     start.TestApp.SelectedPluginId,
				"config": map[string]interface{}{},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the plugin id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id": "",
				"config": map[string]interface{}{
					"sql": "ss",
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("When the plugin id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id": "00000000000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the algorithm config is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id":     "",
				"config": nil,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the algorithm config is invalid", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/config", inits.GetJson(map[string]interface{}{
				"id": "",
				"config": map[string]interface{}{
					"ss": "ss",
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestRestoreAlgorithm(t *testing.T) {
	Convey("Subject: Test Plugin Restore Algorithm Api\n", t, func() {

		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/restore", inits.GetJson(map[string]interface{}{
				"id": start.TestApp.SelectedPluginId,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the plugin id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/restore", inits.GetJson(map[string]interface{}{
				"id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the plugin id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/algorithm/restore", inits.GetJson(map[string]interface{}{
				"id": "0000000000000000000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestDeleteAlgorithm(t *testing.T) {
	plugin, _ := models.GetPluginById(start.TestApp.SelectedPluginId, true)
	plugin, _ = models.AddPlugin([]byte(plugin.Content), start.TestApp.Id)

	Convey("Subject: Test Plugin Restore Algorithm Api\n", t, func() {

		Convey("when the mongodb has errors", func() {
			monkey.Patch(mongo.FindOne, func(collection string, query interface{}, result interface{}) error {
				return errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
				"id": plugin.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)

			monkey.Unpatch(mongo.FindOne)

			monkey.Patch(models.DeletePlugin, func(pluginId string) error {
				return errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
				"id": plugin.Id,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.DeletePlugin)
		})

		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
				"id": plugin.Id,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the plugin is selected", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
				"id": start.TestApp.SelectedPluginId,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the plugin id is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
				"id": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the plugin id does not exist", func() {
			r := inits.GetResponse("POST", "/v1/api/plugin/delete", inits.GetJson(map[string]interface{}{
				"id": "0000000000000000000000000000000000",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}
