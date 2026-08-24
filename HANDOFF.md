# Handoff Protocol

## When to hand off

- The task changes domain.
- Specialist knowledge is required.
- Independent review is required — an implementer cannot approve their own work.
- Context has grown too large to manage effectively.

## What a handoff costs

A handoff does **not** automatically reduce token usage. The summary costs
tokens to write and to read, and detail is lost in it. A handoff buys
headroom, not savings.

Hand off when one of the four conditions above is genuinely met — not on a
fixed cadence, and not because switching feels tidy. Frequent switching costs
more than it saves.

## Required handoff record

Every handoff carries all nine fields. A handoff missing any of them is
incomplete and the receiving role should reject it.

```markdown
## Handoff — <from-role> → <to-role>

**Current mode:** <mode from MODES.md>

**Completed work:**
- What was actually done, not what was attempted

**Files modified:**
- path/to/file — what changed and why

**Architecture decisions:**
- Decision, alternatives rejected, and the reason

**Open issues:**
- Known problems, each with its current state

**Remaining work:**
- What is left, in the order it should be done

**Known risks:**
- What could break, and what would reveal it

**Verification status:**
- Each item: PASS / FAIL / NOT RUN / UNKNOWN

**Next recommended agent:** <role> — and why
```

## Rules

- **Carry the acceptance criteria forward verbatim.** They are the contract;
  paraphrasing them loses the contract.
- **Do not restart work.** Continue from the summary. Re-deriving what the
  previous role established is the most common way handoffs waste tokens.
- **Preserve decisions and their reasons.** A decision without its reason gets
  reversed by the next role.
- **Summarize the narrative, not the contract.** Drop how the work happened;
  keep what was decided and what must still hold.
- Never mark a verification item PASS in a handoff on the strength of the
  previous role's claim. Either you ran it or it is UNKNOWN.
