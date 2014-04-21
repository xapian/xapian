#!/bin/sh

sphinx-apidoc -F -o sphinx-doc xapian -H "Xapian Python Bindings" -A "Xapian Team" -V "1.3.3" -R "1.3.3"
make -C sphinx-doc html
# bxdg-open sphinx-doc/_build/html/index.html 
