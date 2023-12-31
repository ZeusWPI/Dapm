package main

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"strconv"
	"strings"
	"time"
	"unicode"

	piondtls "github.com/pion/dtls/v2"
	"github.com/plgd-dev/go-coap/v3/dtls"
	"github.com/plgd-dev/go-coap/v3/message"
	"github.com/plgd-dev/go-coap/v3/udp/client"
)

func initCoap() (conn *client.Conn) {
	address := concatStrings(conf.CoapIp, ":", conf.CoapPort)

	var err error
	conn, err = dtls.Dial(address, &piondtls.Config{
		PSK: func(_ []byte) ([]byte, error) {
			return []byte(conf.CoapKey), nil
		},
		PSKIdentityHint: []byte(conf.CoapIdentity),
		CipherSuites:    []piondtls.CipherSuiteID{piondtls.TLS_PSK_WITH_AES_128_CCM_8},
	})
	check(err)

	return
}

func initContext() (ctx context.Context) {
	ctx = context.Background()

	return
}

func changeBrightness(path string, payload string) (err error) {
	globals.mu.Lock()

	ctx, cancel := context.WithTimeout(globals.ctx, time.Second)
	defer cancel()

	_, err = globals.conn.Put(
		ctx,
		path,
		message.AppJSON,
		bytes.NewReader([]byte(payload)),
	)

	globals.mu.Unlock()

	return
}

func changeBrightnessRetries(path string, payload string) (err error) {
	for i := 0; i < conf.CoapRetries; i++ {
		err := changeBrightness(path, payload)
		if !checkNonFatal(err) {
			return err
		}
		time.Sleep(200 * time.Millisecond)
	}

	return
}

func getBrightness(path string) (number int, err error) {
	globals.mu.Lock()

	number = -1

	resp, err := globals.conn.Get(
		globals.ctx,
		path,
	)

	globals.mu.Unlock()

	isError := checkNonFatal(err)
	if isError {
		return
	}

	body := resp.Body()
	buf := new(strings.Builder)

	_, err = io.Copy(buf, body)
	isError = checkNonFatal(err)
	if isError {
		return
	}

	_, after, _ := strings.Cut(buf.String(), "5851\":")

	for i, c := range after {
		if !unicode.IsDigit(c) {
			number, err = strconv.Atoi(after[:i])
			checkNonFatal(err)
			break
		}
	}

	return
}

func blink() {
	brightness, err := getBrightness(conf.CoapRandomLed)
	if checkNonFatal(err) {
		return
	}

	var newBrightness int
	if brightness < conf.BlinkCutOff {
		newBrightness = int(254 -  (float64(brightness) * conf.BlinkMultiplier))
	} else {
		newBrightness = int(float64(brightness) * conf.BlinkMultiplier)
	}

	for i := 0; i < conf.BlinkAmount; i++ {
		err := changeBrightness(conf.CoapKelder, fmt.Sprintf(`{"5851": %d}`, newBrightness))

		if checkNonFatal(err) {
			changeBrightnessRetries(conf.CoapKelder, fmt.Sprintf(`{"5851": %d}`, brightness))
			return
		}

		time.Sleep(300 * time.Millisecond)
		err = changeBrightness(conf.CoapKelder, fmt.Sprintf(`{"5851": %d}`, brightness))

		if checkNonFatal(err) {
			changeBrightnessRetries(conf.CoapKelder, fmt.Sprintf(`{"5851": %d}`, brightness))
			return
		}

		time.Sleep(300 * time.Millisecond)
	}
}

func led(content *POSTContent) {
	for _, light := range content.Lights {
		changeBrightness(fmt.Sprintf(`/15001/%d`, light), fmt.Sprintf(`{ "3311": [{ "5851": %d }]}`, content.Brightness))
	}
}

func group(content *POSTContent) {
	changeBrightness(fmt.Sprintf(`/15004/%d`, content.Group), fmt.Sprintf(`{ "5851": %d }`, content.Brightness))
}
