package output

import (
	"reflect"
	"strings"
)

// Output represents an output option
type Output interface {
	Info(string)
	Error(string)
	Fatal(string)
}

var outputs = make(map[string]Output)

func registerOutput(output Output) {
	parts := strings.Split(reflect.TypeOf(output).String(), ".")
	outputs[strings.ToLower(parts[len(parts)-1])] = output
}

// ForName returns the output for a given name
func ForName(name string) Output {
	if output, ok := outputs[name]; ok {
		return output
	}
	// We always want an output, so default to text
	return outputs["text"]
}
