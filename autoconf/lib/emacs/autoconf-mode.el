;;; autoconf-mode.el --- autoconf code editing commands for Emacs

;; Author: Martin Buchholz (martin@xemacs.org)
;; Maintainer: Martin Buchholz
;; Keywords: languages, faces, m4, configure

;; This file is part of Autoconf

;; Copyright (C) 2001, 2006 Free Software Foundation, Inc.
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software
;; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
;; 02110-1301, USA.

;; A major mode for editing autoconf input (like configure.in).
;; Derived from m4-mode.el by Andrew Csillag (drew@staff.prodigy.com)

;;; Your should add the following to your Emacs configuration file:

;;  (autoload 'autoconf-mode "autoconf-mode"
;;            "Major mode for editing autoconf files." t)
;;  (setq auto-mode-alist
;;        (cons '("\\.ac\\'\\|configure\\.in\\'" . autoconf-mode)
;;              auto-mode-alist))

;;; Code:

;;thank god for make-regexp.el!
(defvar autoconf-font-lock-keywords
  `(("\\bdnl \\(.*\\)"  1 font-lock-comment-face t)
    ("\\$[0-9*#@]" . font-lock-variable-name-face)
    ("\\b\\(m4_\\)?\\(builtin\\|change\\(com\\|quote\\|word\\)\\|d\\(e\\(bug\\(file\\|mode\\)\\|cr\\|f\\(ine\\|n\\)\\)\\|iv\\(ert\\|num\\)\\|nl\\|umpdef\\)\\|e\\(rrprint\\|syscmd\\|val\\)\\|f\\(ile\\|ormat\\)\\|gnu\\|i\\(f\\(def\\|else\\)\\|n\\(c\\(lude\\|r\\)\\|d\\(ex\\|ir\\)\\)\\)\\|l\\(en\\|ine\\)\\|m\\(4\\(exit\\|wrap\\)\\|aketemp\\|kstemp\\)\\|p\\(atsubst\\|opdef\\|ushdef\\)\\|regexp\\|s\\(hift\\|include\\|ubstr\\|ys\\(cmd\\|val\\)\\)\\|tra\\(ceo\\(ff\\|n\\)\\|nslit\\)\\|un\\(d\\(efine\\|ivert\\)\\|ix\\)\\)\\b" . font-lock-keyword-face)
    ("^\\(\\(m4_\\)?define\\|A._DEFUN\\|m4_defun\\)(\\[?\\([A-Za-z0-9_]+\\)" 3 font-lock-function-name-face)
    "default font-lock-keywords")
)

(defvar autoconf-mode-syntax-table nil
  "syntax table used in autoconf mode")
(setq autoconf-mode-syntax-table (make-syntax-table))
(modify-syntax-entry ?\" "\""  autoconf-mode-syntax-table)
;;(modify-syntax-entry ?\' "\""  autoconf-mode-syntax-table)
(modify-syntax-entry ?#  "<\n" autoconf-mode-syntax-table)
(modify-syntax-entry ?\n ">#"  autoconf-mode-syntax-table)
(modify-syntax-entry ?\( "()"   autoconf-mode-syntax-table)
(modify-syntax-entry ?\) ")("   autoconf-mode-syntax-table)
(modify-syntax-entry ?\[ "(]"  autoconf-mode-syntax-table)
(modify-syntax-entry ?\] ")["  autoconf-mode-syntax-table)
(modify-syntax-entry ?*  "."   autoconf-mode-syntax-table)
(modify-syntax-entry ?_  "_"   autoconf-mode-syntax-table)

(defvar autoconf-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map '[(control c) (\;)] 'comment-region)
    map))

(defun autoconf-current-defun ()
  "Autoconf value for `add-log-current-defun-function'.
This tells add-log.el how to find the current macro."
  (save-excursion
    (if (re-search-backward "^\\(m4_define\\|m4_defun\\|A._DEFUN\\)(\\[*\\([A-Za-z0-9_]+\\)" nil t)
	(buffer-substring (match-beginning 2)
			  (match-end 2))
      nil)))

;;;###autoload
(defun autoconf-mode ()
  "A major-mode to edit Autoconf files like configure.ac.
\\{autoconf-mode-map}
"
  (interactive)
  (kill-all-local-variables)
  (use-local-map autoconf-mode-map)

  (make-local-variable 'add-log-current-defun-function)
  (setq add-log-current-defun-function 'autoconf-current-defun)

  (make-local-variable 'comment-start)
  (setq comment-start "# ")
  (make-local-variable 'parse-sexp-ignore-comments)
  (setq parse-sexp-ignore-comments t)

  (make-local-variable	'font-lock-defaults)
  (setq major-mode 'autoconf-mode)
  (setq mode-name "Autoconf")
  (setq font-lock-defaults `(autoconf-font-lock-keywords nil))
  (set-syntax-table autoconf-mode-syntax-table)
  (run-hooks 'autoconf-mode-hook))

(provide 'autoconf-mode)

;;; autoconf-mode.el ends here
