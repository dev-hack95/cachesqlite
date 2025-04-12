package main

import (
	"time"

	"github.com/dev-hack95/cachesqlite/client"
	"github.com/dev-hack95/cachesqlite/helper"

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
		key := c.Request.URL.Query().Get("key")
		data, err := conn.Get(key)
		if err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
			return
		}

		if data == "" {
			c.JSON(400, gin.H{"error": "no record found"})
			return
		}
		c.JSON(200, gin.H{"value": data})
	})

	r.GET("/set/value", func(c *gin.Context) {
		key := c.Request.URL.Query().Get("key")
		value := c.Request.URL.Query().Get("value")
		err := conn.Set(key, value, 10*time.Minute)
		if err != nil {
			c.JSON(400, gin.H{"error": err.Error()})
		}
		c.JSON(200, gin.H{"status": "success"})
	})

	helper.RunServer("0.0.0.0:8000", conn, r)
}
