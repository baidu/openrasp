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

package agent

import (
	"encoding/json"
	"errors"
	"github.com/astaxie/beego"
	"github.com/gorilla/websocket"
	"net/http"
	"rasp-cloud/controllers"
	"rasp-cloud/models"
	"strings"
	"sync"
	"time"
)

type WebsocketController struct {
	controllers.BaseController
}

var (
	wsUpgrader = websocket.Upgrader{
		ReadBufferSize:    4096,
		WriteBufferSize:   4096,
		HandshakeTimeout:  5 * time.Second,
		CheckOrigin: func(r *http.Request) bool {
			return true
		},
	}
	InChan    = make(chan *wsMessage, 1000)
	OutChan   = make(chan *wsMessage, 1000)
	resChan   = make(chan []byte)           // 读IAST返回结果
	CloseChan = make(chan byte)
	IsClosed  = false
)

// 客户端读写消息
type wsMessage struct {
	messageType int
	data        []byte
}

type wsConnection struct {
	wsSocket  *websocket.Conn // 底层websocket
	inChan    chan *wsMessage // 写IAST
	outChan   chan *wsMessage // 读IAST
	mutex     sync.Mutex // Mutex互斥锁，避免重复关闭管道
	isClosed  bool
	closeChan chan byte // 关闭通知
}

func (wsConn *wsConnection) wsReadLoop() {
	for {
		msgType, data, err := wsConn.wsSocket.ReadMessage()
		if err != nil {
			goto error
		}
		dataStr := string(data)
		if len(dataStr) != 0 {
			req := &wsMessage{}
			if string(data) == "startup" {
				req = &wsMessage{
					msgType,
					[]byte("=== send heartbeat and building ws connections ==="),
				}
			} else {
				req = &wsMessage{
					msgType,
					[]byte(dataStr),
				}
			}
			// 放入请求队列
			select {
			case wsConn.inChan <- req:
			case <-wsConn.closeChan:
				goto closed
			}
		}
	}
error:
	wsConn.wsClose()
closed:
	beego.Info("websocket is closed.")
}

func (wsConn *wsConnection) wsWriteLoop() {
	for {
		select {
		// 取一个应答
		case msg := <-wsConn.outChan:
			// 写给websocket
			if err := wsConn.wsSocket.WriteMessage(msg.messageType, msg.data); err != nil {
				goto error
			}
		case <-wsConn.closeChan:
			goto closed
		}
	}
error:
	wsConn.wsClose()
closed:
	beego.Info("websocket is closed.")
}

func (wsConn *wsConnection) procLoop() {
	go func() {
		for {
			time.Sleep(3 * time.Second)
			if err := wsConn.wsWrite(websocket.TextMessage, []byte("heartbeat from OpenRASP cloud")); err != nil {
				beego.Error("heartbeat fail")
				wsConn.wsClose()
				break
			}
		}
	}()

	for {
		msg, err := wsConn.wsRead()
		if err != nil {
			beego.Error("read fail")
			break
		}
		if strings.Index(string(msg.data), "status") == -1 {
			err = wsConn.wsWrite(msg.messageType, msg.data)
			if err != nil {
				beego.Error("write fail")
				break
			}
		} else {
			resChan <- msg.data
		}
	}
}

// @router / [get]
func (o *WebsocketController) Get() {
	wsSocket, err := wsUpgrader.Upgrade(o.Ctx.ResponseWriter, o.Ctx.Request, nil)
	if err != nil {
		beego.Error("upgrade err:", err)
	}
	wsConn := &wsConnection{
		wsSocket:  wsSocket,
		inChan:    InChan,
		outChan:   OutChan,
		closeChan: CloseChan,
		isClosed:  IsClosed,
	}

	// 处理器
	go wsConn.procLoop()
	// 读协程
	go wsConn.wsReadLoop()
	// 写协程
	go wsConn.wsWriteLoop()
}

// @router / [post]
func (o *WebsocketController) Post() {
	var param struct {
		Order string         `json:"order"`
		Data  *models.Iast   `json:"data" `
	}
	o.UnmarshalJson(&param)

	if param.Data == nil {
		o.ServeError(http.StatusBadRequest, "search data can not be empty")
	}
	if param.Order == "" {
		o.ServeError(http.StatusBadRequest, "order can not be empty")
	}
	if strings.Index(o.Ctx.Input.Header("Content-Type"), "application/json") == -1 {
		o.ServeError(http.StatusUnsupportedMediaType, "Unsupported Media Type")
	}

	wsConn := &wsConnection{
		inChan:    InChan,
		outChan:   OutChan,
		closeChan: CloseChan,
		isClosed:  IsClosed,
	}

	if err := wsConn.wsWrite(websocket.TextMessage, o.Ctx.Input.RequestBody); err != nil {
		beego.Error("send msg from web failed!")
		wsConn.wsClose()
	}

	select {
	// 取一个应答
	case msg := <-resChan:
		if string(msg) != "" {
			var result = make(map[string]interface{})
			if err := json.Unmarshal(msg, &result); err == nil {
				o.Serve(result)
			} else {
				o.ServeError(http.StatusBadRequest, "Invalid JSON from iast", err)
			}
			beego.Info("msg:", string(msg))
		}
	}
}

func (wsConn *wsConnection) wsWrite(messageType int, data []byte) error {
	select {
	case wsConn.outChan <- &wsMessage{messageType, data}:
	case <-wsConn.closeChan:
		return errors.New("websocket closed")
	}
	return nil
}

func (wsConn *wsConnection) wsRead() (*wsMessage, error) {
	select {
	case msg := <-wsConn.inChan:
		return msg, nil
	case <-wsConn.closeChan:
	}
	return nil, errors.New("websocket closed")
}

func (wsConn *wsConnection) wsClose() {
	wsConn.wsSocket.Close()
	wsConn.mutex.Lock()
	defer wsConn.mutex.Unlock()
	if !wsConn.isClosed {
		wsConn.isClosed = true
		close(wsConn.closeChan)
	}
}
