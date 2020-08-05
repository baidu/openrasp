package test

import (
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/tests/inits"
	_ "rasp-cloud/tests/start"
	"rasp-cloud/models"
	"github.com/bouk/monkey"
	"github.com/pkg/errors"
	"golang.org/x/crypto/bcrypt"
	"github.com/astaxie/beego/context"
	"reflect"
	"rasp-cloud/mongo"
)

func TestUserLogin(t *testing.T) {
	Convey("Subject: Test Logout Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/user/login", inits.GetJson(map[string]interface{}{
				"username": "openrasp",
				"password": "admin@123",
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the mongodb has errors", func() {
			monkey.Patch(models.NewCookie, func(id string, userId string) error {
				return errors.New("")
			})
			r := inits.GetResponse("POST", "/v1/user/login", inits.GetJson(map[string]interface{}{
				"username": "openrasp",
				"password": "admin@123",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
			monkey.Unpatch(models.NewCookie)
		})

		Convey("when the username or password is empty", func() {
			r := inits.GetResponse("POST", "/v1/user/login", inits.GetJson(map[string]interface{}{
				"username": "",
				"password": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the length of username or password is greater than 512", func() {
			r := inits.GetResponse("POST", "/v1/user/login", inits.GetJson(map[string]interface{}{
				"username": inits.GetLongString(513),
				"password": inits.GetLongString(513),
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the username or password is incorrect", func() {
			r := inits.GetResponse("POST", "/v1/user/login", inits.GetJson(map[string]interface{}{
				"username": "123456789",
				"password": "12345612155",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("test islogin api", func() {
			r := inits.GetResponse("POST", "/v1/user/islogin", inits.GetJson(map[string]interface{}{}))
			So(r.Status, ShouldEqual, 0)
		})
	})
}

func TestUserLogout(t *testing.T) {
	Convey("Subject: Test Logout Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/user/logout", inits.GetJson(map[string]interface{}{}))
			So(r.Status, ShouldEqual, 0)
		})
	})
}

func TestUserUpdate(t *testing.T) {
	Convey("Subject: Test User Update Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/user/update", inits.GetJson(map[string]interface{}{
				"old_password": "admin@123",
				"new_password": "admin@123",
			}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when the old_password is empty", func() {
			r := inits.GetResponse("POST", "/v1/user/update", inits.GetJson(map[string]interface{}{
				"old_password": "",
				"new_password": "admin@123",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the new_password is empty", func() {
			r := inits.GetResponse("POST", "/v1/user/update", inits.GetJson(map[string]interface{}{
				"old_password": "admin@123",
				"new_password": "",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when the password format is invalid", func() {
			r := inits.GetResponse("POST", "/v1/user/update", inits.GetJson(map[string]interface{}{
				"old_password": "admin@123",
				"new_password": "admin",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when mongo has errors", func() {
			monkey.Patch(models.RemoveAllCookie, func() (error) {
				return errors.New("")
			})
			defer monkey.Unpatch(models.RemoveAllCookie)

			r := inits.GetResponse("POST", "/v1/user/update", inits.GetJson(map[string]interface{}{
				"old_password": "admin@123",
				"new_password": "admin@123",
			}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("test reset user", func() {
			err := models.ResetUser("admin@123")
			So(err, ShouldEqual, nil)

			err = models.ResetUser("123")
			So(err, ShouldNotEqual, nil)

			monkey.Patch(bcrypt.GenerateFromPassword, func(password []byte, cost int) ([]byte, error) {
				return nil, errors.New("")
			})
			err = models.ResetUser("admin@123")
			So(err, ShouldNotEqual, nil)

			monkey.Patch(bcrypt.CompareHashAndPassword, func(hashedPassword, password []byte) error {
				return errors.New("")
			})
			err = models.ComparePassword("admin@123", "admin@123")
			So(err, ShouldNotEqual, nil)
			monkey.Unpatch(bcrypt.CompareHashAndPassword)

			monkey.Patch(mongo.FindId, func(string, string, interface{}) error {
				return errors.New("")
			})
			_, err = models.GetLoginUserName()
			So(err, ShouldNotEqual, nil)
			_, err = models.VerifyUser("openrasp", "admin@123")
			So(err, ShouldNotEqual, nil)
			err = models.UpdatePassword("admin@123", "admin@123")
			So(err, ShouldNotEqual, nil)
			monkey.Unpatch(mongo.FindId)

			err = models.UpdatePassword("admin@123", "admin@123")
			So(err, ShouldNotEqual, nil)
			monkey.Unpatch(bcrypt.GenerateFromPassword)
		})

	})
}

func TestCheckDefault(t *testing.T)  {
	Convey("Subject: Test Check Default Api\n", t, func() {
		Convey("when the param is valid", func() {
			r := inits.GetResponse("POST", "/v1/user/default", inits.GetJson(map[string]interface{}{}))
			So(r.Status, ShouldEqual, 0)
		})

		Convey("when mongo has errors", func() {
			monkey.Patch(models.CheckDefaultPasswordWithDefaultUser, func() (result bool, err error) {
				return false, errors.New("")
			})
			defer monkey.Unpatch(models.CheckDefaultPasswordWithDefaultUser)

			monkey.Patch(models.CheckDefaultPassword, func(cookie string) (result bool, err error) {
				return false, errors.New("")
			})
			defer monkey.Unpatch(models.CheckDefaultPassword)

			r := inits.GetResponse("POST", "/v1/user/default", inits.GetJson(map[string]interface{}{}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})

		Convey("when cookie is not null", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&context.Context{}), "GetCookie",
				func(*context.Context, string) string {
					return "111111"
				})
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&context.Context{}), "GetCookie")

			r := inits.GetResponse("POST", "/v1/user/default", inits.GetJson(map[string]interface{}{}))
			So(r.Status, ShouldBeGreaterThan, 0)
		})
	})
}