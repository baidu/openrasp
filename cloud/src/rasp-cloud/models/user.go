//Copyright 2017-2019 Baidu Inc.
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http: //www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

package models

import (
	"github.com/astaxie/beego"
	"github.com/astaxie/beego/logs"
	"github.com/pkg/errors"
	"golang.org/x/crypto/bcrypt"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"os"
	"rasp-cloud/mongo"
	"rasp-cloud/tools"
	"regexp"
	"rasp-cloud/conf"
)

const (
	userCollectionName = "user"
	userName           = "openrasp"
)

type User struct {
	Id       string `bson:"_id"`
	Name     string `json:"name" bson:"name"`
	Password string `json:"password" bson:"password"`
}

var (
	userId string
)

func init() {
	count, err := mongo.Count(userCollectionName)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed, "failed to get the count of user collection", err)
	}
	index := &mgo.Index{
		Key:        []string{"name"},
		Unique:     true,
		Background: true,
		Name:       "name",
	}
	err = mongo.CreateIndex(userCollectionName, index)
	if err != nil {
		tools.Panic(tools.ErrCodeMongoInitFailed, "failed to create name index for user collection", err)
	}
	if count <= 0 {

		hash, err := generateHashedPassword("admin@123")
		if err != nil {
			tools.Panic(tools.ErrCodeGeneratePasswdFailed, "failed to generate the default hashed password", err)
		}
		userId = mongo.GenerateObjectId()
		user := User{
			Id:       userId,
			Name:     userName,
			Password: hash,
		}
		err = mongo.Insert(userCollectionName, user)
		if err != nil {
			tools.Panic(tools.ErrCodeMongoInitFailed, "failed to create default user", err)
		}

	} else {
		var user *User
		err = mongo.FindOne(userCollectionName, bson.M{}, &user)
		if err != nil {
			tools.Panic(tools.ErrCodeMongoInitFailed, "failed to get admin user", err)
		}
		userId = user.Id
	}

	if *conf.AppConfig.Flag.StartType == conf.StartTypeReset {
		if *conf.AppConfig.Flag.Password == "" {
			tools.Panic(tools.ErrCodeResetUserFailed, "the password can not be empty", err)
		}
		err := ResetUser(*conf.AppConfig.Flag.Password)
		if err != nil {
			tools.Panic(tools.ErrCodeResetUserFailed, "failed to reset administrator", err)
		}
		beego.Info("reset the administrator password successfully")
		os.Exit(0)
	}
}

func ResetUser(newPwd string) error {
	err := validPassword(newPwd)
	if err != nil {
		return errors.New("invalid password: " + err.Error())
	}
	pwd, err := generateHashedPassword(newPwd)
	if err != nil {
		return errors.New("failed to generate password: " + err.Error())
	}
	err = mongo.UpdateId(userCollectionName, userId, bson.M{"password": pwd, "name": userName})
	return err
}

func generateHashedPassword(password string) (string, error) {
	hash, err := bcrypt.GenerateFromPassword([]byte(password), bcrypt.DefaultCost)
	if err != nil {
		beego.Error("failed to generate hashed password: " + err.Error())
		return "", err
	}
	return string(hash), nil
}

func ComparePassword(hashedPassword string, password string) error {
	err := bcrypt.CompareHashAndPassword([]byte(hashedPassword), []byte(password))
	if err != nil && err != bcrypt.ErrMismatchedHashAndPassword {
		logs.Error("CompareHashAndPassword function error: " + err.Error())
	}
	return err
}

func validPassword(password string) error {
	hasNum := regexp.MustCompile(".*[0-9].*").Match([]byte(password))
	hasLetter := regexp.MustCompile(".*([a-z]|[A-Z]).*").Match([]byte(password))
	if len(password) < 8 || len(password) > 50 || !hasNum || !hasLetter {
		return errors.New("password must contain both letters and numbers, and the length of password must be between [8, 50]")
	}
	return nil
}

func GetLoginUserName() (userName string, err error) {
	var user *User
	err = mongo.FindId(userCollectionName, userId, &user)
	if err != nil {
		return
	}
	return user.Name, nil
}

//func GetHashedLoginPassword() (pwd string, err error) {
//	var user *User
//	err = mongo.FindId(userCollectionName, userId, &user)
//	if err != nil {
//		return
//	}
//	return user.Password, nil
//}

func VerifyUser(userName string, pwd string) error {
	var user *User
	err := mongo.FindId(userCollectionName, userId, &user)
	if err != nil {
		return err
	}
	if userName != user.Name {
		return errors.New("username is incorrect")
	}
	return ComparePassword(user.Password, pwd)
}

func UpdatePassword(oldPwd string, newPwd string) error {
	err := VerifyUser(userName, oldPwd)
	if err != nil {
		return errors.New("old password is incorrect")
	}
	err = validPassword(newPwd)
	if err != nil {
		return errors.New("Password does not meet complexity requirements: " + err.Error())
	}
	pwd, err := generateHashedPassword(newPwd)
	if err != nil {
		return errors.New("failed to update new password")
	}
	err = mongo.UpdateId(userCollectionName, userId, bson.M{"password": pwd})
	return err
}
