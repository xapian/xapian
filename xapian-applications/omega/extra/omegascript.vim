" Vim syntax file
" Language:	OmegaScript
" Maintainer:	Richard Boulton <Richard Boulton>
" URL:		http://www.xapian.org/
" Last Change:  2002 Aug 16

" To install: place this file in ~/.vim/syntax/omegascript.vim
" and then create or add the following lines, without the "'s commenting
" them out, to ~/,vim/filetype.vim
"if exists("did_load_filetypes")
"  finish
"endif
"augroup filetypedetect
"  au! BufRead,BufNewFile */templates/* setfiletype omegascript
"augroup END

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if !exists("main_syntax")
  if version < 600
    syntax clear
  elseif exists("b:current_syntax")
  finish
endif
  let main_syntax = 'omegascript'
endif

" Include HTML highlighting at the top level
runtime! syntax/html.vim
unlet b:current_syntax

" Html included in commands
" Just do very basic highlighting, because it is likely to be only a fragment.
" Can't sensibly insist on matching angle brackets, even.
syn match omegaScriptHTML contained +[<>="]+ 

" I _think_ omegascript is case sensitive
syn case match

" tag names
syn match omegaScriptError "[${},]"
syn match omegaScriptError2 "[${,]"
syn match omegaScriptLiteral "$[$().]"
syn match omegaScriptArg contained "\$[1-9_]"

syn match omegaScriptCommand "$[a-zA-Z][a-zA-Z]*" contains=omegaScriptCommandKW,omegaScriptCommandKWDeprecated
syn match omegaScriptBegin contained "$[a-zA-Z][a-zA-Z]*{"he=e-1 contains=omegaScriptKW
syn region omegaScriptItem start="$[a-zA-Z][a-zA-Z]*{" end="}" contains=omegaScriptItem,omegaScriptCommand,omegaScriptBegin,omegaScriptSep,omegaScriptLiteral,omegaScriptArg,omegaScriptError2,omegaScriptHtml,omegaScriptComment keepend extend
syn match omegaScriptSep contained ","
syn region omegaScriptCommentBracePair start="{"rs=s+2 end="}" contains=omegaScriptCommentBracePair keepend extend
syn match omegaScriptCommentStart contained "{"
syn region omegaScriptComment matchgroup=omegaScriptCommentStart start="${" end="}" contains=omegaScriptCommentBracePair,omegaScriptComment keepend extend

syn keyword omegaScriptKW contained allterms cgi cgilist date def
syn keyword omegaScriptKW contained env field filesize
syn keyword omegaScriptKW contained freq highlight hitlist
syn keyword omegaScriptKW contained hostname html htmlstrip
syn keyword omegaScriptKW contained list map nice
syn keyword omegaScriptKW contained opt prettyterm
syn keyword omegaScriptKW contained range record
syn keyword omegaScriptKW contained relevant set set_relevant
syn keyword omegaScriptKW contained setmap topterms url
syn keyword omegaScriptCommandKW contained dbname defaultop error fmt freqs
syn keyword omegaScriptCommandKW contained hitsperpage id last lastpage msize
syn keyword omegaScriptCommandKW contained percentage query querydescription
syn keyword omegaScriptCommandKW contained queryterms relevants score terms
syn keyword omegaScriptCommandKW contained thispage topdoc version
syn keyword omegaScriptCommandKWDeprecated contained freqs

hi link omegaScriptCommand Statement
hi link omegaScriptBegin Statement
hi link omegaScriptItem PreProc
hi link omegaScriptLiteral Special
hi link omegaScriptKW Statement
hi link omegaScriptArg Type
hi link omegaScriptCommandKW Statement
hi link omegaScriptCommandKWDeprecated Special
hi link omegaScriptSep Normal
hi link omegaScriptError Error
hi link omegaScriptError2 Error
hi link omegaScriptHtml Constant
hi link omegaScriptComment Comment
hi link omegaScriptCommentBracePair Comment
hi link omegaScriptCommentStart Comment

syn sync fromstart

let b:current_syntax = "omegascript"

if main_syntax == 'omegascript'
  unlet main_syntax
endif

" vim: ts=8
