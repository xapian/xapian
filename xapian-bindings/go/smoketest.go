package xapian

import (
  "fmt"
  "testing"
  "strings"
)

func TestVersion(t *testing.T) {
  if v := fmt.Sprintf("%d.%d.%d", Major_version(), Minor_version(), Revision()); Version_string() != v {
    t.Errorf("Unexpected version output")
  }
  if len(strings.Split(Version_string(),".")) != 3 {
    t.Errorf("Version_string() not X.Y.Z")
  }
  if strings.Split(Version_string(),".")[0] != "1" {
    t.Errorf("Version_string() not 1.Y.Z")
  } 
}

func TestGeneric1(t *testing.T) {
  stem := NewStem("english")
  if stem.Get_description() != "Xapian::Stem(english)" {
    t.Errorf("Unexpected str(stem)")
  }
  doc := NewDocument()
  doc.Set_data("a\x00b")
  if doc.Get_data() == "a" {
    t.Errorf("get_data+set_data truncates at a zero byte")
  }
  if doc.Get_data() != "a\x00b" {
    t.Errorf("get_data+set_data doesn't transparently handle a zero byte");
  }
  doc.Set_data("is there anybody out there?")
  doc.Add_term("XYzzy")
  doc.Add_posting(stem.Apply("is"), uint(1))
  doc.Add_posting(stem.Apply("there"), uint(2))
  doc.Add_posting(stem.Apply("anybody"), uint(3))
  doc.Add_posting(stem.Apply("out"), uint(4))
  doc.Add_posting(stem.Apply("there"), uint(5))

  db := Inmemory_open()
  db.Add_document(doc)
  if db.Get_doccount() != 1 {
    t.Errorf("Unexpected db.get_doccount()")
  }
  
  if NewQuery(XapianQueryOp(QueryOP_OR), NewQuery(XapianQueryOp(QueryOP_OR), "smoke", "test"), NewQuery("terms")).Get_description() != "Query(((smoke OR test) OR terms))" {
    t.Errorf("Expected query Query(((smoke OR test) OR terms))");
  }
/*
  TODO: refactor how queries are created and create tests for
  terms := []string{"smoke", "test", "terms"}
  expect_query(xapian.Query(xapian.Query.OP_OR, terms), "(smoke OR test OR terms)")
  query1 = xapian.Query(xapian.Query.OP_PHRASE, ("smoke", "test", "tuple"))
  query2 = xapian.Query(xapian.Query.OP_XOR, (xapian.Query("smoke"), query1, "string"))
  expect_query(query1, "(smoke PHRASE 3 test PHRASE 3 tuple)")
  expect_query(query2, "(smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string)")
  subqs = ["a", "b"]
  expect_query(xapian.Query(xapian.Query.OP_OR, subqs), "(a OR b)")
  expect_query(xapian.Query(xapian.Query.OP_VALUE_RANGE, 0, '1', '4'), "VALUE_RANGE 0 1 4")
*/
}
