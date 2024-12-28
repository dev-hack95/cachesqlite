package helper

import (
	"cahcesqlite/client"
	"log"
	"os"
	"os/signal"
	"syscall"

	"github.com/gin-gonic/gin"
)

func RunServer(host string, conn *client.Connection, r *gin.Engine) {
	cleanup := make(chan os.Signal, 1)
	signal.Notify(cleanup, os.Interrupt, syscall.SIGTERM)

	go func() {
		if err := r.Run("0.0.0.0:8000"); err != nil {
			log.Printf("Server error: %v", err)
			cleanup <- os.Interrupt // Trigger cleanup on error
		}
	}()

	conn.Cleanup()

	<-cleanup
	log.Println("Server shutting down...")
}
