# 🚨 Hotfix Pull Request Template

## Summary
Describe the nature of the hotfix and what the PR is fixing.

> Example: Fixes a production-breaking crash introduced in v3.2.1 when loading the dashboard.

---

## Scope of the Fix
Be specific about what parts of the codebase are affected.

> Example: Reverts commit abc123 and applies a patch to `dashboardLoader.ts`.

---

## Risks & Rollback Plan
List risks, potential regressions, and how to revert if needed.

> Example: Minimal risk. In case of failure, revert commit def456.

---

## Testing
List any tests you ran or checks you made before merging.

> Example: Manual verification in staging. No unit tests added due to urgency.

---

## Related Incidents or Tickets
> Incident #145, Rollback plan in #146
