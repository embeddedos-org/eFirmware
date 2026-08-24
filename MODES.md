# Modes

Work happens in one mode at a time. Switch only when the current mode's
objective is complete — not when the work feels slow.

State the current mode in every report.

| Mode | Objective | Complete when |
|---|---|---|
| **Discovery** | Understand the problem | Requirements and unknowns are named |
| **Planning** | Produce an implementation strategy | Tasks exist with acceptance criteria |
| **Research** | Gather technical information | The unknowns from Discovery are answered or declared unanswerable |
| **Architecture** | Validate the design | Structure, interfaces, and decisions are recorded |
| **Implementation** | Write code | Code compiles and the change is complete |
| **Verification** | Validate the changes | Every applicable check in VERIFY.md has a result |
| **Optimization** | Improve quality without changing behavior | Behavior is provably unchanged and the metric improved |
| **Documentation** | Update documentation | Docs match what shipped |
| **Release** | Prepare deployment | Release notes and rollback guidance exist |
| **Maintenance** | Resolve defects and technical debt | The defect is fixed and covered by a test |

## Rules

- **One mode at a time.** Writing code during Discovery is how unvalidated
  assumptions get built.
- **Do not skip Planning.** A change that looks small is exactly where skipped
  planning costs the most.
- **Optimization may not change behavior.** If behavior changes, that is
  Implementation, and it needs its own tests.
- Entering a mode without its inputs is a blocker — report it per the Failure
  Handling section of CLAUDE.md rather than proceeding on assumption.

## Transitions

```text
Discovery → Planning → Architecture → Implementation → Verification
                ↑                            ↓
             Research                   Documentation → Release
                                             ↓
                                        Maintenance
```

Backward transitions are expected and are not failures. A Verification result
that invalidates the design returns to Architecture, not to Implementation.
