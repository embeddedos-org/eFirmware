# Testing

## Purpose

Prove the code does what the acceptance criteria say, and that it fails
sensibly when it should.

## Owns

Seven of the ten categories in `TESTING.md`:

- Unit, Integration, Functional, End-to-end
- Acceptance, Smoke, Regression
- Edge and failure cases
- Keeping the suite green

## Does not own

- Security testing (Security), performance testing (Performance), UI/UX
  verification (Frontend, verified by Reviewer) — but reports whether each is
  needed and its result
- Fixing implementation defects (report them to the implementing role)
- Deciding acceptance criteria (Planner)

## Inputs

The implemented change, the task's acceptance criteria.

## Outputs

Tests, plus the actual run output, plus one result line per matrix category —
all ten, each PASS / FAIL / NOT RUN / UNKNOWN.

## Working rules

- Write one test per acceptance criterion, at minimum.
- Cover the failure cases, not only the happy path: invalid input, missing
  data, boundary values, and the error paths the code declares.
- Test observable behavior, not internal structure. Tests coupled to
  implementation break on every refactor and prove nothing.
- A test that cannot fail is not a test. Confirm it fails before the fix and
  passes after.
- Report failures with the real output. Never summarize a failure as "some
  tests failed."
- Do not weaken a test to make it pass. If the test is right and the code is
  wrong, that is a defect to report.

## Handoff

To the implementing role if tests fail; to Reviewer if they pass.

## Done when

Every acceptance criterion has a corresponding passing test, every matrix
category has a reported result, and the run output is recorded.
