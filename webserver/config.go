package main

import (
	"encoding/json"
	"os"
)

type Configuration struct {
	CoapIp          string  `json:"coap_ip"`
	CoapPort        string  `json:"coap_port"`
	CoapKey         string  `json:"coap_key"`
	CoapIdentity    string  `json:"coap_identity"`
	CoapKelder      string  `json:"coap_kelder"`
	CoapRandomLed   string  `json:"coap_random_led"`
	CoapRetries     int     `json:"coap_retries"`
	BlinkMultiplier float64 `json:"blink_multiplier"`
	BlinkAmount     int     `json:"blink_amount"`
	BlinkCutOff     int     `json:"blink_cut_off"`
	WebPort         string  `json:"web_port"`
}

func parseConfig() (conf Configuration) {
	file, err := os.Open("config.json")
	check(err)

	json.NewDecoder(file).Decode(&conf)

	return
}
