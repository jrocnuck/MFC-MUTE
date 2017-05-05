; Modification History
;;
;; 2001-March-10   Jason Rohrer
;; Created.
;; Fixed so that c++ mode is used for .h files.
;; Fixed [C-backspace] mapping.
;;
;; 2001-May-21   Jason Rohrer
;; Changed so that Java inclass items are not doubly indented.
;;
;; 2002-January-21   Jason Rohrer
;; Gave in and set to use spaces instead of tabs.
;; This is primarily motivated by the fact that tabs look terrible
;; in CVS-web.
;;

(custom-set-variables
 '(tab-stop-list (quote (4 8 12 16 20 24 28 32 36 40 44 48 52 56 60))))
(custom-set-faces)


;(setq-default scroll-step 1)               ; turn off jumpy scroll
(setq-default visible-bell t)              ; no beeps, flash on errors

(display-time)                             ; display the time on modeline
(column-number-mode t)                     ; display the column number on modeline
(setq-default kill-whole-line t)           ; ctrl-k kills whole line if at col 0
(setq-default fill-column 80)              ; wrap at col 80
(setq-default tab-width 4)                 ; show tabs as 4 cols

(setq font-lock-maximum-decoration t)      ; use colours in font lock mode
(setq font-lock-maximum-size nil)          ; turn off limit on font lock mode

; turn on font-lock everywhere (if possible)
(global-font-lock-mode 1 t)

; make sure C-backspace kills a word
( global-set-key `[C-backspace]   `backward-kill-word )

; background color, light yellow
( add-to-list  'default-frame-alist '( background-color . "#FFFFDC" ) )

; auto fill mode
( add-hook 'c-mode-common-hook 'turn-on-auto-fill )

; fix comment syntax highlighting problems
( setq font-lock-support-mode 'lazy-lock-mode )
( setq lazy-lock-defer-time 0 )

; delete highlighted text as we type
( delete-selection-mode t )

; make sure c++ mode is used for .h files
( setq auto-mode-alist ( 
	append '(
		("\\.h$" . c++-mode)
		)
	auto-mode-alist
	)
)


; off for now, since it screws up indenting
; load the line numbers package
;( load "setnu" );
; turn it on
;( add-hook 'c-mode-common-hook 'turn-on-setnu-mode )


( copy-face 'italic  'font-lock-comment-face )
( set-face-foreground 'font-lock-comment-face "#00AA00" )
( setq font-lock-comment-face 'font-lock-comment-face )

( copy-face 'default 'font-lock-string-face )
( set-face-foreground 'font-lock-string-face "#FF00FF" )
( setq font-lock-string-face 'font-lock-string-face )

( copy-face 'bold 'font-lock-type-face )
( set-face-foreground 'font-lock-type-face "#0000FF" )
( setq font-lock-type-face 'font-lock-type-face )

( copy-face 'bold 'font-lock-keyword-face )
( set-face-foreground 'font-lock-keyword-face "#C88000" )
( setq font-lock-keyword-face 'font-lock-keyword-face )


( copy-face 'default 'font-lock-function-name-face )
( set-face-foreground 'font-lock-function-name-face "#C80000" )
( setq font-lock-function-name-face 'font-lock-function-name-face )

( copy-face 'default 'font-lock-variable-name-face )
( set-face-foreground 'font-lock-variable-name-face "#0000FF" )
( setq font-lock-variable-name-face 'font-lock-variable-name-face )

( copy-face 'bold 'font-lock-constant-face )
( set-face-foreground 'font-lock-constant-face "#C800FF" )
( setq font-lock-constant-face 'font-lock-constant-face )

( copy-face 'bold 'font-lock-warning-face )
( set-face-foreground 'font-lock-warning-face "#FF0000" )
( setq font-lock-warning-face 'font-lock-warning-face )

( copy-face 'default 'font-lock-reference-face )
( set-face-foreground 'font-lock-reference-face "#00FFFF" )
( setq font-lock-reference-face 'font-lock-reference-face )



( defconst my-c-common-style '(
	( c-tab-always-indent . t )
	( indent-tabs-mode . nil )	;; set to nil to use spaces instead of tabs
    ( c-comment-only-line-offset . 4 )
    ( c-hanging-braces-alist '(
		( defun-open before after )
		( defun-close  before  after )  
		( class-open before  after )  
		( class-close before  after ) 
		( block-open  before after )
		( block-close . c-snug-do-while )
		( substatement-open  before after )
		( statement-case-open  before after )
		( extern-lang-open  before   after )
		( extern-lang-close  before  after )
		( brace-list-open )
		( brace-list-close after )    

		( brace-list-intro )
		( brace-list-entry )
		)
	)

	( c-offsets-alist . ( 
		( string . -1000 )
		( c . c-lineup-C-comments )
		( defun-open . 0 )
		( defun-close . + )
		( defun-block-intro . + )
		( class-open . 0 )
		( class-close . + )
		( inline-open . 0 )
		( inline-close . + )
		( topmost-intro . 0 )
		( topmost-intro-cont . 0 )
		( member-init-intro . + )
		( member-init-cont . 0 )
		( inher-intro . + )
		( inher-cont . 0 )
		( block-open . 0 )
		( block-close . + )
		( brace-list-open . + )
		( brace-list-close . 0 )
		( brace-list-intro . + )
		( brace-list-entry . 0 )
		( statement . 0 )
		( statement-cont . + )
		( statement-block-intro . + )
		( statement-case-intro . + )
		( statement-case-open . + )
		( substatement . + )
		( substatement-open . 0 )
		( case-label . + )
		( access-label . - )
		( label . -1000 )
		( do-while-closure . 0 )
		( else-clause . 0 )
		( comment-intro . 0 )
		( arglist-intro . + )
		( arglist-cont . 0 )
		( arglist-cont-nonempty . c-lineup-arglist )
		( arglist-close . 0 )
		( stream-op . + )
		( inclass . + )
		( cpp-macro . -1000 )
		( friend . 0 )
		( objc-method-intro . 0 )
		( objc-method-args-cont . 0 )
		( objc-method-call-cont . 0 )
		)
	)

    ( c-echo-syntactic-information-p . t )
    ( c-basic-offset . 4 )
    )
	"Jason Rohrer C/C++/Java" )




; For c and c++, we want inclass items to be indented
; further (underneath access labels),
; so override the inclass indent.
( defconst my-c-c++-style '(

	( c-offsets-alist . ( 
		( inclass . ++ )
		)
	)
	)
	"Jason Rohrer C/C++" )



	
( defun my-c-mode-common-hook ()
	;; add my personal style and set it for the current buffer
	( c-add-style "user" my-c-common-style t )
	
	;; keybindings for all supported languages.  We can put these in
	;; c-mode-base-map because c-mode-map, c++-mode-map, objc-mode-map,
	;; java-mode-map, and idl-mode-map inherit from it.
	( define-key c-mode-base-map "\C-m" 'newline-and-indent )
)


( defun my-c-c++-mode-hook ()
	;; add my personal style and set it for the current buffer
	( c-add-style "user" my-c-c++-style t )
)


( defun my-perl-mode-hook ()
  ( setq indent-tabs-mode nil )	;; set to nil to use spaces instead of tabs
  )


( add-hook 'c-mode-common-hook 'my-c-mode-common-hook )

; only add to c/c++, since we don't want
; inclass items doubly-indented in java
( add-hook 'c-mode-hook 'my-c-c++-mode-hook )
( add-hook 'c++-mode-hook 'my-c-c++-mode-hook )



( add-hook 'perl-mode-hook 'my-perl-mode-hook )



