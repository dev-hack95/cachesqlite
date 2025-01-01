package main

import (
	"cahcesqlite/client"
	"cahcesqlite/helper"
	"time"

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

	r.GET("/get/value", func(c *gin.Context) {
		data, _ := conn.Get("key1")
		c.JSON(200, gin.H{"value": data})
	})

	r.GET("/set/value", func(c *gin.Context) {
		err := conn.Set("key1", "value1", 10*time.Minute)
		if err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
		}
		c.JSON(200, gin.H{"status": "success"})
	})

	helper.RunServer("0.0.0.0:8000", conn, r)
}
