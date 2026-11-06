# ci/

The GitHub Actions workflow must live at `.github/workflows/ci.yml` (GitHub requires that path).
Lane C owns it. Put helper scripts for CI in this folder.

Rules (assumed, see DECISIONS.md):
- Linux must be green before a merge to `main`.
- Windows and Mac jobs are allowed to fail until someone can test them.
