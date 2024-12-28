package main

import (
	"cahcesqlite/client"
	"cahcesqlite/helper"

	"github.com/gin-gonic/gin"
)

var conn *client.Connection

func init() {
	conn = client.NewConnection()
	if err := conn.InitDatabase("redis.db"); err != nil {
		panic(err)
	}
}

func main() {
	r := gin.Default()

	helper.RunServer("0.0.0.0:8000", conn, r)
}
