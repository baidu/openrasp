package test

import (
	"github.com/bouk/monkey"
	"github.com/pkg/errors"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/models"
	"rasp-cloud/tests/inits"
	"rasp-cloud/tests/start"
	"testing"
)

func getValidDependency() map[string]interface{} {
	return map[string]interface{}{
		"page":            1,
		"perpage":         1,
		"data":            map[string]interface{}{
			"app_id": start.TestApp.Id,
		},
	}
}

func TestSearchDependency(t *testing.T) {

	Convey("Subject: Test search dependency Api\n", t, func() {

		Convey("when app data valid", func() {
			dependency := getValidDependency()
			r := inits.GetResponse("POST", "/v1/api/dependency/search", inits.GetJson(dependency))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("app_id can not be empty", func() {
			dependency := getValidDependency()
			dependency["data"] = map[string]interface{}{}
			r := inits.GetResponse("POST", "/v1/api/dependency/search", inits.GetJson(dependency))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when search dependency failed", func() {
			dependency := getValidDependency()
			monkey.Patch(models.SearchDependency,
				func(appId string, param *models.SearchDependencyParam) (int64, []map[string]interface{}, error) {
				return 0, nil, errors.New("")
			})
			defer monkey.Unpatch(models.SearchDependency)
			r := inits.GetResponse("POST", "/v1/api/dependency/search", inits.GetJson(dependency))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestAggrWithSearch(t *testing.T) {
	Convey("Subject: Test Aggr With Search Api\n", t, func() {

		Convey("when app data valid", func() {
			dependency := getValidDependency()
			r := inits.GetResponse("POST", "/v1/api/dependency/aggr", inits.GetJson(dependency))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("app_id can not be empty", func() {
			dependency := getValidDependency()
			dependency["data"] = map[string]interface{}{}
			r := inits.GetResponse("POST", "/v1/api/dependency/aggr", inits.GetJson(dependency))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
		Convey("when aggregation dependency failed", func() {
			dependency := getValidDependency()
			monkey.Patch(models.AggrDependencyByQuery,
				func(appId string, param *models.SearchDependencyParam) (int64, []map[string]interface{}, error) {
					return 0, nil, errors.New("")
				})
			defer monkey.Unpatch(models.AggrDependencyByQuery)
			r := inits.GetResponse("POST", "/v1/api/dependency/aggr", inits.GetJson(dependency))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestAgentPost(t *testing.T) {
	Convey("Subject: Test App Get Secrete Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": start.TestRasp.Id,
				"dependency":[]map[string]interface{}{
					{
						"path": "/home/jdk",
						"vendor": "1234",
						"product": "rasp",
						"version": "1.3",
					},
				},
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("rasp_id can not be empty", func() {
			r := inits.GetResponse("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": "",
				"dependency":[]map[string]interface{}{
					{
						"path": "/home/jdk",
						"vendor": "1234",
						"product": "rasp",
						"version": "1.3",
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when mongo has errors", func() {
			monkey.Patch(models.GetRaspById, func(string) (rasp *models.Rasp, err error) {
				return nil, errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": start.TestRasp.Id,
				"dependency":[]map[string]interface{}{
					{
						"path": "/home/jdk",
						"vendor": "1234",
						"product": "rasp",
						"version": "1.3",
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetRaspById)

			monkey.Patch(models.RemoveDependencyByRasp, func(string, string) (err error) {
				return errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": start.TestRasp.Id,
				"dependency":[]map[string]interface{}{
					{
						"path": "/home/jdk",
						"vendor": "1234",
						"product": "rasp",
						"version": "1.3",
					},
				},
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.RemoveDependencyByRasp)
		})

		Convey("item path can not be empty", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency",
				inits.GetJson(map[string]interface{}{
					"rasp_id": start.TestRasp.Id,
					"dependency":[]map[string]interface{}{
						{
							"path": "",
							"vendor": "1234",
							"product": "rasp",
							"version": "1.3",
						},
					},
				}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("item vendor can not be empty", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency",
				inits.GetJson(map[string]interface{}{
					"rasp_id": start.TestRasp.Id,
					"dependency":[]map[string]interface{}{
						{
							"path": "/home/jdk",
							"vendor": "",
							"product": "rasp",
							"version": "1.3",
						},
					},
				}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("item product can not be empty", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency",
				inits.GetJson(map[string]interface{}{
					"rasp_id": start.TestRasp.Id,
					"dependency":[]map[string]interface{}{
						{
							"path": "/home/jdk",
							"vendor": "1234",
							"product": "",
							"version": "1.3",
						},
					},
				}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("item version can not be empty", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency",
				inits.GetJson(map[string]interface{}{
					"rasp_id": start.TestRasp.Id,
					"dependency":[]map[string]interface{}{
						{
							"path": "/home/jdk",
							"vendor": "1234",
							"product": "rasp",
							"version": "",
						},
					},
				}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("when the length of version is greater than 256", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": start.TestRasp.Id,
				"dependency":[]map[string]interface{}{
					{
						"path": "/home/jdk",
						"vendor": "1234",
						"product": "rasp",
						"version": inits.GetLongString(257),
					},
				},
			}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("when the length of path is greater than 1024", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": start.TestRasp.Id,
				"dependency":[]map[string]interface{}{
					{
						"path": inits.GetLongString(1025),
						"vendor": "1234",
						"product": "rasp",
						"version": "1.3",
					},
				},
			}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("when the length of product is greater than 256", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": start.TestRasp.Id,
				"dependency":[]map[string]interface{}{
					{
						"path": "/home/jdk",
						"vendor": "1234",
						"product": inits.GetLongString(257),
						"version": "1.3",
					},
				},
			}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("when the length of vendor is greater than 256", func() {
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency", inits.GetJson(map[string]interface{}{
				"rasp_id": start.TestRasp.Id,
				"dependency":[]map[string]interface{}{
					{
						"path": "/home/jdk",
						"vendor": inits.GetLongString(257),
						"product": "rasp",
						"version": "1.3",
					},
				},
			}))
			So(r.Desc, ShouldEqual, "")
		})

		Convey("when failed to add dependency data to ES", func() {
			monkey.Patch(models.AddDependency, func(*models.Rasp, []*models.Dependency) (err error){
				return errors.New("")
			})
			monkey.Unpatch(models.AddDependency)
			r := inits.GetResponseWithNoBody("POST", "/v1/agent/dependency",
				inits.GetJson(map[string]interface{}{
					"rasp_id": start.TestRasp.Id,
					"dependency":[]map[string]interface{}{
						{
							"path": "/home/jdk",
							"vendor": "1234",
							"product": "rasp",
							"version": "1.3",
						},
					},
				}))
			So(r.Desc, ShouldEqual, "")
		})
	})
}
