;;; textarc-mode.el --- Major mode for text archive files
;;
;; Copyright 2018 Lassi Kortela
;;
;; Author: Lassi Kortela <lassi@lassi.io>
;; URL: https://github.com/lassik/textarc
;; Version: 0.1.0
;; Keywords: archive
;; License: ISC
;;
;; This file is not part of GNU Emacs.
;;
;;; Commentary:
;;
;; Major mode for text archive files
;;
;;; Code:

(defconst textarc-mode-font-lock-keywords
  '(("^\\:.*" 0 'font-lock-string-face)
    ("^>.*" 0 'font-lock-preprocessor-face)
    ("^\\$.*" 0 'font-lock-string-face)
    ("^\\#.*" 0 'font-lock-comment-face)
    ("^\\(end\\|entry\\|format\\).*" 0 'font-lock-keyword-face)
    ("^\\(cksum\\|gid\\|gname\\|link\\|mode\\|size\\|time\\|type\\|uid\\|uname\\)\\(\\).*"
     0 'font-lock-type-face)))

;;;###autoload
(define-derived-mode textarc-mode fundamental-mode "TextArc"
  "Major mode for editing text archive files."
  (set (make-local-variable 'paragraph-separate) "[ \t]*$")
  (set (make-local-variable 'paragraph-start) "[ \t]*$")
  (set (make-local-variable 'comment-start) "#")
  (set (make-local-variable 'font-lock-defaults)
       '(textarc-mode-font-lock-keywords t)))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.textarc$" . textarc-mode))

(provide 'textarc)
