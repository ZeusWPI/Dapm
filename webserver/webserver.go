package main

import (
	"encoding/json"
	"log"
	"net/http"
)

type POSTContent struct {
	Lights     []int
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

	return content, true
}

func ledHandler(w http.ResponseWriter, r *http.Request) {
	content, isOk := errorValidation(w, r)

	if !isOk {
		return
	}

	go led(&content)

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Led changed\n"))
}

func groupHandler(w http.ResponseWriter, r *http.Request) {
	content, isOk := errorValidation(w, r)

	if !isOk {
		return
	}

	go group(&content)

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Group changed\n"))
}

func blinkHandler(w http.ResponseWriter, r *http.Request) {
	isPost := filterPOST(w, r)
	if !isPost {
		return
	}

	go blink()

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("On Off On Off ...\n"))
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
