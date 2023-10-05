(lang dune 3.8)
(using ctypes 0.1)
(using cinaps 1.3)

(name smtcaml)

(generate_opam_files true)

(source
 (github smuenzel/smtcaml))

(authors "Stefan Muenzel")

(maintainers "Stefan Muenzel")

(license MPL-2.0)

(documentation https://url/to/documentation)

(package
 (name smtcaml)
 (synopsis "A short synopsis")
 (description "A longer description")
 (depends ocaml dune)
 (allow_empty true)
 (tags
  (topics "to describe" your project)))

; See the complete stanza docs at https://dune.readthedocs.io/en/stable/dune-files.html#dune-project
