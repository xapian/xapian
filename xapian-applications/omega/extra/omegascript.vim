" Vim syntax file
" Language:	OmegaScript
" Maintainer:	Richard Boulton <Richard Boulton>
" URL:		http://www.xapian.org/
" Last Change:  2002 Aug 16

" To install: place this file in ~/.vim/syntax/omegascript.vim
" and then create or add the following lines, without the "'s commenting
" them out, to ~/.vim/filetype.vim
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

" All keywords which can take parameters
syn keyword omegaScriptKW contained add allterms and cgi cgilist collapse date def div env eq field filesize freq ge gt highlight hitlist hostname html htmlstrip if include le list lt map max min mod mul ne nice not opt or prettyterm range record relevant set setmap setrelevant slice sub topterms url

" All keywords which can take no parameters
syn keyword omegaScriptCommandKW contained add allterms collapse dbname defaultop error fmt freqs hitsperpage id last lastpage msize percentage query querydescription queryterms record relevant relevants score setrelevant terms thispage topdoc topterms version

" Deprecated keywords
syn keyword omegaScriptCommandKWDeprecated contained freqs

hi link omegaScriptCommand Statement
hi link omegaScriptBegin Statement
hi link omegaScriptItem PreProc
hi link omegaScriptLiteral Special
hi link omegaScriptKW Type
hi link omegaScriptArg Type
hi link omegaScriptCommandKW Type
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
