package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"github.com/bouk/monkey"
	"reflect"
	"gopkg.in/mgo.v2"
	"github.com/pkg/errors"
	"rasp-cloud/mongo"
	"rasp-cloud/models"
	"rasp-cloud/tests/start"
)

func TestMongoErr(t *testing.T) {
	Convey("Subject: Test Mongo Err\n", t, func() {
		monkey.PatchInstanceMethod(reflect.TypeOf(&mgo.Query{}), "Count", func(*mgo.Query) (int, error) {
			return 0, errors.New("")
		})
		defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&mgo.Query{}), "Count")

		_, err := mongo.FindAll("app", &map[string]string{},
			&map[string]interface{}{}, 0, 1, "name")
		So(err, ShouldNotEqual, nil)

		_, err = mongo.FindAllBySort("app", &map[string]string{},
			0, 1, &map[string]interface{}{}, "name")
		So(err, ShouldNotEqual, nil)

		_, err = mongo.FindAllWithSelect("app", &map[string]string{},
			&map[string]interface{}{}, &map[string]interface{}{}, 0, 1)
		So(err, ShouldNotEqual, nil)

		_, err = mongo.FindAllWithoutLimit("app", &map[string]string{},
			&map[string]interface{}{}, "name")
		So(err, ShouldNotEqual, nil)

		_, err = models.SetSelectedPlugin("sssssss", "sssssss")
		So(err, ShouldNotEqual, nil)

		monkey.Patch(models.GetLoginUserName, func() (userName string, err error) {
			return "", errors.New("")
		})
		err = models.AddOperation(start.TestApp.Id, 1001, "10.10.10.10", "sss")
		So(err, ShouldNotEqual, nil)
		monkey.Unpatch(models.GetLoginUserName)

		monkey.Patch(mongo.Insert, func(collection string, doc interface{}) error {
			return errors.New("")
		})
		err = models.AddOperation(start.TestApp.Id, 1001, "10.10.10.10", "sss")
		So(err, ShouldNotEqual, nil)
		monkey.Unpatch(mongo.Insert)
	})

}
