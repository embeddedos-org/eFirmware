# Quality Standard

Code is read far more often than it is written. Optimize for the reader.

## Properties

Code should be readable, maintainable, modular, secure, testable, documented,
and consistent with the existing architecture.

Where these conflict, readability and correctness win.

## Rules

**Match the surrounding code.** Naming, structure, comment density, and idiom.
A file written in a different style than its neighbours costs every future
reader, whatever its individual merit.

**Prefer reuse over replacement.** Check whether it already exists before
writing it. Duplication is cheaper to create and far more expensive to own.

**Keep changes minimal and focused.** A bug fix does not need surrounding
cleanup. Unrelated improvements belong in their own task where they can be
reviewed on their own merit.

**Do not design for hypothetical requirements.** Build what the acceptance
criteria state. Abstraction added for a future that never arrives is pure cost.

**Do not add defensive code for conditions that cannot occur.** Validate at
system boundaries; trust internal invariants and framework guarantees.

**Comments state constraints the code cannot show** — why a non-obvious choice
was made, what invariant must hold. Not what the next line does, not where the
code came from, not that the change is correct.

**Complete every implementation.** No placeholders, stubs, or TODOs unless the
user explicitly asked for them.

## Project protection

Never, without explicit justification recorded in `TASKS.md`:

- Delete code without understanding its impact
- Rewrite an entire system when a targeted change suffices
- Change architecture
- Introduce a breaking API change silently
- Ignore a compiler error, failing test, security warning, accessibility
  issue, or performance regression

Prefer the minimal safe change. When two approaches are equally correct, take
the one that is easier to reverse.

## Failure handling

If blocked: **stop**. Explain why, identify what information is missing, and
recommend next actions.

Do not invent solutions to missing information. A plausible guess presented as
a decision is more damaging than an admitted gap, because it is acted upon.
