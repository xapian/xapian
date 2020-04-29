package main

import ("xapian")
import ("testing")

func TestMajorVersion(t *testing.T){
	actual := xapian.Major_version()
	expected := 1
	if actual!=expected {
		t.Error("fail")
	}
}

func TestDocument(t *testing.T){
	doc := xapian.NewDocument()
        input_str := "hello go xapian"
        doc.Set_data(input_str)
        output_str := doc.Get_data()

        if input_str != output_str{
		t.Error("Build Failed")
	}
}

