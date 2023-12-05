package main

import (
	"encoding/json"
	"log"
	"net/http"
)

type POSTContent struct {
	Light      int
	Group      int
	Brightness int
}

func filterPOST(w http.ResponseWriter, r *http.Request) bool {
	switch r.Method {
	case "POST":
		return true
	default:
		w.WriteHeader(http.StatusNotImplemented)
		w.Write([]byte(http.StatusText(http.StatusNotImplemented)))
		return false
	}
}

func errorValidation(w http.ResponseWriter, r *http.Request) (POSTContent, bool) {
	isPost := filterPOST(w, r)
	if !isPost {
		return POSTContent{}, isPost
	}

	var content POSTContent
	err := json.NewDecoder(r.Body).Decode(&content)
	if checkNonFatal(err) || content.Brightness < 0 || content.Brightness > 254 {
		w.WriteHeader(http.StatusUnsupportedMediaType)
		w.Write([]byte(http.StatusText(http.StatusUnsupportedMediaType)))
		return POSTContent{}, false
	}

    log.Println(content)

	return content, true
}

func ledHandler(w http.ResponseWriter, r *http.Request) {
	content, isOk := errorValidation(w, r)

	if !isOk {
		return
	}

	channel := make(chan bool)

	go led(&content, channel)

	result := <-channel

	if result {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("Led changed\n"))
	} else {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte("Error while changing led brightness\n"))
	}
}

func groupHandler(w http.ResponseWriter, r *http.Request) {
	content, isOk := errorValidation(w, r)

	if !isOk {
		return
	}

	channel := make(chan bool)

	go group(&content, channel)

	result := <-channel

	if result {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("Group changed\n"))
	} else {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte("Error while changing group brightness\n"))
	}
}

func blinkHandler(w http.ResponseWriter, r *http.Request) {
	isPost := filterPOST(w, r)
	if !isPost {
		return
	}

	channel := make(chan bool)

	go blink(channel)

	result := <-channel

	if result {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("On Off On Off ...\n"))
	} else {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte("Error while blinking \n"))
	}
}

func initWebserver() {
	http.HandleFunc("/led", ledHandler)
	http.HandleFunc("/group", groupHandler)
	http.HandleFunc("/blink", blinkHandler)
}

func startWebserver() {
	address := concatStrings(":", conf.WebPort)
	log.Println("Webserver running")
	err := http.ListenAndServe(address, nil)
	check(err)
}
