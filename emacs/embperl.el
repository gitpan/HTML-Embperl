;; embperl-minor-mode -- Embedded Perl mode for XEmacs.
;;
;; Copyright (C) 1998-1999 Erik L. Arneson
;;
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 1, or (at your option)
;; any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software
;; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;;
;; Author          : Erik Arneson (erik@mind.net)
;; Created On      : Wed Jul 22 17:16:39 PDT 1998
;; Last Modified By: Erik Arneson
;; Last Modified On: $Date: 1999/02/06 00:07:40 $
;; Version         : 1.00
;; $Id: embperl.el,v 1.1 1999/02/06 00:07:40 erik Exp erik $
;;
;; Please note that this software is very beta and rather broken.  I
;; don't know how useful it will be, although I definitely plan on
;; making it more useable in the future.  If you don't know Emacs Lisp
;; well enough to read this, then you probably shouldn't be using it at
;; all.  But also, if you can figure out Perl and HTML, Emacs Lisp
;; shouldn't be that big a jump, and learning it is well worth your
;; time.

(defvar embperl-minor-mode nil
  "T, if the region is active in the `embperl-minor-mode'.")

(make-variable-buffer-local 'embperl-minor-mode)
(defvar embperl-minor-mode-map (make-sparse-keymap))

(add-minor-mode 'embperl-minor-mode " Embperl" embperl-minor-mode-map)

(define-key embperl-minor-mode-map "\C-cp" 'embperl-narrow-to-perl)
(define-key embperl-minor-mode-map "\C-cc" 'embperl-widen)

(defun embperl-minor-mode (&optional arg)
  "Toggle Embperl minor mode."
  (interactive "P")
  (setq embperl-minor-mode
        (if (null arg)
	    (not embperl-minor-mode)
	  (> (prefix-numeric-value arg) 0))))

(defun embperl-widen ()
  "Widens the buffer back to its original state."
  (interactive)
  (widen)
  (html-mode)
  (embperl-minor-mode t))


(defun embperl-narrow-to-perl ()
  "Narrows the edit-able part of a buffer to an embedded Perl chunk and activates cperl-mode."
  (interactive)
  (let (begin end)
    (save-excursion
      (search-backward-regexp "\\[[-!+\\$\\*]" nil t)
      (setq begin (point)))
    (save-excursion
      (search-forward-regexp "[-!+\\$\\*]\\]" nil t)
      (setq end (point)))
    (narrow-to-region begin end))
  (cperl-mode)
  (embperl-minor-mode t))

;; End
