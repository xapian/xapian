package main

import ("xapian")
import ("testing")
import ("fmt")
func TestMajorVersion(t *testing.T){
	actual := xapian.Major_version()
	fmt.Print("Running test: MajorVersion... ")
	expected := 1
	if actual!=expected {
		t.Error("fail")
	}else{
		fmt.Println("ok")
	}
}

func TestDocument(t *testing.T){
	doc := xapian.NewDocument()
        input_str := "hello go xapian"
        fmt.Print("Running test: Document... ")
	doc.Set_data(input_str)
        output_str := doc.Get_data()

        if input_str != output_str{
		t.Error("Build Failed")
	}else{
		fmt.Println("ok")
	}
}

