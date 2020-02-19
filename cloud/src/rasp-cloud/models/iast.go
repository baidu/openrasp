package models

import (
	"sync"
)

type Iast struct {
	CurrentPage 	  int               `json:"current_page"`
	AppId             string 		    `json:"app_id"`
}

// 客户端读写消息
type WsMessage struct {
	MessageType int
	Data        []byte
}

type IastRegister struct {
	Data map[string]uint8
	Lock sync.Mutex
}

type IastAppId struct {
	Data map[string]bool
	Lock sync.RWMutex
}

var (
	appId          string
	InChan         = make(chan *WsMessage, 1000)
	OutChan        = make(chan *WsMessage, 1000)
	ResChan        = make(chan []byte)           // 读IAST返回结果
	CloseChan      = make(chan byte)
	IsClosed       = false
	Register       = &IastRegister{
		Data: map[string]uint8{
			appId: 0,
		},
	}
	IastApp        = &IastAppId{
		Data: map[string]bool{
			appId: false,
		},
	}
)

func (iast IastRegister) GetIastRegister(k string) uint8 {
	iast.Lock.Lock()
	defer iast.Lock.Unlock()
	return iast.Data[k]
}

func (iast IastRegister) SetIastRegister(k string, v uint8) {
	iast.Lock.Lock()
	defer iast.Lock.Unlock()
	iast.Data[k] = v
}

func (iastApp IastAppId) GetIastAppId(k string) bool {
	iastApp.Lock.RLock()
	defer iastApp.Lock.RLock()
	return iastApp.Data[k]
}

func (iastApp IastAppId) SetIastAppId(k string, v bool) {
	iastApp.Lock.Lock()
	defer iastApp.Lock.Unlock()
	iastApp.Data[k] = v
}