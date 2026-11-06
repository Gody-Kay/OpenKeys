#!/usr/bin/env python3
"""Record laptop specs into the Benchmarks line of PROGRESS.md.

Usage:
  python3 tools/benchmark.py            # print specs only
  python3 tools/benchmark.py --write    # also update PROGRESS.md
Latency / 64-voice CPU / RAM-in-use are filled in later, once the engine exists.
"""
import os, pathlib, platform, re, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent

def cpu_name() -> str:
    try:
        for line in open("/proc/cpuinfo", encoding="utf-8"):
            if line.startswith("model name"):
                return line.split(":", 1)[1].strip()
    except OSError:
        pass
    return platform.processor() or platform.machine()

def ram_gb() -> str:
    try:
        for line in open("/proc/meminfo", encoding="utf-8"):
            if line.startswith("MemTotal"):
                return f"{int(line.split()[1]) / 1048576:.1f} GB RAM"
    except OSError:
        pass
    return "RAM unknown"

def main() -> int:
    spec = f"{cpu_name()}, {os.cpu_count()} threads, {ram_gb()}, {platform.system()} {platform.release()}"
    print(spec)
    if "--write" in sys.argv:
        p = ROOT / "PROGRESS.md"
        s = p.read_text(encoding="utf-8")
        new = re.sub(r"Laptop:[^|\n]*", f"Laptop: {spec} ", s, count=1)
        if new == s:
            print("no 'Laptop:' field found in PROGRESS.md")
            return 1
        p.write_text(new, encoding="utf-8")
        print("PROGRESS.md updated")
    return 0

if __name__ == "__main__":
    sys.exit(main())
