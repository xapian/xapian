package main

import ("fmt"
	"io"
	"encoding/csv"
	"os"
	"xapian"
)

func main(){
	csvfile,err := os.Open("../data/100-objects-v2.csv") //a csv reader to get the valuse from csv file
	if err != nil {
		os.Exit(1)
	}
	db := xapian.NewWritableDatabase("db",xapian.DB_CREATE_OR_OPEN)  //Open or Create the database we are goint to write in
	reader := csv.NewReader(csvfile)
	fields,_ := reader.Read()
	fmt.Println("Fields are ")
	fmt.Println(fields)
	fmt.Println()
	termgenerator := xapian.NewTermGenerator();//set up the termgenerator
	termgenerator.Set_stemmer(xapian.NewStem("en"))//set up the stem object..with lang
	fmt.Println(termgenerator)
	for{
		fields,err := reader.Read()
		if err == io.EOF {
			break
		}
		fmt.Println("id=",fields[0],"title=",fields[2],"desc=",fields[8])
		id_no := fields[0]
		title := fields[2]
		var x uint = 1
		description := fields[8]
		doc := xapian.NewDocument()
		termgenerator.Set_document(doc)
		termgenerator.Index_text(title,x,"S")
		fmt.Println("here")
		termgenerator.Index_text(description,x,"XD")

		termgenerator.Index_text(title)
		termgenerator.Increase_termpos()
		termgenerator.Index_text(description)

		doc.Set_data(id_no + "\n" + title + "\n" + description);
		fmt.Println(doc.Get_data())
		idterm := "Q" + id_no;
		doc.Add_boolean_term(idterm)
		db.Replace_document(idterm,doc)
		xapian.DeleteDocument(doc)
	}
	fmt.Println(db.Get_doccount())
	db.Close()
}

