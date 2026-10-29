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
| Albanian | 71% | 100 | |
| Arabic | 56% | 153 | yes |
| Bulgarian | 80% | 68 | yes |
| Cebuano | 64% | 125 | |
| Croatian | 66% | 118 | yes |
| Czech | 89% | 36 | yes |
| Danish | 66% | 117 | yes |
| Dutch | 52% | 166 | yes |
| Filipino | 62% | 133 | yes |
| French | 69% | 106 | yes |
| Galician | 89% | 36 | yes |
| German | 87% | 44 | yes |
| Greek | 72% | 98 | yes |
| Hungarian | 81% | 65 | yes |
| Indonesian | 63% | 127 | yes |
| Italian | 89% | 38 | yes |
| Japanese | 87% | 44 | yes |
| Korean | 88% | 40 | yes |
| Laotian | 51% | 170 | yes |
| Persian | 71% | 99 | yes |
| Polish | 79% | 71 | yes |
| Portuguese | 84% | 55 | yes |
| Portuguese_BR | 86% | 49 | yes |
| Romana | 55% | 155 | yes |
| Russian | 100% | 0 | yes |
| Ryukyuan | 88% | 40 | |
| SChinese | 86% | 49 | yes |
| Serbian | 93% | 24 | |
| Spanish | 89% | 36 | yes |
| Swedish | 67% | 115 | yes |
| TChinese | 70% | 105 | yes |
| Turkish | 86% | 47 | yes |
| Vietnamese | 56% | 151 | yes |

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
