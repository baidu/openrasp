package test

import (
	"rasp-cloud/tests/inits"
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"time"
	"rasp-cloud/tests/start"
	"github.com/bouk/monkey"
	"rasp-cloud/models"
	"errors"
)

func TestPostReportData(t *testing.T) {
	Convey("Subject: Test Post Report Data Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/agent/report", inits.GetJson(
				map[string]interface{}{
					"time":        time.Now().Nanosecond() / 1000000,
					"request_sum": 1000,
					"rasp_id":     start.TestRasp.Id,
				},
			))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the rasp_id is empty", func() {
			r := inits.GetResponse("POST", "/v1/agent/report", inits.GetJson(
				map[string]interface{}{
					"time":        time.Now().Nanosecond() / 1000000,
					"request_sum": 1000,
					"rasp_id":     "",
				},
			))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the time is less than 0", func() {
			r := inits.GetResponse("POST", "/v1/agent/report", inits.GetJson(
				map[string]interface{}{
					"time":        -10,
					"request_sum": 1000,
					"rasp_id":     start.TestRasp.Id,
				},
			))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the request_sum is less than 0", func() {
			r := inits.GetResponse("POST", "/v1/agent/report", inits.GetJson(
				map[string]interface{}{
					"time":        time.Now().Nanosecond() / 1000000,
					"request_sum": -10,
					"rasp_id":     start.TestRasp.Id,
				},
			))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the ES has errors", func() {
			monkey.Patch(models.GetRaspById, func(string) (*models.Rasp, error) {
				return nil, errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/agent/report", inits.GetJson(
				map[string]interface{}{
					"time":        time.Now().Nanosecond() / 1000000,
					"request_sum": 1000,
					"rasp_id":     start.TestRasp.Id,
				},
			))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetRaspById)

			monkey.Patch(models.AddReportData, func(*models.ReportData, string) error {
				return errors.New("")
			})
			r = inits.GetResponse("POST", "/v1/agent/report", inits.GetJson(
				map[string]interface{}{
					"time":        time.Now().Nanosecond() / 1000000,
					"request_sum": 1000,
					"rasp_id":     start.TestRasp.Id,
				},
			))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.AddReportData)
		})
	})
}

func getValidReportSearchData() map[string]interface{} {
	return map[string]interface{}{
		"start_time": time.Now().Nanosecond(),
		"end_time":   time.Now().Nanosecond(),
		"time_zone":  "+08:00",
		"app_id":     start.TestApp.Id,
		"interval":   "day",
	}
}

func TestSearchReportData(t *testing.T) {
	Convey("Subject: Test Search Report Data Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(
				getValidReportSearchData(),
			))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the start time is empty", func() {
			data := getValidReportSearchData()
			data["start_time"] = nil
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the start time is not number", func() {
			data := getValidReportSearchData()
			data["start_time"] = "asdfsadf"
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the start time is less than 0", func() {
			data := getValidReportSearchData()
			data["start_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the end time is less than 0", func() {
			data := getValidReportSearchData()
			data["end_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the end time is not number", func() {
			data := getValidReportSearchData()
			data["end_time"] = "asdfsdf"
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the end time is empty", func() {
			data := getValidReportSearchData()
			data["end_time"] = nil
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the time_zone is nil", func() {
			data := getValidReportSearchData()
			data["time_zone"] = nil
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the time_zone is not string", func() {
			data := getValidReportSearchData()
			data["time_zone"] = 132456
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the time_zone is empty", func() {
			data := getValidReportSearchData()
			data["time_zone"] = 132456
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the interval is not string", func() {
			data := getValidReportSearchData()
			data["interval"] = 132456
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the interval is empty", func() {
			data := getValidReportSearchData()
			data["interval"] = nil
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the interval is invalid", func() {
			data := getValidReportSearchData()
			data["interval"] = "second"
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the app_id is empty", func() {
			data := getValidReportSearchData()
			data["app_id"] = nil
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the app_id is not string", func() {
			data := getValidReportSearchData()
			data["app_id"] = 123456
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the db error", func() {
			monkey.Patch(models.GetAppById, func(id string) (app *models.App, err error) {
				return nil, errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(
				getValidReportSearchData(),
			))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetAppById)

			monkey.Patch(models.GetHistoryRequestSum, func(startTime int64, endTime int64, interval string, timeZone string,
				appId string) (error, []map[string]interface{}) {
				return errors.New(""), nil
			})
			r = inits.GetResponse("POST", "/v1/api/report/dashboard", inits.GetJson(
				getValidReportSearchData(),
			))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.GetHistoryRequestSum)
		})
	})
}
