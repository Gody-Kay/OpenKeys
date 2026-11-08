# Sounds (not stored in git)
Samples live in `content/sounds/` (gitignored). Every library must have a row in `content/LICENSES.md`.

| Library | Licence | Size | Get it |
|---|---|---|---|
| Salamander Grand Piano V3 | CC-BY-3.0 (credit Alexander Holm) | ~700 MB | `bash tools/fetch_salamander.sh` |
| VSCO 2 Community Edition | CC0-1.0 (credit Versilian Studios requested) | ~2.3 GB | `bash tools/fetch_vsco.sh` then `bash tools/finish_vsco.sh` |

Presets in `content/presets/` reference sounds relative to `content/sounds/`. `tools/check_presets.py` checks the references when the sounds are present.
Status: scripts written from the commands that worked on the maintainer's laptop (Linux). Windows/macOS not tested.
