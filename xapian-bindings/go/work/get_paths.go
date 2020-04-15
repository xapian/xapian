package main

import ("fmt"
		"os"
		"bufio"
		"io"
		"io/ioutil"
		"strings"
)
func InsertStringToFile(path, str string, index int) error {
	lines, err := File2lines(path)
	if err != nil {
		return err
	}

	fileContent := ""
	for i, line := range lines {
		if i == index {
			fileContent += str
		}
		fileContent += line
		fileContent += "\n"
	}

	return ioutil.WriteFile(path, []byte(fileContent), 0644)
}
func LinesFromReader(r io.Reader) ([]string, error) {
	var lines []string
	scanner := bufio.NewScanner(r)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}

	return lines, nil
}
func File2lines(filePath string) ([]string, error) {
	f, err := os.Open(filePath)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	return LinesFromReader(f)
}
func main(){
	goconf, err := os.Open("goconf")
	if (err != nil){
		fmt.Printf("error opening goconf ",err)
		os.Exit(2);
	}
	
	fs :=  bufio.NewScanner(goconf)
	fs.Scan();
	la_path := fs.Text();
	fs.Scan();
	xapian_libs := fs.Text();
	fs.Scan();
	abs_top_builddir := fs.Text();
	abs_top_builddir = "#cgo LDFLAGS: " + abs_top_builddir + "/../xapian-core/"

	// fmt.Println(la_path)
	// fmt.Println(xapian_libs)
	goconf.Close();

	la , err := os.Open(la_path)
	if (err != nil){
		fmt.Println("error open lib tool archive ");
		os.Exit(2);
	}

	var dlname string
	var deps string

	{
		strs := strings.Split(xapian_libs," ")
		if len(strs) > 1{
			deps+=strings.Join(strs[1:]," ")
			fmt.Println(deps)
		}
	}

	la_reader := bufio.NewScanner(la)
	for la_reader.Scan(){
		line := la_reader.Text()
		if len(line) >0 && line[0] != '#' {
			strs := strings.Split(line,"'")
			if strs[0] == "dlname=" {
				// fmt.Println(strs[1])
				dlname = strs[1]
			}
			if (strs[0] == "dependency_libs="){
				deps += strings.Join(strs[1:]," ")
			}
		}
	}
	la.Close()

	config,err := os.Open("../../config.h")
	if err != nil{
		fmt.Println("error opening config.h ")
		os.Exit(2)
	}
	var lt_obj_dir string
	cf_reader := bufio.NewScanner(config)
	for cf_reader.Scan(){
		line := cf_reader.Text()
		if strings.HasPrefix(line,"#define LT_OBJDIR"){
			strs := strings.Split(line,"\"")
			lt_obj_dir = strs[1]
			break;
		}
	}

	fmt.Println("dlname = ",dlname)
	fmt.Println("dl deps = ",deps)
	fmt.Println("libs folder = ",lt_obj_dir)
	var sl []string
	sl = append(sl,lt_obj_dir+dlname,deps)
	final := strings.Join(sl," ")

	fmt.Println(abs_top_builddir+final)
	InsertStringToFile("../xapian.go",abs_top_builddir+final+"\n",18)
	// fmt.Println(sl)
	// final := lt_obj_dir + dlname + deps
	// final = strings.TrimSpace(final)
	// fmt.Println(final)

}