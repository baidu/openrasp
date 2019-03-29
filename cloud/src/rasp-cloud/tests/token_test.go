package test

import (
	"testing"
	"rasp-cloud/tests/inits"
	. "github.com/smartystreets/goconvey/convey"
	"github.com/bouk/monkey"
	"rasp-cloud/models"
	"errors"
	"reflect"
	"github.com/astaxie/beego/context"
)

func TestGetToken(t *testing.T) {
	Convey("Subject: Test Get Token Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/token/get", inits.GetJson(map[string]interface{}{
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongo has error", func() {
			monkey.Patch(models.GetAllToken, func(int, int) (int, []*models.Token, error) {
				return 0, nil, errors.New("")
			})
			defer monkey.Unpatch(models.GetAllToken)
			r := inits.GetResponse("POST", "/v1/api/token/get", inits.GetJson(map[string]interface{}{
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the result is nil", func() {
			monkey.Patch(models.GetAllToken, func(int, int) (int, []*models.Token, error) {
				return 0, nil, nil
			})
			defer monkey.Unpatch(models.GetAllToken)
			r := inits.GetResponse("POST", "/v1/api/token/get", inits.GetJson(map[string]interface{}{
				"page":    1,
				"perpage": 1,
			}))
			So(r.Status, ShouldEqual, 0)
			So(r.Data, ShouldNotEqual, nil)
		})
	})
}

func TestPostToken(t *testing.T) {
	Convey("Subject: Test Post Token Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/api/token", inits.GetJson(map[string]interface{}{
				"description": "token 1",
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongo has errors", func() {
			monkey.Patch(models.AddToken, func(*models.Token) (*models.Token, error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(models.AddToken)
			r := inits.GetResponse("POST", "/v1/api/token", inits.GetJson(map[string]interface{}{
				"description": "token 1",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of description is greater than 1024", func() {
			monkey.Patch(models.AddToken, func(*models.Token) (*models.Token, error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(models.AddToken)
			r := inits.GetResponse("POST", "/v1/api/token", inits.GetJson(map[string]interface{}{
				"description": inits.GetLongString(1025),
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestDeleteToken(t *testing.T) {
	Convey("Subject: Test Delete Token Api\n", t, func() {

		Convey("when the param is valid", func() {
			token := &models.Token{
				Token:       "1234567893216549876314",
				Description: "test-token",
			}
			models.AddToken(token)
			r := inits.GetResponse("POST", "/v1/api/token/delete", inits.GetJson(map[string]interface{}{
				"token": token.Token,
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&context.BeegoInput{}), "Header",
				func(input *context.BeegoInput, key string) string {
					return "123456789"
				},
			)
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&context.BeegoInput{}), "Header")
			r := inits.GetResponse("POST", "/v1/api/token/delete", inits.GetJson(map[string]interface{}{
				"token": "123456789",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the token is empty", func() {
			r := inits.GetResponse("POST", "/v1/api/token/delete", inits.GetJson(map[string]interface{}{
				"token": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the token doesn't exist", func() {
			r := inits.GetResponse("POST", "/v1/api/token/delete", inits.GetJson(map[string]interface{}{
				"token": "132010",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}

func TestHasToken(t *testing.T) {
	Convey("Subject: Test Has Token\n", t, func() {
		Convey("when the token exists", func() {
			token := &models.Token{
				Token:       "1234567893216549876314",
				Description: "test-token",
			}
			models.AddToken(token)
			has, _ := models.HasToken(token.Token)
			So(has, ShouldEqual, true)
		})

		Convey("when the token doesn't exist", func() {
			has, _ := models.HasToken("168941058123")
			So(has, ShouldEqual, false)
		})
	})
}
