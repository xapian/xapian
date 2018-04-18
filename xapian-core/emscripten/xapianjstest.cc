/** @file xapianjstest.cc
 * @brief Emscripten main program test
 */
#include <emscripten.h>
#include <xapian.h>
#include <cstdlib>
#include <iostream>

using namespace std;

int main() {
	Xapian::WritableDatabase db;

	EM_ASM_({
		FS.mkdir("/work");
		FS.mount(NODEFS, {root: '.'},"/work");
		FS.chdir("/work");
    });

    db = Xapian::WritableDatabase("testdb", Xapian::DB_CREATE_OR_OPEN);

	Xapian::TermGenerator termgenerator;
	Xapian::Document doc;
	termgenerator.set_document(doc);
	termgenerator.index_text("This is a test document.");
	db.replace_document("Q1", doc);

	cout << "Created database with " << db.get_doccount() <<
			" document" << endl;
}
