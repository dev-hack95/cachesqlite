package client

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -lsqlite3
#include "../src/main.c"
*/
import "C"
import "unsafe"

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

func (c *Connection) TestInsert() {
	C.test_insert(c.conn)
}

func (c *Connection) Cleanup() {
	C.memdb_to_disk_transfer(c.conn)
}
