package xapian

import (
	"errors"
	"strings"

	raw "xapian.org/xapian/raw"
)

var DatabaseNotFoundError = errors.New("DatabaseNotFoundError")

var errorMap = map[string]error{
	"DatabaseNotFoundError": DatabaseNotFoundError,
}

func Major_version() int {
	return raw.Major_version()
}

func Minor_version() int {
	return raw.Minor_version()
}

func Version_string() string {
	return raw.Version_string()
}

func NewDocument() raw.Document {
	return raw.NewDocument()
}

func NewTermGenerator() raw.TermGenerator {
	return raw.NewTermGenerator()
}

func NewDatabase(args ...interface{}) (db raw.Database, err error) {
	defer func() {
		if r := recover(); r != nil {
			if errMsg, ok := r.(string); ok {
				for msgPrefix, errType := range errorMap {
					if strings.HasPrefix(errMsg, msgPrefix) {
						err = errType
						return
					}
				}
			}
			panic(r)
		}
	}()

	db = raw.NewDatabase(args...)
	return
}
