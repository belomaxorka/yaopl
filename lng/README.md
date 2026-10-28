# Translation guide

## For translators

YAOPL takes its translations from <https://github.com/yaopl/yaopl-lang>, a fork of OPL's language repository\\
Provide your submissions there.\\
Languages were moved there for reducing commit stress for the main repository.\\
Check if your `.yml` file has untranslated strings. Example:

```yml
  MENU:
  - untranslated
  - original: Menu
```

You should change this into:

```yml
  MENU: Menu_on_your_language
```

If you want to test your changes:

- run `make download_lng`
- make your changes int `lng_src/*.yml`
- run `make languages`
- test generated file from `lng/lang_*.lng`.

## For developers

If you add the language string into the code,\\
propose your changes into `lng_tmpl/_base.yml` with the necessary comments.\\
For example, if you add the string `_STR_NETWORK_STARTUP_ERROR`,\\
then you should update the base file like:

```yml
- comment: Generic network error message.
  label: NETWORK_STARTUP_ERROR
  string: '%d: Network startup error.'
```

After running `make languages` propose the new entries to [the language repo](https://github.com/yaopl/yaopl-lang);\\
its README says how they are added without reformatting the files.

It is not recommended to rename or remove already existing strings.\\
In such a case, you will need to edit all language yml files manually.
