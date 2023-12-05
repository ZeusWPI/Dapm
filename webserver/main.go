package main

import (
	"context"
	"log"
	"sync"

	udpClient "github.com/plgd-dev/go-coap/v3/udp/client"
)

type SafeGlobals struct {
	mu   sync.Mutex
	conn *udpClient.Conn
	ctx  context.Context
}

var globals SafeGlobals = SafeGlobals{}
var conf Configuration = parseConfig()

func main() {
	log.Println("Setting up connection...")
	globals.conn = initCoap()
	log.Println("Creating context...")
	globals.ctx = initContext()

	defer globals.conn.Close()

	log.Println("Starting webserver...")

	initWebserver()
	startWebserver()

}
