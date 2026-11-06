# SESSION RULES (read this first, every session)

Project working title: **OpenKeys** (keyboard + studio instrument).
Three sessions work in parallel: **A = Engine**, **B = UI**, **C = Content & Build**.

## 1. Start of session (3 reads max)
1. This file.
2. Last 40 lines of `PROGRESS.md`.
3. Your lane's current task in `PLAN.md`.
Read nothing else until a task needs it. Use grep / line ranges, never whole big files.

## 2. Act first, talk less
- Name the target file(s) for the task, write the code, build it, run it.
- Replies are 5 lines max: files changed, command run + result, next step.
- No recaps, no tutorials, no pasted code in chat, no restating the plan.
- A task is NOT done until a passing build AND a passing test/audio-render are shown in command output.
- Run `tail -n 30` on long logs, never print them whole.

## 3. When to ask (and when not to)
Ask ONE multiple-choice question (recommended option first) ONLY before:
- changing a shared interface (`src/common`, Performance JSON schema)
- adding/removing a dependency or changing a licence
- deleting or renaming files outside your lane
- any choice that would cost more than 1 hour to undo
For everything else: decide, do it, add a one-line entry to `DECISIONS.md`.
If the question gets no answer, take the recommended option and log it as "assumed".

## 4. Lanes (stay inside yours)
| Session | Owns |
|---|---|
| A Engine | `src/engine`, `src/dsp`, `src/synth`, `tests/engine` |
| B UI | `src/ui`, `src/app`, `assets/ui`, `tests/ui` |
| C Content & Build | `tools/`, `content/`, `ci/`, `installers/`, `tests/integration`, `docs/` |
| Shared | `src/common` (interfaces): A owns it. Others ask via REQUESTS in `PROGRESS.md` |

Never edit another lane's files. Write a request in `PROGRESS.md` instead.

## 5. Git
- One branch per session: `session-a`, `session-b`, `session-c`.
- Small commits: `area: what changed`.
- Before a task: `git pull --rebase origin main`. After a task passes build + tests: merge to `main`.
- Never force-push `main`.

## 6. Code rules
- C++17, JUCE. Real-time audio thread: no allocation, no locks, no file access, no logging.
- Every DSP module gets a unit test or an offline render test (render to WAV, check no NaN, no clipping).
- Benchmark laptop target (4 GB RAM, Core i5, no GPU): 64 voices + effects under 60% of one core; total RAM under 1.5 GB. Record the numbers in `PROGRESS.md`.
- No dependency or sound library without a licence line in `content/LICENSES.md`. Free-to-redistribute only (CC0, CC-BY, public domain, permissive).
- Use our own names in the UI and code (Master Knob, Scenes, Setlist, Parts). No Yamaha/Roland trademarks, artwork or copied layouts.

## 7. End of session (always)
1. Update `PROGRESS.md`: Done / Next / Blocked / Requests.
2. Tick finished items in `FEATURES.md`.
3. Commit and push.

## 8. Token savers
- Do not re-read files you just wrote.
- One task at a time, finish it, then the next.
- If something is unclear and not covered in section 3, pick the simplest option that keeps the build green.
