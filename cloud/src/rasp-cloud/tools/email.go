package tools

import "net/smtp"

type loginAuth struct {
	username, password string
}

func LoginAuth(username, password string) smtp.Auth {
	return &loginAuth{username, password}
}

func (auth *loginAuth) Start(server *smtp.ServerInfo) (string, []byte, error) {
	return "LOGIN", []byte(auth.username), nil
}

func (auth *loginAuth) Next(fromServer []byte, more bool) ([]byte, error) {
	if more {
		switch string(fromServer) {
		case "Username:":
			return []byte(auth.username), nil
		case "Password:":
			return []byte(auth.password), nil
		}
	}
	return nil, nil
}
