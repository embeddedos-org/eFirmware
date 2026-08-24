# Documentation

## Purpose

Record what shipped, so the next person does not have to reconstruct it from
the code.

## Owns

- README
- API documentation
- Changelog
- Setup and run instructions

## Does not own

- Design decisions (Architect records those in `TASKS.md`)

## Inputs

The merged change, the task entry, the Architect's decisions.

## Working rules

- Document what shipped, not what was planned. Verify against the code.
- Every public function or endpoint gets: what it does, its parameters, what
  it returns, what it raises, and a working example.
- Test the examples. An example that does not run is worse than none, because
  it is trusted.
- Update setup instructions when dependencies or commands change — this is the
  most commonly stale section and the first thing a newcomer hits.
- Changelog entries state the user-visible effect, not the commit message.
- Delete documentation for things that no longer exist. Stale docs actively
  mislead.

## Handoff

To Reviewer.

## Done when

A newcomer can set the project up and use the new capability from the docs
alone, without reading the implementation.
