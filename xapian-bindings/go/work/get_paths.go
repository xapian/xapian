package main

import (
	"bufio"
	"fmt"
	"io"
	"io/ioutil"
	"os"
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

func main() {
	args := os.Args
	fmt.Println(args)
	if len(args) < 1{
		fmt.Println("getting CGO Flags failed ")
		os.Exit(2)
	}
	xapian_core := args[1]
	xapian_core = xapian_core + "/../xapian-core/"
	pkgconfig_path := xapian_core + "pkgconfig/"
	fmt.Println(pkgconfig_path)

	files, err := ioutil.ReadDir(pkgconfig_path)

	if err != nil {
		fmt.Println(err)
		os.Exit(2)
	}

	for _, f := range files {
		if strings.HasSuffix(f.Name(), ".pc") {
			pkgconfig_path += f.Name()
			break
		}
	}

	fmt.Println(pkgconfig_path)

	config, err := os.Open("../../config.h")
	if err != nil {
		fmt.Println("error opening config.h ")
		os.Exit(2)
	}
	var lt_obj_dir string
	cf_reader := bufio.NewScanner(config)
	for cf_reader.Scan() {
		line := cf_reader.Text()
		if strings.HasPrefix(line, "#define LT_OBJDIR") {
			strs := strings.Split(line, "\"")
			lt_obj_dir = strs[1]
			break
		}
	}

	fmt.Println(lt_obj_dir)

	pkgconfig_file, err := os.Open(pkgconfig_path)
	if err != nil {
		fmt.Println(err)
		os.Exit(2)
	}
	var lib_name string
	var lib_deps string
	pk_reader := bufio.NewScanner(pkgconfig_file)
	for pk_reader.Scan() {
		line := pk_reader.Text()
		line = strings.TrimSpace(line)
		strs := strings.Split(line, " ")
		if len(strs) > 0 && strs[0] == "Libs:" {
			for _, v := range strs {
				if strings.Contains(v, "lxapian") {
					lib_name = v
					break
				}
			}
		}
		if len(strs) >= 1 && strs[0] == "Libs.private:" {
			lib_deps = strings.Join(strs[1:], " ")
		}
	}
	var lc string
	if len(args) == 3{
		lc = args[2]
	}
	lib_dir := xapian_core + lt_obj_dir
	fmt.Println(lib_name, lib_deps)
	library := strings.TrimSpace(lib_name + lib_deps)
	final2 := "#cgo LDFLAGS: " + "-L" + lib_dir + " -Wl,-rpath," + lib_dir + " " + library +" " + lc
	fmt.Println(final2)
	InsertStringToFile("../xapian.go", final2+"\n", 18)

}


