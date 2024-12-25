package helper

import (
	"cahcesqlite/cachesqlitelocal"
	"log"
	"os"
	"os/signal"
	"syscall"

	"github.com/gin-gonic/gin"
)

func RunServer(host string, conn *cachesqlitelocal.Connection, r *gin.Engine) {
	cleanup := make(chan os.Signal, 1)
	signal.Notify(cleanup, os.Interrupt, syscall.SIGTERM)

	defer conn.Cleanup()

	go func() {
		if err := r.Run("0.0.0.0:8000"); err != nil {
			log.Printf("Server error: %v", err)
			cleanup <- os.Interrupt // Trigger cleanup on error
		}
	}()

	<-cleanup
	log.Println("Server shutting down...")
}
