# Orchestration Workflow

## Order of work

Every task runs these steps in this order. Roles are defined in `AGENTS.md`;
each role's scope is in `.ai/<role>.md`.

| # | Step | Role | Runs |
|---|---|---|---|
| 1 | Tasks with acceptance criteria | Planner | Always |
| 2 | Structure and decisions | Architect | Always |
| 3 | Implementation — own scope only | Backend / Frontend | Always |
| 4 | Prove the acceptance criteria | Testing | Always |
| 5 | Exploitable defects | Security | Conditional — `SECURITY.md` triggers |
| 6 | Measured regressions | Performance | Conditional — change touches a hot path |
| 7 | Record what shipped | Documentation | Conditional — public behavior changed |
| 8 | Verify every criterion, issue the verdict | Reviewer | Always |
| 9 | Notes and rollback | Release | Conditional — preparing a deployment |

```text
Planner → Architect → Backend / Frontend → Testing
                                              ↓
                                  Security · Performance
                                              ↓
                                       Documentation
                                              ↓
                                        Reviewer → Release
```

## Rules

- **Read `.ai/<role>.md` before acting in that role.**
- **A role does only its own work.** Scope creep across roles defeats the
  separation that makes review meaningful.
- **Do not skip a step because the change looks small.** Skipping requires the
  user's stated agreement.
- **A skipped conditional step is recorded NOT RUN with a reason** in the
  report and in `TASKS.md` — never omitted silently.
- **Steps may run in parallel where genuinely independent** — Security and
  Performance do not depend on each other.
- **Escalate backward, not forward.** A review finding that invalidates the
  design returns to Architect, not to the implementer. Backward transitions
  are expected and are not failures.
- **The Reviewer is never the implementer.** No role approves its own work.

### Context

- Keep context small.
- Load only required files.
- Never send the entire repository unless necessary.
- Reuse previous work instead of regenerating it.
- Verify before completing.

This ordering is this project's convention, enforced because consistency is
worth more than per-task optimization — not because it is provably best for
every model or framework.

## Token Management

If an agent is approaching its context limit:

1. Summarize completed work.
2. Save decisions.
3. Hand off to the next agent.
4. Continue from the summary.
5. Do not restart work.
6. Preserve acceptance criteria.

### What handoffs do and do not buy you

Switching agents at the context limit is a common strategy, but it does **not**
automatically reduce token usage. Every handoff requires a summary, and that
summary costs tokens to write and to read. A handoff trades a large live
context for a smaller lossy one — it buys headroom, not savings.

Total cost is minimized by:

- **Targeted file selection** — read the three files that matter, not the
  directory that contains them.
- **Context reuse** — keep a stable prefix so caching applies; avoid rewriting
  the system prompt or tool set mid-run.
- **Concise handoffs** — carry acceptance criteria and decisions forward, drop
  the narrative of how the work happened.

Hand off when context is genuinely the constraint, not on a fixed cadence.
Frequent switching costs more than it saves.

## Completion Definition

A task is complete only when:

✓ Code compiles
✓ Every test-matrix category reported, none FAIL — see `TESTING.md`
✓ Documentation updated
✓ No placeholder implementations
✓ No unfinished files
✓ Acceptance criteria satisfied

A check that was not run is not a check that passed. If something could not be
verified, say so explicitly and name what is unverified.
