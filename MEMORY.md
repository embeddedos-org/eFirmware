# Project Memory

What survives a session boundary. Sessions are disposable; this is not.

## What belongs here

| Kind | Preserve | Lossy? |
|---|---|---|
| **Requirements** | Verbatim | Never — this is the contract |
| **Architecture decisions** | Decision + alternatives rejected + reason | Never |
| **Task status** | Open, in progress, blocked, done, with owner | Never |
| **Resume summaries** | Narrative of completed work | Yes — this is the only lossy layer |
| **Known risks** | What could break and what would reveal it | Never |

Requirements and decisions are copied forward unchanged. Only the narrative of
how work happened gets compressed.

## Where it lives

- `TASKS.md` — task status, acceptance criteria, decisions table
- `ORCHESTRATION.md` — workflow conventions
- Handoff records — per HANDOFF.md

## Resume order

On starting a session that continues prior work, read in this order:

1. Requirements — the contract
2. Architecture decisions — the constraints
3. Open tasks and their acceptance criteria
4. The latest resume summary — the narrative

A session that starts from the summary alone has lost the contract and will
drift. The summary is the last thing you read, not the first.

## What does not belong here

- Anything the repository already records — code structure, git history, past
  fixes. Duplicating them creates a second source of truth that drifts.
- Secrets, tokens, keys, or credentials of any kind. Memory is replayed into
  future contexts; a secret written once is replayed indefinitely.
- Speculation. Record what was decided, not what might be decided.

## Maintenance

- Update an existing entry rather than appending a contradicting one.
- Delete entries that turn out to be wrong. A stale decision is worse than a
  missing one, because it is trusted.
- Convert relative dates to absolute ones — "last week" is meaningless on
  resume.
