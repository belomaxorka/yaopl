# Translations

YAOPL's menu is translated into 33 languages. The translations are made in
[yaopl/yaopl-lang](https://github.com/yaopl/yaopl-lang), a fork of OPL's language repository that adds the
strings only YAOPL has. Every build packs them into `OPNPS2LD-LANGS-<version>.7z`; how to install one is in
[Art, themes and languages](features/customizing.md#languages). A string that is not translated yet is shown
in English.

## How complete each language is

*Ready* is the share of YAOPL's strings that are translated, or intentionally left in English (device names,
for example), rounded down. *Own font* means the package has a font file for the language, to copy next to
its `.lng` file. Arabic and Persian are written right to left and need an `EXTRA_FEATURES=1` ELF from the
`OPNPS2LD-VARIANTS` package.

As of 2026-10-29; the current numbers are in the summary of the latest
[CI-compile run](https://github.com/yaopl/yaopl-lang/actions/workflows/compilation.yml?query=branch%3Amain)
of yaopl-lang.

| Language | Ready | Untranslated | Own font |
| --- | ---: | ---: | --- |
| Albanian | 72% | 97 | |
| Arabic | 57% | 150 | yes |
| Bulgarian | 81% | 65 | yes |
| Cebuano | 65% | 122 | |
| Croatian | 67% | 115 | yes |
| Czech | 90% | 33 | yes |
| Danish | 67% | 114 | yes |
| Dutch | 53% | 163 | yes |
| Filipino | 62% | 130 | yes |
| French | 70% | 103 | yes |
| Galician | 90% | 33 | yes |
| German | 88% | 41 | yes |
| Greek | 72% | 95 | yes |
| Hungarian | 82% | 62 | yes |
| Indonesian | 64% | 124 | yes |
| Italian | 89% | 35 | yes |
| Japanese | 88% | 41 | yes |
| Korean | 89% | 37 | yes |
| Laotian | 52% | 167 | yes |
| Persian | 72% | 96 | yes |
| Polish | 80% | 68 | yes |
| Portuguese | 85% | 52 | yes |
| Portuguese_BR | 86% | 46 | yes |
| Romana | 56% | 152 | yes |
| Russian | 100% | 0 | yes |
| Ryukyuan | 89% | 37 | |
| SChinese | 86% | 46 | yes |
| Serbian | 93% | 21 | |
| Spanish | 90% | 33 | yes |
| Swedish | 67% | 112 | yes |
| TChinese | 70% | 102 | yes |
| Turkish | 87% | 44 | yes |
| Vietnamese | 57% | 148 | yes |

The names are those of the files, `lang_<Language>.lng`: Romana is Romanian, SChinese and TChinese are
Simplified and Traditional Chinese, Portuguese_BR is Brazilian Portuguese.

## Helping

To translate a string, correct one or add a language, edit `<Language>.yml` in
[yaopl/yaopl-lang](https://github.com/yaopl/yaopl-lang) and open a pull request; its
[README](https://github.com/yaopl/yaopl-lang#readme) explains the file format. The pull request's CI compiles
every language and keeps the `.lng` files as an artifact for testing on a console. It also fails when a
translated string does not keep the `%s`, `%d` or `%i` placeholders of the English one in the same order,
because such a string can crash the menu.

Problems with a translation can be reported as an [issue](https://github.com/yaopl/yaopl/issues); name the
language.
