package client

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -lsqlite3
#include "../src/main.c"
*/
import "C"
import (
	"time"
	"unsafe"
)

type Connection struct {
	conn *C.struct_Connection
}

func NewConnection() *Connection {
	return &Connection{
		conn: &C.struct_Connection{},
	}
}

func (c *Connection) InitDatabase(filename string) error {
	cFilename := C.CString(filename)
	defer C.free(unsafe.Pointer(cFilename))

	C.init_database(c.conn, cFilename)
	C.merge_database(c.conn)
	return nil
}

func (c *Connection) Set(key string, value string, t time.Duration) error {
	cKey := C.CString(key)
	cValue := C.CString(value)
	defer C.free(unsafe.Pointer(cKey))
	defer C.free(unsafe.Pointer(cValue))

	C.set(c.conn, cKey, cValue, C.long(ConvertToSeconds(t)))
	return nil
}

func (c *Connection) Get(key string) (string, error) {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	data := C.get(c.conn, cKey)

	return C.GoString(data), nil
}

func (c *Connection) Del(key string) error {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	C.del(c.conn, cKey)

	return nil
}

func (c *Connection) Cleanup() {
	C.memdb_to_disk_transfer(c.conn)
}

func ConvertToSeconds(d time.Duration) int64 {
	return int64(d / time.Second)
}
