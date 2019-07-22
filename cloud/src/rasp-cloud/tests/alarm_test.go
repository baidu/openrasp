package test

import (
	_ "rasp-cloud/tests/start"
	"testing"
	. "github.com/smartystreets/goconvey/convey"
	"rasp-cloud/models"
	"rasp-cloud/tests/start"
	"time"
	"github.com/bouk/monkey"
	"reflect"
	"github.com/astaxie/beego/httplib"
	"net/http"
	"net/smtp"
	"crypto/tls"
	"net"
	"path/filepath"
	"rasp-cloud/tools"
	"io"
	"rasp-cloud/models/logs"
	"errors"
	"rasp-cloud/mongo"
	"os"
	"html/template"
)

type writerCloser struct {
}

func (*writerCloser) Write([]byte) (n int, err error) {
	return 1, nil
}

func (*writerCloser) Close() error {
	return nil
}

func TestAttackAlarmPush(t *testing.T) {
	Convey("Subject: Test Alarm Push\n", t, func() {

		start.TestApp.EmailAlarmConf = models.EmailAlarmConf{
			Enable:     true,
			ServerAddr: "qq.smtp.com:25",
			UserName:   "test",
			Password:   "test",
			Subject:    "test",
			RecvAddr:   []string{"test@openrasp.com"},
			TlsEnable:  false,
		}

		start.TestApp.HttpAlarmConf = models.HttpAlarmConf{
			Enable:   true,
			RecvAddr: []string{"http://openrasp.com/alarm"},
		}

		start.TestApp.DingAlarmConf = models.DingAlarmConf{
			Enable:     true,
			AgentId:    "manager6632",
			CorpId:     "ding70235c2f4657eb6378f",
			CorpSecret: "123456789",
			RecvUser:   []string{"2263285838022"},
			RecvParty:  []string{"92843"},
		}

		alarms := []map[string]interface{}{
			{
				"attack_type": "sql",
				"event_time":  time.Now().String(),
			},
		}

		monkey.PatchInstanceMethod(reflect.TypeOf(&httplib.BeegoHTTPRequest{}), "Response",
			func(*httplib.BeegoHTTPRequest) (*http.Response, error) {
				return &http.Response{StatusCode: 200}, nil
			},
		)

		monkey.PatchInstanceMethod(reflect.TypeOf(&httplib.BeegoHTTPRequest{}), "Bytes",
			func(*httplib.BeegoHTTPRequest) ([]byte, error) {
				return []byte(`{"errcode":0,"errmsg":"ok","access_token":"1as35f15we1f5"}`), nil
			},
		)

		monkey.PatchInstanceMethod(reflect.TypeOf(&httplib.BeegoHTTPRequest{}), "Bytes",
			func(*httplib.BeegoHTTPRequest) ([]byte, error) {
				return []byte(`{"errcode":0,"errmsg":"ok","access_token":"1as35f15we1f5"}`), nil
			},
		)

		apppath, _ := filepath.Abs(filepath.Dir("./"))
		monkey.Patch(tools.GetCurrentPath, func() (string, error) {
			return apppath, nil
		})

		monkey.Patch(smtp.PlainAuth, func(identity, username, password, host string) smtp.Auth {
			return nil
		})

		monkey.Patch(tls.DialWithDialer, func(*net.Dialer, string, string, *tls.Config) (*tls.Conn, error) {
			return &tls.Conn{}, nil
		})

		monkey.Patch(smtp.NewClient, func(conn net.Conn, host string) (*smtp.Client, error) {
			return &smtp.Client{}, nil
		})

		monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Extension",
			func(*smtp.Client, string) (bool, string) {
				return true, ""
			},
		)

		monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Auth",
			func(*smtp.Client, smtp.Auth) error {
				return nil
			},
		)

		monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Mail",
			func(*smtp.Client, string) error {
				return nil
			},
		)

		monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Rcpt",
			func(*smtp.Client, string) error {
				return nil
			},
		)

		monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Data",
			func(*smtp.Client) (io.WriteCloser, error) {
				return &writerCloser{}, nil
			},
		)

		monkey.Patch(smtp.SendMail, func(string, smtp.Auth, string, []string, []byte) error {
			return nil
		})

		Convey("when the alarm is for testing ", func() {
			models.PushAttackAlarm(start.TestApp, 1, alarms, true)
		})

		Convey("when the alarm is for attack", func() {
			models.PushAttackAlarm(start.TestApp, 1, alarms, false)
		})

		Convey("when the email start with tls", func() {
			start.TestApp.EmailAlarmConf.TlsEnable = true
			start.TestApp.EmailAlarmConf.ServerAddr = "smtp.rasp.com"
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldEqual, nil)
		})

		Convey("when then the func of get hostname has errors", func() {
			start.TestApp.EmailAlarmConf.From = ""
			monkey.Patch(os.Hostname, func() (name string, err error) {
				return "", errors.New("")
			})
			defer monkey.Unpatch(os.Hostname)
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldEqual, nil)
		})

		Convey("when the email subject is empty", func() {
			start.TestApp.EmailAlarmConf.Subject = ""
			start.TestApp.EmailAlarmConf.ServerAddr = "smtp.rasp.com"
			start.TestApp.EmailAlarmConf.Password = ""
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldEqual, nil)
			start.TestApp.EmailAlarmConf.Subject = "openrasp"
		})

		Convey("when tls dialog has errors", func() {
			monkey.Patch(tls.DialWithDialer, func(*net.Dialer, string, string, *tls.Config) (*tls.Conn, error) {
				return &tls.Conn{}, errors.New("")
			})
			defer monkey.Unpatch(tls.DialWithDialer)
			start.TestApp.EmailAlarmConf.TlsEnable = true
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when the creation of tls client has errors", func() {
			monkey.Patch(smtp.NewClient, func(conn net.Conn, host string) (*smtp.Client, error) {
				return &smtp.Client{}, errors.New("")
			})
			defer monkey.Unpatch(smtp.NewClient)
			start.TestApp.EmailAlarmConf.TlsEnable = true
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when tls client auth has error ", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Auth",
				func(*smtp.Client, smtp.Auth) error {
					return errors.New("")
				},
			)
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Auth")
			start.TestApp.EmailAlarmConf.TlsEnable = true
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when tls client mail has error ", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Mail",
				func(*smtp.Client, string) error {
					return errors.New("")
				},
			)
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Mail")
			start.TestApp.EmailAlarmConf.TlsEnable = true
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when tls client rcpt has error ", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Rcpt",
				func(*smtp.Client, string) error {
					return errors.New("")
				},
			)
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Rcpt")
			start.TestApp.EmailAlarmConf.TlsEnable = true
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when tls client data has error ", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Data",
				func(*smtp.Client) (io.WriteCloser, error) {
					return &writerCloser{}, errors.New("")
				},
			)
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&smtp.Client{}), "Data")
			start.TestApp.EmailAlarmConf.TlsEnable = true
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when normal smtp has error ", func() {
			monkey.Patch(smtp.SendMail, func(string, smtp.Auth, string, []string, []byte) error {
				return errors.New("")
			})
			defer monkey.Unpatch(smtp.SendMail)
			start.TestApp.EmailAlarmConf.TlsEnable = false
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when html template error ", func() {
			monkey.Patch(template.ParseFiles, func(filenames ...string) (*template.Template, error) {
				return nil, errors.New("")
			})
			defer monkey.Unpatch(template.ParseFiles)
			start.TestApp.EmailAlarmConf.TlsEnable = false
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when template render has error ", func() {
			monkey.PatchInstanceMethod(reflect.TypeOf(&template.Template{}), "Execute",
				func(*template.Template, io.Writer, interface{}) error {
					return errors.New("")
				},
			)
			defer monkey.UnpatchInstanceMethod(reflect.TypeOf(&template.Template{}), "Execute")
			start.TestApp.EmailAlarmConf.TlsEnable = false
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			So(err, ShouldNotEqual, nil)
		})

		Convey("when the email server address is empty", func() {
			start.TestApp.EmailAlarmConf.TlsEnable = false
			start.TestApp.EmailAlarmConf.ServerAddr = ""
			err := models.PushEmailAttackAlarm(start.TestApp, 1, alarms, false)
			start.TestApp.EmailAlarmConf.ServerAddr = "smtp.rasp.com"
			So(err, ShouldNotEqual, nil)
		})

	})
}

func TestAlarmDetect(t *testing.T) {
	Convey("Subject: Test Alarm Detect\n", t, func() {
		Convey("when es has new alarms", func() {
			monkey.Patch(logs.SearchLogs, func(int64, int64, bool, map[string]interface{}, string,
				int, int, bool, ...string) (int64, []map[string]interface{}, error) {
				return 1, []map[string]interface{}{}, nil
			})
			defer monkey.Unpatch(logs.SearchLogs)
			models.HandleAttackAlarm()
		})

		Convey("when es has error", func() {
			monkey.Patch(logs.SearchLogs, func(int64, int64, bool, map[string]interface{}, string,
				int, int, bool, ...string) (int64, []map[string]interface{}, error) {
				return 1, []map[string]interface{}{}, errors.New("")
			})
			defer monkey.Unpatch(logs.SearchLogs)
			models.HandleAttackAlarm()
		})

		Convey("when es has not new alarms", func() {
			models.HandleAttackAlarm()
		})

		Convey("when mongodb has error", func() {
			monkey.Patch(mongo.FindAllWithSelect, func(string, interface{}, interface{}, interface{},
				int, int) (count int, err error) {
				return 0, errors.New("")
			})
			defer monkey.Unpatch(mongo.FindAllWithSelect)
			models.HandleAttackAlarm()
		})
	})
}
