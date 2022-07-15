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
	coreFlag := args[1]
	buildFlag := args[2]
	cxxflags := getFlags(args[3])
	cppflags := getFlags(args[4])
	lcflags := getFlags(args[5])
	xapianCXXFlags := getFlags(args[6])
	xapianLibs := getFlags(args[7])
	cxxflags = strings.ReplaceAll(cxxflags, "-fstack-protector", "")

	//Creates required dirs in GOPATH for building xapian
	goXapianBuildDir := CreateDirsForXapian()
	fmt.Println("Directory for building xapian go bindings : ", goXapianBuildDir)

	var IC_FLAGS, LD_FLAGS string

	//Build accordingly if core is preinstalled or not.
	if coreFlag == "without-core" {
		if buildFlag == "build" {
			IC_FLAGS, LD_FLAGS = buildWithOutCore()
		}
		if buildFlag == "install" {
			IC_FLAGS, LD_FLAGS = installWithOutCore()
		}
	}

	if coreFlag == "with-core" {
		if buildFlag == "build" {
			IC_FLAGS, LD_FLAGS = xapianCXXFlags, xapianLibs
		}
		if buildFlag == "install" {
			IC_FLAGS, LD_FLAGS = xapianCXXFlags, xapianLibs
		}
	}
	LD_FLAGS = LD_FLAGS + " " + lcflags
	cxxflags = "#cgo CXXFLAGS: " + cxxflags
	cppflags = "#cgo CPPFLAGS: " + cppflags

	goBindings, _ := filepath.Abs("../")
	copyAndInsert(goBindings, goXapianBuildDir, LD_FLAGS, IC_FLAGS, cxxflags, cppflags)
	if buildFlag == "build" {
		build(goXapianBuildDir)
	}
	if buildFlag == "install" {
		install(goXapianBuildDir)
	}
}

//Function to create directory to build Xapian bindings for go.
func CreateDirsForXapian() string {
	gopathBytes, err := exec.Command("go", "env", "GOPATH").Output()
	if err != nil {
		fmt.Println(err)
		return "err"
	}
	gopath := string(gopathBytes[:len(gopathBytes)-1])
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

	goXapianBuildDir := filepath.Join(gopath, "src", "xapian.org", "xapian")
	if _, err := os.Stat(goXapianBuildDir); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", goXapianBuildDir)
		os.MkdirAll(goXapianBuildDir, 0777)
	}

	rawPath := filepath.Join(gopath, "src", "xapian.org", "xapian")
	if _, err := os.Stat(rawPath); os.IsNotExist(err) {
		fmt.Println("Creating dir - ", rawPath)
		os.Mkdir(rawPath, 0777)
	}

	return goXapianBuildDir
}

func install(goXapianBuildDir string) {
	cmd := exec.Command("go", "install", "-x", goXapianBuildDir)
	cmd.Dir = goXapianBuildDir
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

func build(goXapianBuildDir string) {
	cmd := exec.Command("go", "build", "-x", goXapianBuildDir)
	cmd.Dir = goXapianBuildDir
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
	xapianBindings := filepath.Join("../../")
	xapianBindings, _ = filepath.Abs(xapianBindings)
	xapianCore := filepath.Join(xapianBindings, "/../xapian-core/")
	pkgConfigPath := filepath.Join(xapianCore, "pkgconfig")
	filesPkgConfig, err := ioutil.ReadDir(pkgConfigPath)

	if err != nil {
		fmt.Println(err, "Error reading files in pkgconfig Directory")
		os.Exit(2)
	}

	if err != nil {
		fmt.Println(err, "Error reading files in pkgconfig Directory")
		os.Exit(2)
	}

	xapianPkgConfigPath := ""
	for _, f := range filesPkgConfig {
		if strings.HasSuffix(f.Name(), ".pc") {
			xapianPkgConfigPath = filepath.Join(pkgConfigPath, f.Name())
			break
		}
	}
	fmt.Println("xapian_pkgconfig_path - ", xapianPkgConfigPath)

	var IC_FLAGS string
	include := exec.Command("pkg-config", xapianPkgConfigPath, "--cflags")
	if output, err := include.Output(); err != nil {
		log.Fatalf("%s failed with %s", include, err)
	} else {
		IC_FLAGS = string(output[:len(output)-1])
		fmt.Println(string(IC_FLAGS))
	}

	var LD_FLAGS string
	ld_flags := exec.Command("pkg-config", xapianPkgConfigPath, "--libs")
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
	xapianBindings := filepath.Join("../../")
	xapianBindings, _ = filepath.Abs(xapianBindings)
	xapianCore := filepath.Join(xapianBindings, "/../xapian-core/")
	xapianHeaders := filepath.Join(xapianCore, "include/")
	pkgConfigPath := filepath.Join(xapianCore, "pkgconfig")

	fmt.Println("xapian_bindings - ", xapianBindings)
	fmt.Println("xapian-core - ", xapianCore)
	fmt.Println("xapian-headers - ", xapianHeaders)
	fmt.Println("pkgconfig - ", pkgConfigPath)

	filesPkgConfig, err := ioutil.ReadDir(pkgConfigPath)

	if err != nil {
		fmt.Println(err, "Error reading files in pkgconfig Directory")
		os.Exit(2)
	}

	xapianPkgConfigPath := ""
	for _, f := range filesPkgConfig {
		if strings.HasSuffix(f.Name(), ".pc") {
			xapianPkgConfigPath = filepath.Join(pkgConfigPath, f.Name())
			break
		}
	}

	fmt.Println("xapian_pkgconfig_path - ", xapianPkgConfigPath)

	config, err := os.Open(filepath.Join(xapianBindings, "config.h"))
	if err != nil {
		fmt.Println("error opening config.h ")
		os.Exit(2)
	}
	var ltObjDir string
	cfReader := bufio.NewScanner(config)
	for cfReader.Scan() {
		line := cfReader.Text()
		if strings.HasPrefix(line, "#define LT_OBJDIR") {
			strs := strings.Split(line, "\"")
			ltObjDir = strs[1]
			break
		}
	}

	fmt.Println("lt_obj_dir : ", ltObjDir)

	pkgConfigFile, err := os.Open(xapianPkgConfigPath)
	if err != nil {
		fmt.Println(err)
		os.Exit(2)
	}
	var libName string
	var libDeps string
	pkgReader := bufio.NewScanner(pkgConfigFile)
	for pkgReader.Scan() {
		line := pkgReader.Text()
		line = strings.TrimSpace(line)
		strs := strings.Split(line, " ")
		if len(strs) > 0 && strs[0] == "Libs:" {
			for _, v := range strs {
				if strings.Contains(v, "lxapian") {
					libName = v
					break
				}
			}
		}
		if len(strs) >= 1 && strs[0] == "Libs.private:" {
			libDeps = strings.Join(strs[1:], " ")
		}
	}
	fmt.Println("lib_name : ", libName)
	fmt.Println("lib_deps : ", libDeps)

	libDir := filepath.Join(xapianCore, ltObjDir)
	fmt.Println("lib_dir - ", libDir)

	library := strings.TrimSpace(libName + " " + libDeps)

	LDFLAGS := "#cgo LDFLAGS: " + "-L" + libDir + " -Wl,-rpath," + libDir + " " + library + " "
	ICFLAGS := "#cgo CXXFLAGS: " + "-I" + xapianHeaders

	LDFLAGS = strings.TrimSpace(LDFLAGS)
	ICFLAGS = strings.ReplaceAll(ICFLAGS, "\\", "/")
	LDFLAGS = strings.ReplaceAll(LDFLAGS, "\\", "/")

	return ICFLAGS, LDFLAGS
}

func copyAndInsert(goBindings, goXapianBuildDir, LD_FLAGS, IC_FLAGS, cxxflags, cppflags string) {
	xapianBuildGo := filepath.Join(goBindings, "xapian.go")
	InsertStringToFile(xapianBuildGo, LD_FLAGS+"\n", 18)
	InsertStringToFile(xapianBuildGo, IC_FLAGS+"\n", 19)
	InsertStringToFile(xapianBuildGo, cxxflags+"\n", 20)
	InsertStringToFile(xapianBuildGo, cppflags+"\n", 21)

	copyFileContents(filepath.Join(goBindings, "go.mod"), filepath.Join(goXapianBuildDir, "go.mod"))
	copyFileContents(filepath.Join(goBindings, "xapian.go"), filepath.Join(goXapianBuildDir, "xapian.go"))
	copyFileContents(filepath.Join(goBindings, "go_wrap.h"), filepath.Join(goXapianBuildDir, "go_wrap.h"))
	copyFileContents(filepath.Join(goBindings, "go_wrap.cxx"), filepath.Join(goXapianBuildDir, "go_wrap.cxx"))
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
		log.Fatalf("failed to open file: %s", err)
	}
	defer original.Close()

	// Create new file
	new, err := os.Create(dst)
	if err != nil {
		log.Fatalf("failed to create or truncate file: %s", err)
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
