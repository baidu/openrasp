package test

import (
	"testing"
	_ "rasp-cloud/tests/inits"
	_ "rasp-cloud/tests/start"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/es"
)

func TestDeleteLogs(t *testing.T) {
	Convey("Subject: Test Delete Logs\n", t, func() {
		Convey("when failed to delete expired data for index", func() {
			es.DeleteLogs("12345")
		})
	})
}

func TestBulkInsertAlarm(t *testing.T) {
	Convey("Subject: Test Bulk Insert Alarm\n", t, func() {
		Convey("when failed to get app_id param from alarm", func() {
			es.BulkInsertAlarm("test", []map[string]interface{}{
				{"app_id": nil},
			})
		})
		Convey("when the type of alarm's app_id param is not string", func() {
			es.BulkInsertAlarm("test", []map[string]interface{}{
				{"app_id": 1111},
			})
		})
	})
}