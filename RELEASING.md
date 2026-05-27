# Release Process

## Version Convention

- During development, `pyproject.toml` has version `X.Y.Z.dev0`
- At release time, the `.dev0` suffix is stripped
- After release, version is bumped to the next `X.Y.Z.dev0`

## Release Checklist

1. **Prepare the release PR:**
   - Update `version` in `pyproject.toml`: remove `.dev0` suffix (e.g., `0.12.0.dev0` → `0.12.0`)
   - Update CHANGELOG.md with release notes
   - Merge PR to `main`

2. **Create the release:**
   - Create a git tag: `git tag v0.12.0`
   - Push the tag: `git push origin v0.12.0`
   - Create a GitHub Release from the tag at https://github.com/IntentDev/touchpy-dev/releases/new
   - This triggers the `wheels.yml` workflow which:
     - Builds wheels for Python 3.9-3.13 (Windows x64)
     - Publishes to PyPI via trusted publishing
     - Uploads wheels as GitHub Release assets
   - This also triggers the `docs.yml` workflow which rebuilds and deploys documentation

3. **Post-release:**
   - Create a follow-up PR bumping version to next `.dev0` (e.g., `0.13.0.dev0`)
   - Merge to `main`

## PyPI Trusted Publishing Setup (one-time)

Configure at https://pypi.org under your account → Publishing:

- **Owner**: `IntentDev`
- **Repository**: `touchpy-dev` (update after repo rename to `touchpy`)
- **Workflow name**: `wheels.yml`
- **Environment**: (leave blank)

## Manual Workflow Triggers

All workflows can be triggered manually from the GitHub Actions tab:

- **wheels.yml**: Build wheels without publishing (manual dispatch skips PyPI upload)
- **docs.yml**: Rebuild and deploy documentation
- **ci.yml**: Runs automatically on push/PR
