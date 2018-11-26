package api

import (
	"encoding/json"
	"fmt"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
)

type ReportController struct {
	controllers.BaseController
}

var (
	intervals = [...]string{"hour", "day", "month"}
)

// @router /dashboard [post]
func (o *ReportController) Search() {
	var query map[string]interface{}
	err := json.Unmarshal(o.Ctx.Input.RequestBody, &query)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "Invalid JSON request", err)
	}
	startTimeParam := query["start_time"]
	if startTimeParam == nil {
		o.ServeError(http.StatusBadRequest, "start_time cannot be empty")
	}
	startTime, ok := startTimeParam.(float64)
	if !ok {
		o.ServeError(http.StatusBadRequest, "start_time must be number")
	}
	endTimeParam := query["end_time"]
	if endTimeParam == nil {
		o.ServeError(http.StatusBadRequest, "end_time cannot be empty")
	}
	endTime, ok := endTimeParam.(float64)
	if !ok {
		o.ServeError(http.StatusBadRequest, "end_time must be number")
	}
	intervalParam := query["interval"]
	if intervalParam == nil {
		o.ServeError(http.StatusBadRequest, "interval cannot be empty")
	}
	interval, ok := intervalParam.(string)
	if !ok {
		o.ServeError(http.StatusBadRequest, "interval must be string")
	}
	timeZoneParam := query["time_zone"]
	if timeZoneParam == nil {
		o.ServeError(http.StatusBadRequest, "time_zone cannot be empty")
	}
	timeZone, ok := timeZoneParam.(string)
	if !ok {
		o.ServeError(http.StatusBadRequest, "time_zone must be string")
	}
	isValidInterval := false
	for index := range intervals {
		if interval == intervals[index] {
			isValidInterval = true
		}
	}
	if !isValidInterval {
		o.ServeError(http.StatusBadRequest, "the interval must be in"+fmt.Sprintf("%v", intervals))
	}
	appIdParam := query["app_id"]
	if appIdParam == nil {
		appIdParam = "*"
	}
	appId, ok := appIdParam.(string)
	if !ok {
		o.ServeError(http.StatusBadRequest, "app_id must be string")
	}
	_, err = models.GetAppById(appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get app", err)
	}
	err, result := models.GetHistoryRequestSum(int64(startTime), int64(endTime), interval, timeZone, appId)
	if err != nil {
		o.ServeError(http.StatusBadRequest, "failed to get request sum form ES", err)
	}
	o.Serve(result)

}
