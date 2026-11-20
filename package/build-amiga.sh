#!/bin/sh
# The release archive, for Cradle's stove_build (target package), as
# amigadosbox's package/build-amiga.sh: Oolite linked by
# tools/cradle-fulllink/build-amiga.sh with the stove (STOVE=), then
# tools/make_package.py packs oolite-VERSION-68040.lha with its .sha256 and
# .readme: the Oolite drawer with its icons (icons/), the game (oolite),
# Oolite-Start (package/files/), the ReadMe, upstream's Resources, the
# libraries the drawer carries in Libs/ (package/libs.json, each checked by
# SHA-256) and the licences; and beside the drawer the Amiga Installer script
# Install-Oolite and its icon (package/installer/, made by amicradle
# scripts/ports/port_installer.py from package/installer.json).
#
#   OOLITE_BUILD   a full-link OUT that already holds oolite-68040 (skips the link)
#   UPSTREAM       Oolite's checkout at UPSTREAM.json's commit (default: the full link's)
#   OOLITE_JSLIB, OOLITE_JSENGINE, OOLITE_SDL3MODULE   the libraries (package/libs.json)
#
# VERSION is package/VERSION. Nothing is published from here.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/.." && pwd)
OUT=${OUT:-"$ROOT/build-amiga-package"}
VERSION=$(cat "$HERE/VERSION")
mkdir -p "$OUT"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
BUILD="$OUT/build"
rm -rf "$BUILD"
mkdir -p "$BUILD"
UCOMMIT=$(python3 -c "import json,sys; print(json.load(open(sys.argv[1]))['commit'])" "$ROOT/UPSTREAM.json")
U=${UPSTREAM:-${OOLITE_WORK:-$HOME/t5scratch/oolite-20261119}/oolite}
test "$(git -C "$U" rev-parse HEAD)" = "$UCOMMIT" || { echo "UPSTREAM $U is not at $UCOMMIT"; exit 1; }

if [ -z "${OOLITE_BUILD:-}" ]; then
  : "${STOVE:?set STOVE to a Cradle stove root, or OOLITE_BUILD to a full link's OUT}"
  OOLITE_BUILD="$OUT/fulllink"
  OUT="$OOLITE_BUILD" sh "$ROOT/tools/cradle-fulllink/build-amiga.sh" > "$OUT/build-output.txt" 2>&1 ||
    { tail -n 60 "$OUT/build-output.txt"; exit 1; }
fi
test -s "$OOLITE_BUILD/oolite-68040" || { echo "no oolite-68040 in $OOLITE_BUILD"; exit 1; }
cp "$OOLITE_BUILD/oolite-68040" "$BUILD/oolite"

# the drawer's data: upstream's Resources, the libraries, the licences
STAGE="$BUILD/stage"
mkdir -p "$STAGE/licences"
cp -R "$U/Resources" "$STAGE/Resources"
cp "$U/LICENSE.md" "$STAGE/licences/Oolite-LICENSE.md"
cp "$HERE/files/GPL-2.0.txt" "$STAGE/licences/Oolite-GPL-2.0.txt"
python3 - "$HERE/libs.json" "$STAGE" <<'PY'
import hashlib, json, os, shutil, sys
from pathlib import Path
spec, stage = json.load(open(sys.argv[1])), Path(sys.argv[2])
base = Path(os.path.expanduser(spec["default_dir"]))
for lib in spec["libs"]:
    src = Path(os.path.expanduser(os.environ.get(lib["env"]) or base / lib["drawer"]))
    got = hashlib.sha256(src.read_bytes()).hexdigest()
    if got != lib["sha256"]:
        raise SystemExit(f"{src}: sha256 {got}, package/libs.json pins {lib['sha256']} ({lib['what']})")
    dst = stage / lib["drawer"]
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, dst)
    print(f"{lib['drawer']}: {src}")
PY
(cd "$STAGE" && find . -type f | sed 's|^\./||' | LC_ALL=C sort > "$BUILD/data.list" &&
    tar --format=ustar --owner=0 --group=0 --numeric-owner -cf "$BUILD/data.tar" -T "$BUILD/data.list")

python3 - "$ROOT" "$BUILD" "$UCOMMIT" <<'PY'
import hashlib, json, subprocess, sys
from pathlib import Path
root, build, ucommit = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
git = lambda *a: subprocess.run(["git", "-C", str(root), *a], capture_output=True, text=True, check=True).stdout.strip()
receipt = {
    "executable_sha256": sha(build / "oolite"), "executable_bytes": (build / "oolite").stat().st_size,
    "data_tar_sha256": sha(build / "data.tar"), "data_files": len((build / "data.list").read_text().splitlines()),
    "upstream": "https://github.com/OoliteProject/oolite", "upstream_commit": ucommit,
    "repository_commit": git("rev-parse", "HEAD"), "source_date_epoch": git("log", "-1", "--format=%ct"),
}
(build / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
print(json.dumps(receipt, indent=2))
PY
cp "$BUILD/receipt.json" "$OUT/receipt.json"
python3 "$ROOT/tools/make_package.py" --build "$BUILD" --version "$VERSION" --out "$OUT"
