package controllers

type PingController struct {
	BaseController
}

// @router / [get,post]
func (o *PingController) Ping() {
	o.ServeWithEmptyData()
}
