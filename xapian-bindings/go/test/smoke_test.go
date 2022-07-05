package main

import (
	"fmt"
	"testing"

	"xapian.org/xapian"
)

func TestMajorVersion(t *testing.T) {
	actual := xapian.Major_version()
	fmt.Print("Running test: MajorVersion... ")
	expected := 1
	if actual != expected {
		t.Error("fail")
	} else {
		fmt.Println("ok")
	}
}

func TestDocument(t *testing.T) {
	doc := xapian.NewDocument()
	input_str := "hello go xapian"
	fmt.Print("Running test: Document... ")
	doc.Set_data(input_str)
	output_str := doc.Get_data()

	if input_str != output_str {
		t.Error("Build Failed")
	} else {
		fmt.Println("ok")
	}

	tg := xapian.NewTermGenerator()
	tg.Set_document(doc)
	tg.Index_text(input_str)
	if doc.Termlist_count() != 3 {
		t.Errorf("Wrong number of terms: %d", doc.Termlist_count())
	}
}

func TestOpenDatabase(t *testing.T) {
	_, err := xapian.NewDatabase("non-existing-db")
	if err != nil {
		fmt.Println("ok")
	}
	fmt.Println("ok")
}
