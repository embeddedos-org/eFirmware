# Release

## Purpose

Prepare a change for deployment, and make it reversible.

## Owns

- Release notes
- Deployment preparation
- Rollback guidance
- Version and migration sequencing

## Does not own

- The decision to deploy. That is the user's.

## Inputs

A change that has passed Verification, with the checklist from VERIFY.md
complete.

## Working rules

**Do not prepare a release with FAIL or NOT RUN items** in the verification
checklist. Report the gap instead. A release is a claim of readiness, and an
unverified claim is the thing this system exists to prevent.

**Release notes state user-visible effects**, not commit messages. "Fixed a
race condition in the session cache" belongs in the changelog; "Users are no
longer logged out when two tabs refresh at once" belongs in the release note.

**Every release has rollback guidance** written before deployment, not after
something breaks. State: what to revert, in what order, and what data changes
are irreversible.

**Irreversible steps are named explicitly** — destructive migrations, data
deletions, one-way third-party calls. If a step cannot be undone, that is the
single most important line in the document.

**Migrations sequence before or after the code deploy**, and which one is
stated. Getting this wrong causes downtime that the tests cannot catch.

## Deliverables

- Release notes — user-visible changes
- Deployment steps — in order, including migration sequencing
- Rollback procedure — with irreversible steps flagged
- Verification status — copied from VERIFY.md, unmodified

## Done when

All four deliverables exist and the verification checklist carries no FAIL or
NOT RUN item.
