package main

import (
	"log"
	"strings"
)

func check(err error) {
	if err != nil {
		log.Fatalf("Error: %v", err)
	}
}

func checkNonFatal(err error) bool {
	if err != nil {
		log.Printf("Error: %v\n", err)
        return true
	}
    return false
}

func concatStrings(args ...string) string {
	builder := new(strings.Builder)

	for i := 0; i < len(args); i++ {
		_, err := builder.WriteString(args[i])
		check(err)
	}

	return builder.String()
}
