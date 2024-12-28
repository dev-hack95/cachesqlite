package main

import (
	"cahcesqlite/cachesqlitelocal"
	"cahcesqlite/helper"

	"github.com/gin-gonic/gin"
)

var conn *cachesqlitelocal.Connection

func init() {
	conn = cachesqlitelocal.NewConnection()
	if err := conn.InitDatabase("redis.db"); err != nil {
		panic(err)
	}
}

func main() {
	defer conn.Cleanup()
	r := gin.Default()

	helper.RunServer("0.0.0.0:8000", conn, r)
}
