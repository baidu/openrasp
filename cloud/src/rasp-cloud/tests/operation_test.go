package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
	"time"
	"rasp-cloud/tests/start"
	"github.com/bouk/monkey"
	"rasp-cloud/models"
	"errors"
)

func getOperationData() map[string]interface{} {

	return map[string]interface{}{
		"data": map[string]interface{}{
			"ip":      "127.0.0.1",
			"app_id":  start.TestApp.Id,
			"user":    "user",
			"type_id": 10001,
			"id":      "0000000",
		},
		"start_time": time.Now().Nanosecond(),
		"end_time":   time.Now().Nanosecond(),
		"page":       1,
		"perpage":    1,
	}

}

func TestOperationSearch(t *testing.T) {
	Convey("Subject: Test Operation Search Api\n", t, func() {

		Convey("when param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/operation/search",
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when data is nil", func() {
			data := getOperationData()
			data["data"] = nil
			r := inits.GetResponse("POST", "/v1/api/operation/search",
				inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when start_time is less than 0", func() {
			data := getOperationData()
			data["start_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/operation/search",
				inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when end_time is less than 0", func() {
			data := getOperationData()
			data["end_time"] = -10
			r := inits.GetResponse("POST", "/v1/api/operation/search",
				inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when start_time is greater than end_time", func() {
			data := getOperationData()
			data["start_time"] = 15
			data["end_time"] = 10
			r := inits.GetResponse("POST", "/v1/api/operation/search",
				inits.GetJson(data))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the mongodb error", func() {
			monkey.Patch(models.FindOperation, func(data *models.Operation, startTime int64, endTime int64,
				page int, perpage int) (count int, result []models.Operation, err error) {
				return 0, nil, errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/api/operation/search",
				inits.GetJson(getOperationData()))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

	})
}
