package main

import ("fmt"
	"xapian"
)
func main(){
	qs := "Ansoina"
	var offset uint = 0
	var pagesize uint =0
	db := xapian.NewDatabase("/home/test2/xapian/xapian-bindings/go/examples/db")
	fmt.Println(db,offset,pagesize)
	qp := xapian.NewQueryParser()
	fmt.Println(qp)
	qp.Set_stemmer(xapian.NewStem("en"))
	fmt.Println("here")
	var xp xapian.XapianQueryParserStem_strategy

	qp.Set_stemming_strategy(xp)
	qp.Add_prefix("title","S")
	qp.Add_prefix("description","XD")

	query := qp.Parse_query(qs)
	fmt.Println(query,qp)
	enquire := xapian.NewEnquire(db)
	enquire.Set_query(query)

	mset := enquire.Get_mset(offset,pagesize);
	fmt.Println(mset)
	mset.Sort_by_relevance()
	fmt.Printf("%T %v\n",mset,mset)
	//m := mset.Begin()
	//fmt.Println(m.Get_document())
	fmt.Println(mset.Get_description())
	fmt.Println(mset.Get_termfreq(qs))
	fmt.Println(db.Get_avlength())
	db.Close()
}
