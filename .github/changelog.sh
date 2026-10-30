#!/bin/sh
# CHANGELOG.md helpers for CI. The YAOPL sections are "## Unreleased" and one "## VERSION (YYYY-MM-DD)" per
# release, newest first; their entries are "- Area: text. (commits)", continued on lines indented by two.
#   sh .github/changelog.sh section NAME     print the entries of section NAME (Unreleased, 2.0.0-rc1, 2.0.0);
#                                            exit 4 if there is no such section
#   sh .github/changelog.sh notes VERSION    the entries for the release notes of VERSION: its section and,
#                                            for a release, the sections of its candidates under their headings;
#                                            exit 4 if VERSION has no section
#   sh .github/changelog.sh since BASE       print the YAOPL entries added or changed since commit BASE; exit 3
#                                            if BASE has no CHANGELOG.md in this format
#   sh .github/changelog.sh check BASE HEAD  fail if BASE..HEAD changes what users get but not CHANGELOG.md, or
#                                            if CHANGELOG.md has no Unreleased section; a commit message with a
#                                            line "[no changelog]" skips the first check

# A YAOPL section heading: "## Unreleased" or "## 2.0.0-rc1 (2026-10-28)".
YAOPL_HEADING='^## (Unreleased|[0-9][^ ]*( [(].*[)])?)$'
CR=$(printf '\r')

# The file $1 of commit $2 (the working tree without $2), with LF line ends.
changelog() {
	if [ -n "$2" ]; then git show "$2:$1"; else cat "$1"; fi | tr -d "$CR"
}

# One line per entry of the YAOPL sections, its lines separated by tabs.
entries() {
	awk -v h="$YAOPL_HEADING" '
	function flush() { if (cur != "") print cur; cur = "" }
	/^## / { flush(); yaopl = $0 ~ h; next }
	!yaopl { next }
	/^- / { flush(); cur = $0; next }
	/^  / && cur != "" { cur = cur "\t" $0; next }
	{ flush() }
	END { flush() }'
}

# The lines of section $1 without its heading and the blank lines around them.
section() {
	changelog CHANGELOG.md | awk -v name="$1" '
	/^## / { if (p) exit; p = ($0 == "## " name || index($0, "## " name " (") == 1); next }
	p { lines[++n] = $0 }
	END {
		if (!p) exit 4
		first = 1; while (first <= n && lines[first] == "") first++
		while (n >= first && lines[n] == "") n--
		for (i = first; i <= n; i++) print lines[i]
	}'
}

case "$1" in
section)
	section "$2"
	;;
notes)
	section "$2" || exit 4
	case "$2" in
	*-*) ;;
	*)
		# A release lists its candidates' sections too, newest first.
		changelog CHANGELOG.md | grep -F "## $2-" | grep '^## ' | while read -r heading; do
			name=${heading#'## '}
			printf '\n### %s\n\n' "$name"
			section "${name%% (*}"
		done
		;;
	esac
	;;
since)
	if [ -z "$2" ] || ! changelog CHANGELOG.md "$2" 2>/dev/null | grep -q '^## Unreleased$'; then
		exit 3
	fi
	old=$(mktemp)
	changelog CHANGELOG.md "$2" | entries > "$old"
	# An entry is new if its text is new: moving it from Unreleased to a release does not count.
	changelog CHANGELOG.md | entries | awk -F '\t' '
	NR == FNR { seen[$0] = 1; next }
	!($0 in seen) { for (i = 1; i <= NF; i++) print $i }' "$old" -
	rm -f "$old"
	;;
check)
	base=$2
	head=${3:-HEAD}
	if ! changelog CHANGELOG.md "$head" | grep -q '^## Unreleased$'; then
		echo "::error::CHANGELOG.md has no '## Unreleased' section: a release renames it and starts a new one" \
			"(see CONTRIBUTING.md, \"Builds and versions\")"
		exit 1
	fi
	if [ -z "$base" ] || ! git cat-file -e "$base^{commit}" 2>/dev/null; then
		echo "No base commit to compare with, CHANGELOG.md not checked"
		exit 0
	fi
	if git log --format=%B "$base..$head" | grep -qxF '[no changelog]'; then
		echo "A commit says [no changelog], CHANGELOG.md not checked"
		exit 0
	fi
	files=$(git diff --name-only "$base" "$head")
	if echo "$files" | grep -qx 'CHANGELOG.md'; then
		echo "CHANGELOG.md is updated"
		exit 0
	fi
	# Host checks, docs, CI and repository files do not change what users get.
	code=$(echo "$files" | grep -vE '^(labs/|notes/|docs/|\.github/|[^/]*\.md$|mkdocs\.yml$|CREDITS$|LICENSE$|\.[^/]*$)')
	if [ -n "$code" ]; then
		echo "::error::These files change what users get, but CHANGELOG.md has no new line (see CONTRIBUTING.md;" \
			"if nothing user-visible changed, add a line [no changelog] to a commit message):"
		echo "$code"
		exit 1
	fi
	echo "Only docs, host checks or CI changed"
	;;
*)
	echo "usage: $0 section NAME | notes VERSION | since BASE | check BASE [HEAD]" >&2
	exit 2
	;;
esac
