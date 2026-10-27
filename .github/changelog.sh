#!/bin/sh
# CHANGELOG.md helpers for CI.
#   sh .github/changelog.sh latest           print the newest YAOPL section (for the release notes)
#   sh .github/changelog.sh check BASE HEAD  fail if BASE..HEAD changes what users get but not CHANGELOG.md;
#                                            a commit message with a line "[no changelog]" skips it

case "$1" in
latest)
	awk '/^## / { if (p) exit } /^## YAOPL / { p = 1 } p' CHANGELOG.md
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
	code=$(echo "$files" | grep -vE '^(labs/|notes/|\.github/|[^/]*\.md$|CREDITS$|LICENSE$|\.[^/]*$)')
	if [ -n "$code" ]; then
		echo "::error::These files change what users get, but CHANGELOG.md has no new line (see CLAUDE.md;" \
			"if nothing user-visible changed, add a line [no changelog] to a commit message):"
		echo "$code"
		exit 1
	fi
	echo "Only docs, host checks or CI changed"
	;;
*)
	echo "usage: $0 latest | check BASE [HEAD]" >&2
	exit 2
	;;
esac
