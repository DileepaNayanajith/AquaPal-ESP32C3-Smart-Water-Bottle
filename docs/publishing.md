# Publishing on GitHub

No remote GitHub changes were made when this package was prepared. Push it with your own authenticated account after reviewing it.

For a **new** repository called `AquaPal-ESP32C3-Smart-Water-Bottle`:

```bash
cd AquaPal-ESP32C3-Smart-Water-Bottle
git init
git add README.md CHANGELOG.md docs hardware .gitignore
git commit -m "docs: record the original AquaPal project and hardware"
git add media
git commit -m "media: add genuine 2025 demo video and extracted frames"
git add firmware tests
git commit -m "firmware: add reconstructed ESP32-C3 reference implementation"
gh auth login     # only if GitHub CLI isn't already authenticated
gh repo create DileepaNayanajith/AquaPal-ESP32C3-Smart-Water-Bottle --public --source=. --remote=origin --push
```

If a repository already exists, **do not use `gh repo create`**. Clone it and carefully merge the prepared files instead, preserving any original sketch and historical code. Do not overwrite original firmware with the reconstructed implementation.

No commit is backdated to 2025: the project was built then, but this repository update was written in 2026.
