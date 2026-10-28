#!/bin/sh
# CHANGELOG.md helpers for CI.
#   sh .github/changelog.sh latest           print the newest YAOPL section
#   sh .github/changelog.sh since BASE       print the YAOPL entries added or changed since commit BASE,
#                                            under their headings (for the release notes); exit 3 if
#                                            BASE has no CHANGELOG.md
#   sh .github/changelog.sh check BASE HEAD  fail if BASE..HEAD changes what users get but not CHANGELOG.md;
#                                            a commit message with a line "[no changelog]" skips it

# One line per entry of the YAOPL sections: section, area and the entry's lines, separated by tabs.
entries() {
	awk '
	function flush() { if (cur != "") print sec "\t" area "\t" cur; cur = "" }
	/^## / { flush(); sec = /^## YAOPL / ? $0 : ""; area = ""; next }
	/^### / { flush(); area = $0; next }
	sec == "" { next }
	/^- / { flush(); cur = $0; next }
	/^  / && cur != "" { cur = cur "\t" $0; next }
	{ flush() }
	END { flush() }'
}

case "$1" in
latest)
	awk '/^## / { if (p) exit } /^## YAOPL / { p = 1 } p' CHANGELOG.md
	;;
since)
	if [ -z "$2" ] || ! git cat-file -e "$2:CHANGELOG.md" 2>/dev/null; then
		exit 3
	fi
	old=$(mktemp)
	git show "$2:CHANGELOG.md" | entries > "$old"
	entries < CHANGELOG.md | awk -F '\t' '
	NR == FNR { seen[$0] = 1; next }
	!($0 in seen) {
		if ($1 != sec) { printf "%s%s\n", n++ ? "\n" : "", $1; sec = $1; area = "" }
		if ($2 != area && $2 != "") printf "\n%s\n", $2
		area = $2
		for (i = 3; i <= NF; i++) print $i
	}' "$old" -
	rm -f "$old"
	;;
check)
	base=$2
	head=${3:-HEAD}
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
		echo "::error::These files change what users get, but CHANGELOG.md has no new line (see CLAUDE.md;" \
			"if nothing user-visible changed, add a line [no changelog] to a commit message):"
		echo "$code"
		exit 1
	fi
	echo "Only docs, host checks or CI changed"
	;;
*)
	echo "usage: $0 latest | since BASE | check BASE [HEAD]" >&2
	exit 2
	;;
esac
