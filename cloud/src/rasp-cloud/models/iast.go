package models

type Iast struct {
	CurrentPage 	  int               `json:"current_page"`
}

// 客户端读写消息
type WsMessage struct {
	MessageType int
	Data        []byte
}

var (
	InChan    = make(chan *WsMessage, 1000)
	OutChan   = make(chan *WsMessage, 1000)
	ResChan   = make(chan []byte)           // 读IAST返回结果
	CloseChan = make(chan byte)
	IsClosed  = false
)

//func init()  {
//	w := &WsMessage{
//		MessageType: 0,
//		Data:        nil,
//	}
//	InChan <- w
//}