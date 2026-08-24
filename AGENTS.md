# Agent Responsibilities

Planner
- Understand the request.
- Break work into tasks.
- Assign work.

Architect
- Design structure.
- Choose patterns.

Backend
- APIs
- Database
- Business logic

Frontend
- UI
- Components
- Accessibility

Testing
- Unit, integration, functional, end-to-end
- Acceptance, smoke, regression
- (Security, performance, and UI/UX testing belong to those roles — see
  `TESTING.md`)

Security
- Authentication
- Validation
- Secrets

Performance
- Profiling
- Optimization
- Scalability

Reviewer
- Final review
- Verify requirements
- Merge findings

Documentation
- README
- API docs
- Changelog

Release
- Release notes
- Deployment preparation
- Rollback guidance

---

## Working instructions

Each role's instructions live in `.ai/<role>.md`. Read yours before acting.

| Role | File |
|---|---|
| Planner | `.ai/planner.md` |
| Architect | `.ai/architect.md` |
| Backend | `.ai/backend.md` |
| Frontend | `.ai/frontend.md` |
| Testing | `.ai/testing.md` |
| Security | `.ai/security.md` |
| Performance | `.ai/performance.md` |
| Documentation | `.ai/docs.md` |
| Reviewer | `.ai/reviewer.md` |
| Release | `.ai/release.md` |

**Developer** is the implementation function, split here into **Backend** and
**Frontend** because the skills and review criteria differ. Where a source
refers to a single Developer role, it maps to both.

## Standing rules

- **A role does only its own work.** Scope creep across roles defeats the
  separation that makes review meaningful.
- **No role approves its own work.** The Reviewer is a different role than the
  implementer, always.
- Handoffs follow `HANDOFF.md` and carry all nine required fields.

These assignments are this project's convention, not a claim about which
arrangement is best in general. Different models and agent frameworks have
different strengths — adjust the split when evidence from this codebase
warrants it.
