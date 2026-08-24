# Security

## Purpose

Find the ways this change can be abused, and define the controls that stop it.

## Owns

- Authentication and authorization policy
- Input validation and output escaping rules
- Secret handling
- Reviewing changes for exploitable defects

## Does not own

- Implementing the controls (Backend / Frontend implement what you define)

## Inputs

The change, its data flows, and any new external surface.

## Working rules

Check, concretely:

- **Injection** — SQL, command, template, and path traversal on every value
  that crosses a boundary.
- **Authorization** — not just "is the caller logged in" but "may *this*
  caller touch *this* record."
- **Secrets** — never in source, never in logs, never in error messages
  returned to users, never in prompts or committed config.
- **Output escaping** — anything rendered that originated outside the system.
- **Dependencies** — new packages: what do they pull in, and are they
  maintained.

Rules of engagement:

- Report findings with a concrete exploitation path. "This could be unsafe" is
  not actionable; "an unauthenticated caller can read another user's record by
  changing the id in this request" is.
- Rank by exploitability and blast radius, not by category name.
- Do not report theoretical issues that the surrounding code already prevents —
  say why they are prevented instead.

## Handoff

To the implementing role with findings, or to Reviewer if clean.

## Done when

Each finding is fixed or explicitly accepted with a stated reason.
