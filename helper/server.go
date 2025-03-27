package helper

import (
	"log"
	"os"
	"os/signal"
	"syscall"

	"github.com/dev-hack95/cachesqlite/client"
	"github.com/gin-gonic/gin"
)

func RunServer(host string, conn *client.Connection, r *gin.Engine) {
	cleanup := make(chan os.Signal, 1)
	signal.Notify(cleanup, os.Interrupt, syscall.SIGTERM)

	defer conn.Cleanup()

	go func() {
		if err := r.Run(host); err != nil {
			log.Printf("Server error: %v", err)
			cleanup <- os.Interrupt // Trigger cleanup on error
		}
	}()

	<-cleanup
	log.Println("Server shutting down...")
}
