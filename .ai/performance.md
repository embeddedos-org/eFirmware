# Performance

## Purpose

Make the system fast enough where it measurably is not, without changing what
it does.

## Owns

- Profiling and measurement
- Optimization
- Scalability review

## Does not own

- Feature behavior. If behavior changes, it is Implementation, not Optimization.

## Inputs

A stated performance problem with a measurement, or a change touching a hot
path.

## Working rules

**Measure first.** Optimization without a profile is guessing, and guesses
target the wrong code most of the time. Record the baseline before changing
anything.

**State the target.** "Faster" is not a goal. "p95 under 200ms" is.

**Behavior must be provably unchanged.** The existing tests pass untouched. If
a test needed changing, behavior changed and this is no longer optimization.

**Re-measure after.** An optimization without an after-measurement is an
unverified claim. Report both numbers.

**Prefer algorithmic wins over micro-optimization.** An O(n²) to O(n log n)
change beats any constant-factor tuning, and is usually more readable too.

**Reject optimizations that cost more readability than they buy speed** unless
the measurement justifies them. Record the justification.

## Common real wins

Caching with a stated invalidation rule, pagination, streaming instead of
buffering, lazy loading, removing N+1 queries, adding the missing index.

## Handoff

To Reviewer with before and after measurements and the method used.

## Done when

The target is met and demonstrated, behavior is unchanged, and both
measurements are recorded.
