package main

import (
	"bufio"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

func main() {
	fmt.Print("GOPATH -  ")
	gopath_bytes, err := exec.Command("go", "env", "GOPATH").Output()
	if err != nil {
		fmt.Println(err)
		return
	}
	gopath := string(gopath_bytes[:len(gopath_bytes)-1])
	fmt.Println(gopath)

	if _, err := os.Stat(gopath); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", gopath)
		os.Mkdir(gopath, 0777)
	}

	if _, err := os.Stat(filepath.Join(gopath, "src")); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", filepath.Join(gopath, "src"))
		os.Mkdir(filepath.Join(gopath, "src"), 0777)
	}

	if _, err := os.Stat(filepath.Join(gopath, "bin")); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", filepath.Join(gopath, "bin"))
		os.Mkdir(filepath.Join(gopath, "bin"), 0777)
	}

	if _, err := os.Stat(filepath.Join(gopath, "pkg")); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", filepath.Join(gopath, "pkg"))
		os.Mkdir(filepath.Join(gopath, "pkg"), 0777)
	}

	go_xapian_build_dir := filepath.Join(gopath, "src/xapian")
	if _, err := os.Stat(go_xapian_build_dir); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", go_xapian_build_dir)
		os.Mkdir(go_xapian_build_dir, 0777)
	}

	fmt.Println("go build dir for xapian - ", go_xapian_build_dir)

	args := os.Args
	if len(args) < 1 {
		fmt.Println("getting CGO Flags failed ")
		os.Exit(2)
	}

	if args[1] == "install-bindings" {
		fmt.Println("installing bindinds ...", go_xapian_build_dir)
		_, err := exec.Command("go", "install", "-x", go_xapian_build_dir).Output()
		if err != nil {
			fmt.Println("Installation Failed", err)
			os.Exit(2)
		}
		fmt.Println("Installation of Go bindings Successfull!")
		return
	}

	var xapian_bindings, xapian_cxxflags, xapian_cppflags, lc string
	temp := strings.Split(args[1], "-->")
	if len(temp) > 1 {
		xapian_bindings = strings.Join(temp[1:], " ")
	}
	temp = strings.Split(args[2], "-->")
	if len(temp) > 1 {
		xapian_cxxflags = strings.Join(temp[1:], " ")
		xapian_cxxflags = strings.ReplaceAll(xapian_cxxflags, "-fstack-protector", "")
	}
	temp = strings.Split(args[3], "-->")
	if len(temp) > 1 {
		xapian_cppflags = strings.Join(temp[1:], " ")
	}
	temp = strings.Split(args[4], "-->")
	if len(temp) > 1 {
		lc = strings.Join(temp[1:], " ")
	}

	xapian_bindings = filepath.Join("../../")
	xapian_bindings, _ = filepath.Abs(xapian_bindings)
	fmt.Println("xapian_bindings - ", xapian_bindings)
	xapian_core := filepath.Join(xapian_bindings, "/../xapian-core/")
	fmt.Println("xapian-core - ", xapian_core)
	xapian_headers := filepath.Join(xapian_core, "include/")
	fmt.Println("xapian-headers - ", xapian_headers)

	pkgconfig_path := filepath.Join(xapian_core, "pkgconfig")
	fmt.Println("pkgconfig - ", pkgconfig_path)

	files_pkgconfig, err := ioutil.ReadDir(pkgconfig_path)

	if err != nil {
		fmt.Println(err)
		os.Exit(2)
	}
	xapian_pkgconfig_path := ""
	for _, f := range files_pkgconfig {
		if strings.HasSuffix(f.Name(), ".pc") {
			xapian_pkgconfig_path = filepath.Join(pkgconfig_path, f.Name())
			break
		}
	}

	fmt.Println("xapian_pkgconfig_path - ", xapian_pkgconfig_path)

	config, err := os.Open(filepath.Join(xapian_bindings, "config.h"))
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

	fmt.Println("lt_obj_dir : ", lt_obj_dir)

	pkgconfig_file, err := os.Open(xapian_pkgconfig_path)
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
	fmt.Println("lib_name : ", lib_name)
	fmt.Println("lib_deps : ", lib_deps)
	// var lc string
	// if len(args) == 3{
	// 	lc = args[2]
	// }
	lib_dir := filepath.Join(xapian_core, lt_obj_dir)
	fmt.Println("lib_dir - ", lib_dir)

	// return

	library := strings.TrimSpace(lib_name + lib_deps)

	LDFLAGS := "#cgo LDFLAGS: " + "-L" + lib_dir + " -Wl,-rpath," + lib_dir + " " + library + " " + lc
	ICFLAGS := "#cgo CXXFLAGS: " + "-I" + xapian_headers
	CXXFLAGS := "#cgo CXXFLAGS: " + xapian_cxxflags
	CPPFLAGS := "#cgo CPPFLAGS: " + xapian_cppflags

	LDFLAGS = strings.TrimSpace(LDFLAGS)
	go_bindings := filepath.Join(xapian_bindings, "go")
	xapian_go := filepath.Join(xapian_bindings, "go", "xapian.go")
	ICFLAGS = strings.ReplaceAll(ICFLAGS, "\\", "/")
	LDFLAGS = strings.ReplaceAll(LDFLAGS, "\\", "/")

	fmt.Println(LDFLAGS)
	fmt.Println(ICFLAGS)
	fmt.Println(CXXFLAGS)
	fmt.Println(CPPFLAGS)

	InsertStringToFile(xapian_go, LDFLAGS+"\n", 18)
	InsertStringToFile(xapian_go, ICFLAGS+"\n", 19)
	InsertStringToFile(xapian_go, CXXFLAGS+"\n", 20)
	InsertStringToFile(xapian_go, CPPFLAGS+"\n", 21)

	copyFileContents(xapian_go, filepath.Join(go_xapian_build_dir, "xapian.go"))
	copyFileContents(filepath.Join(go_bindings, "go_wrap.h"), filepath.Join(go_xapian_build_dir, "go_wrap.h"))
	copyFileContents(filepath.Join(go_bindings, "go_wrap.cxx"), filepath.Join(go_xapian_build_dir, "go_wrap.cxx"))
	_, err = exec.Command("go", "build", "-x", go_xapian_build_dir).Output()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("Build Successfull!")
}

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

func copyFileContents(src, dst string) (err error) {
	original, err := os.Open(src)
	if err != nil {
		log.Fatal(err)
	}
	defer original.Close()

	// Create new file
	new, err := os.Create(dst)
	if err != nil {
		log.Fatal(err)
	}
	defer new.Close()

	//This will copy
	_, err = io.Copy(new, original)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("copied ", src)
	return err
}
