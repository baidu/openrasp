package tools

import "time"

type CronTabTime struct {
	Year   int
	Month  int
	Day    int
	Hour   int
	Min    int
	Sec    int
	Nsec   int
}

func CronTabTimer(f func(), cron *CronTabTime, day int) {
	now := time.Now()
	// 获取第二天时间
	next := now.Add(time.Hour * 24 * time.Duration(day))
	next = time.Date(next.Year(), next.Month(), next.Day(), cron.Hour, cron.Min, cron.Sec, cron.Nsec, next.Location())
	t := time.NewTimer(next.Sub(now))
	go func() {
		for {
			select {
			case <- t.C:
				f()
			}
			t.Reset(time.Hour * 24 * time.Duration(day))
		}
	}()
}
