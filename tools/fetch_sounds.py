#!/usr/bin/env python3
"""Download free-licence sound libraries listed in content/sources.json.

Usage:
  python3 tools/fetch_sounds.py --list
  python3 tools/fetch_sounds.py --dry-run
  python3 tools/fetch_sounds.py [--only NAME] [--dest content/sounds]

Refuses any library whose licence is not on the allow-list or that is not
logged in content/LICENSES.md. Verifies sha256 when given. Unzips .zip files.
"""
import argparse, hashlib, json, pathlib, sys, urllib.request, zipfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
from check_licenses import ALLOWED  # noqa: E402

def sha256(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()

def fetch(lib: dict, dest_root: pathlib.Path, dry: bool, licenses_text: str) -> bool:
    name = lib["name"]
    if lib.get("license") not in ALLOWED:
        print(f"[skip] {name}: licence {lib.get('license')!r} not allowed")
        return False
    if name not in licenses_text:
        print(f"[skip] {name}: not logged in content/LICENSES.md")
        return False
    if not lib.get("sha256"):
        print(f"[skip] {name}: no sha256 in sources.json (use --hash FILE)")
        return False
    dest = dest_root / lib.get("dest", name)
    print(f"[{'plan' if dry else 'get '}] {name} -> {dest}")
    if dry:
        return True
    dest.mkdir(parents=True, exist_ok=True)
    fname = dest / pathlib.Path(lib["url"]).name
    if not (fname.exists() and sha256(fname) == lib["sha256"]):
        urllib.request.urlretrieve(lib["url"], fname)
    want = lib.get("sha256")
    if want and sha256(fname) != want:
        fname.unlink()
        print(f"[fail] {name}: checksum mismatch")
        return False
    if fname.suffix == ".zip":
        with zipfile.ZipFile(fname) as z:
            base = dest.resolve()
            for m in z.namelist():
                if not (dest / m).resolve().is_relative_to(base):
                    print(f"[fail] {name}: unsafe path in zip: {m}")
                    return False
            z.extractall(dest)
    return True

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--only")
    ap.add_argument("--hash", help="print sha256 of a local file, then exit")
    ap.add_argument("--dest", default=str(ROOT / "content" / "sounds"))
    a = ap.parse_args()
    if a.hash:
        print(sha256(pathlib.Path(a.hash)))
        return 0

    libs = json.loads((ROOT / "content" / "sources.json").read_text(encoding="utf-8")).get("libraries", [])
    if a.only:
        libs = [l for l in libs if l["name"] == a.only]
    if a.list or not libs:
        for l in libs:
            print(f"{l['name']}  [{l.get('license')}]  {l['url']}")
        if not libs:
            print("no libraries listed in content/sources.json yet")
        return 0
    text = (ROOT / "content" / "LICENSES.md").read_text(encoding="utf-8")
    ok = [fetch(l, pathlib.Path(a.dest), a.dry_run, text) for l in libs]
    return 0 if all(ok) else 1

if __name__ == "__main__":
    sys.exit(main())
