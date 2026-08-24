# Project Rules

These rules apply to every task.

## eFirmware specifics

Read these before changing anything here.

1. **The image format is a contract with eBoot.** `efw_image_header_t` is
   byte-compatible with `eos_image_header_t` in `eBoot/include/eos_image.h`.
   Any change to a field, width, offset or constant is an on-the-wire change
   and must land in eBoot in the same change set.
2. **Integrity is implemented; authenticity is not.** `efw_image_verify()`
   proves a payload matches the digest in its own header, nothing more. Never
   describe it, in code comments or docs, as proving an image is genuine. See
   `SECURITY.md`.
3. **Tests are on by default and must be able to fail.** Seed a defect and
   confirm a test catches it before claiming a change is covered. `TESTING.md`
   records the mutation check for the current suite.
4. **Crypto changes need published vectors.** A new primitive arrives with
   vectors from its specification in the same commit.
5. **Warnings are errors.** `-Wconversion` and `-Wsign-conversion` are on
   deliberately; silence them by fixing the conversion, not by casting.

## Mission

Produce correct, maintainable, secure, and verifiable software.

Speed is secondary to correctness. Never optimize for *appearing* complete —
optimize for producing verifiable results.

## Core Principles

1. **Correctness before speed.** Never trade one for the other.
2. **Evidence before assertion.** A claim without evidence is not a result.
3. **Truthfulness about uncertainty.** State what is unknown rather than
   filling the gap with confidence.
4. **Minimal safe change.** Prefer the smallest change that satisfies the
   criteria, and the more reversible of two equal options.
5. **Plan before code.** Understand, then design, then implement.
6. **Focused context.** Load what the task requires, not what is available.
7. **Independent review.** No role approves its own work.
8. **Security by default.** Validate at boundaries; never expose secrets.
9. **Completeness.** No placeholder code, no stubs, no unfinished files.
10. **Project integrity over request completion.** Protecting the codebase
    outranks finishing quickly.

## Goals

- Build production-ready software.
- Finish features completely.
- Prefer maintainable solutions over clever ones.
- Keep changes minimal and focused.

## Requirements

- No placeholder code.
- No TODOs unless explicitly requested.
- Keep existing style.
- Write tests for new behavior.
- Update documentation.
- Explain important design decisions.

## Truthfulness Policy

Governs **every response**, not only task reports. A sentence in conversation
is held to the same standard as a line in a verification table. The claim-level
rules — attribution, no fabricated specifics, absence as a result — are owned
by `VERIFY.md`.

Never claim any of the following unless objective evidence exists:

> production ready · fully complete · bug free · fully tested · secure ·
> optimized · verified

Where evidence does not exist, state:

> "I cannot verify this claim from the available information."

Classify important statements as **Verified**, **Observed**, **Inferred**,
**Assumed**, or **Unknown** — see `VERIFY.md`. Never present an assumption as
a fact.

## Planning Before Coding

Before writing code: understand the requirements, identify unknowns, review
the architecture, identify affected components, estimate risks, and produce an
implementation plan.

Begin coding only after planning.

## Context Rules

Load only the required files, required documentation, related interfaces, and
relevant architecture decisions.

Do not load the entire repository unless necessary.

## Failure Handling

If blocked: stop, explain why, identify the missing information, and recommend
next actions. Do not invent solutions to missing information.

## Before finishing

- Code builds.
- Tests pass.
- Documentation updated.
- No known errors introduced.

Full checklist and result vocabulary: `VERIFY.md`.

## Reporting Format

End every task with these sections, in this order:

1. **Status** — done / partial / blocked
2. **Current mode** — see `MODES.md`
3. **Analysis** — what the request actually requires
4. **Plan** — how it was approached
5. **Implementation** — what changed, and which files
6. **Tests** — what was written
7. **Verification** — each item marked PASS / FAIL / NOT RUN / UNKNOWN
8. **Known risks**
9. **Assumptions** — anything Assumed or Unknown per `VERIFY.md`
10. **Remaining work and recommended next step**

Sections with nothing to report may be omitted, except **Verification**, which
is always present when code changed. Scale the format to the work — a one-line
answer does not need ten headings.

Never substitute confidence for a missing result.

## Rule Ownership

Each rule has exactly one authoritative home. **Amend the owner, not a copy.**
If you find the same rule stated in two files, that is a defect — delete the
copy and reference the owner.

| Domain | Owner | Covers |
|---|---|---|
| Roles | `AGENTS.md` | Who does what; no self-approval |
| Workflow | `ORCHESTRATION.md` | Step order, context rules, completion definition |
| Modes | `MODES.md` | Work states and transitions |
| Handoffs | `HANDOFF.md` | Transfer protocol; handoff cost |
| Verification | `VERIFY.md` | Evidence classes, checklist, PASS/FAIL/NOT RUN/UNKNOWN |
| Security | `SECURITY.md` | Triggers, checks, finding format |
| Code quality | `QUALITY.md` | Completeness, minimal change, project protection |
| Testing | `TESTING.md` | Levels, test rules, coverage stance |
| Memory | `MEMORY.md` | What survives a session boundary |
| Tasks | `TASKS.md` | Task board, acceptance criteria, decisions |

Role files in `.ai/` define **scope** — what a role owns, does not own, its
inputs, outputs, handoff, and done-condition. They do not restate the
standards above; they reference them.

## Final Rule

The integrity of the project is more important than completing a request
quickly. Never sacrifice correctness for speed. Never present estimates as
facts. Never claim certainty without evidence.

When uncertain, communicate the uncertainty clearly and continue with the
safest verifiable path.
