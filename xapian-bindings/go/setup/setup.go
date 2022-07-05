/*
This is cross-platform setup file for building and installing xapian-bindings.
All the required flags are passed to this program as command line arguments.
*/
package main

import (
	"bufio"
	"bytes"
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
	//Checking if required arguments were passed
	args := os.Args
	if len(args) != 8 {
		fmt.Println("getting CGO Flags failed")
		os.Exit(2)
	}

	// Get all the flags
	core_flag := args[1]
	build_flag := args[2]
	cxxflags := getFlags(args[3])
	cppflags := getFlags(args[4])
	lcflags := getFlags(args[5])
	xapian_cxxflags := getFlags(args[6])
	xapian_libs := getFlags(args[7])
	cxxflags = strings.ReplaceAll(cxxflags, "-fstack-protector", "")

	//Creates required dirs in GOPATH for building xapian
	go_xapian_build_dir := CreateDirsForXapian()
	fmt.Println("Directory for building xapian go bindings : ", go_xapian_build_dir)

	var IC_FLAGS, LD_FLAGS string

	//Build accordingly if core is preinstalled or not.
	if core_flag == "without-core" {
		if build_flag == "build" {
			IC_FLAGS, LD_FLAGS = buildWithOutCore()
		}
		if build_flag == "install" {
			IC_FLAGS, LD_FLAGS = installWithOutCore()
		}
	}

	if core_flag == "with-core" {
		if build_flag == "build" {
			IC_FLAGS, LD_FLAGS = xapian_cxxflags, xapian_libs
		}
		if build_flag == "install" {
			IC_FLAGS, LD_FLAGS = xapian_cxxflags, xapian_libs
		}
	}
	LD_FLAGS = LD_FLAGS + " " + lcflags
	cxxflags = "#cgo CXXFLAGS: " + cxxflags
	cppflags = "#cgo CPPFLAGS: " + cppflags

	go_bindings, _ := filepath.Abs("../")
	fmt.Println(go_bindings)
	copyAndInsert(go_bindings, go_xapian_build_dir, LD_FLAGS, IC_FLAGS, cxxflags, cppflags)
	if build_flag == "build" {
		build(go_xapian_build_dir)
	}
	if build_flag == "install" {
		install(go_xapian_build_dir)
	}
}

//Function to create directory to build Xapian bindings for go.
func CreateDirsForXapian() string {
	fmt.Print()
	gopath_bytes, err := exec.Command("go", "env", "GOPATH").Output()
	if err != nil {
		fmt.Println(err)
		return "err"
	}
	gopath := string(gopath_bytes[:len(gopath_bytes)-1])
	fmt.Println("GOPATH -  ", gopath)

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

	go_xapian_build_dir := filepath.Join(gopath, "src/xapian.org/xapian/raw")
	if _, err := os.Stat(go_xapian_build_dir); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", go_xapian_build_dir)
		os.MkdirAll(go_xapian_build_dir, 0777)
	}

	return go_xapian_build_dir
}

func install(go_xapian_build_dir string) {
	cmd := exec.Command("go", "install", "-x", go_xapian_build_dir)
	cmd.Dir = go_xapian_build_dir
	var stdoutBuf, stderrBuf bytes.Buffer
	cmd.Stdout = io.MultiWriter(os.Stdout, &stdoutBuf)
	cmd.Stderr = io.MultiWriter(os.Stderr, &stderrBuf)

	err := cmd.Run()
	if err != nil {
		log.Fatalf("cmd.Run() failed with %s\n", err)
	}
	outStr, errStr := stdoutBuf.String(), stderrBuf.String()
	fmt.Println(outStr, errStr)
	fmt.Println("Go bindings installation successful!")
}
func build(go_xapian_build_dir string) {
	cmd := exec.Command("go", "build", "-x", go_xapian_build_dir)
	cmd.Dir = go_xapian_build_dir
	var stdoutBuf, stderrBuf bytes.Buffer
	cmd.Stdout = io.MultiWriter(os.Stdout, &stdoutBuf)
	cmd.Stderr = io.MultiWriter(os.Stderr, &stderrBuf)

	err := cmd.Run()
	if err != nil {
		log.Fatalf("cmd.Run() failed with %s\n", err)
	}
	outStr, errStr := stdoutBuf.String(), stderrBuf.String()
	fmt.Println(outStr, errStr)
	fmt.Println("Go bindings built successful!")
}
func installWithOutCore() (string, string) {
	xapian_bindings := filepath.Join("../../")
	xapian_bindings, _ = filepath.Abs(xapian_bindings)
	xapian_core := filepath.Join(xapian_bindings, "/../xapian-core/")
	pkgconfig_path := filepath.Join(xapian_core, "pkgconfig")
	files_pkgconfig, err := ioutil.ReadDir(pkgconfig_path)

	if err != nil {
		fmt.Println(err, "Error reading files in pkgconfig Directory")
		os.Exit(2)
	}

	xapian_pkgconfig := ""
	for _, f := range files_pkgconfig {
		if strings.HasSuffix(f.Name(), ".pc") {
			xapian_pkgconfig = strings.TrimSuffix(f.Name(), ".pc")
			break
		}
	}
	fmt.Println("xapian_pkgconfig_path - ", xapian_pkgconfig)

	var IC_FLAGS string
	include := exec.Command("pkg-config", xapian_pkgconfig, "--cflags")
	if output, err := include.Output(); err != nil {
		log.Fatalf("%s failed with %s", include, err)
	} else {
		IC_FLAGS = string(output[:len(output)-1])
		fmt.Println(string(IC_FLAGS))
	}

	var LD_FLAGS string
	ld_flags := exec.Command("pkg-config", xapian_pkgconfig, "--libs")
	if output, err := ld_flags.Output(); err != nil {
		log.Fatalf("%s failed with %s", ld_flags, err)
	} else {
		LD_FLAGS = string(output[:len(output)-1])
		fmt.Println(LD_FLAGS)
	}

	LD_FLAGS = "#cgo LDFLAGS: " + LD_FLAGS
	IC_FLAGS = "#cgo CXXFLAGS: " + IC_FLAGS
	return IC_FLAGS, LD_FLAGS
}

func buildWithOutCore() (string, string) {
	xapian_bindings := filepath.Join("../../")
	xapian_bindings, _ = filepath.Abs(xapian_bindings)
	xapian_core := filepath.Join(xapian_bindings, "/../xapian-core/")
	xapian_headers := filepath.Join(xapian_core, "include/")
	pkgconfig_path := filepath.Join(xapian_core, "pkgconfig")

	fmt.Println("xapian_bindings - ", xapian_bindings)
	fmt.Println("xapian-core - ", xapian_core)
	fmt.Println("xapian-headers - ", xapian_headers)
	fmt.Println("pkgconfig - ", pkgconfig_path)

	files_pkgconfig, err := ioutil.ReadDir(pkgconfig_path)

	if err != nil {
		fmt.Println(err, "Error reading files in pkgconfig Directory")
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

	lib_dir := filepath.Join(xapian_core, lt_obj_dir)
	fmt.Println("lib_dir - ", lib_dir)

	library := strings.TrimSpace(lib_name + " " + lib_deps)

	LDFLAGS := "#cgo LDFLAGS: " + "-L" + lib_dir + " -Wl,-rpath," + lib_dir + " " + library + " "
	ICFLAGS := "#cgo CXXFLAGS: " + "-I" + xapian_headers

	LDFLAGS = strings.TrimSpace(LDFLAGS)
	ICFLAGS = strings.ReplaceAll(ICFLAGS, "\\", "/")
	LDFLAGS = strings.ReplaceAll(LDFLAGS, "\\", "/")

	return ICFLAGS, LDFLAGS
}

func copyAndInsert(go_bindings, go_xapian_build_dir, LD_FLAGS, IC_FLAGS, cxxflags, cppflags string) {
	rawPath := filepath.Join(go_bindings, "raw")
	if _, err := os.Stat(rawPath); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", rawPath)
		os.Mkdir(rawPath, 0777)
	}

	copyFileContents(filepath.Join(go_bindings, "go.mod"), filepath.Join(go_xapian_build_dir, "go.mod"))
	copyFileContents(filepath.Join(rawPath, "xapian.go"), filepath.Join(go_xapian_build_dir, "raw", "xapian.go"))
	copyFileContents(filepath.Join(rawPath, "go_wrap.h"), filepath.Join(go_xapian_build_dir, "raw", "go_wrap.h"))
	copyFileContents(filepath.Join(rawPath, "go_wrap.cxx"), filepath.Join(go_xapian_build_dir, "raw", "go_wrap.cxx"))

	xapian_build_go := filepath.Join(go_xapian_build_dir, "raw", "xapian.go")
	InsertStringToFile(xapian_build_go, LD_FLAGS+"\n", 18)
	InsertStringToFile(xapian_build_go, IC_FLAGS+"\n", 19)
	InsertStringToFile(xapian_build_go, cxxflags+"\n", 20)
	InsertStringToFile(xapian_build_go, cppflags+"\n", 21)
}

func getFlags(arg string) string {
	result := ""
	temp := strings.Split(arg, "-->")
	if len(temp) > 1 {
		result = strings.Join(temp[1:], " ")
	}
	return result
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
