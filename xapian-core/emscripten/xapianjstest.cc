/** @file
 * @brief Emscripten main program test
 */
#include <emscripten.h>
#include <xapian.h>
#include <iostream>

using namespace std;

int main() {
#ifdef NODEFS
    EM_ASM({
	    FS.mkdir("/work");
	    FS.mount(NODEFS, {root: '.'},"/work");
	    FS.chdir("/work");
    });
#endif

    Xapian::WritableDatabase db("testdb", Xapian::DB_CREATE_OR_OPEN);

    Xapian::TermGenerator termgenerator;
    Xapian::Document doc;
    termgenerator.set_document(doc);
    termgenerator.index_text("This is a test document.");
    db.replace_document("Q1", doc);

    cout << "Created database with " << db.get_doccount()
	 << " documents" << endl;
}
