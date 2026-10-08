# Web installer – adoption and hardware tests

CYD_Display_Test adopted the shared [cyd-web-installer](https://github.com/anthonyjclarke/cyd-web-installer)
tooling in 1.1.0, following the rollout RUNBOOK. The pilot is
CYD_AnimatedPixelClock (`docs/WEB_INSTALLER_PLAN.md`).

---

## Build facts (1.1.0-dev, espressif32@6.12.0)

| Env     | Installer label             | `firmware.bin` | Slot use |
|:--------|:----------------------------|:---------------|:---------|
| `cyd28` | CYD 2.8″ – ESP32-2432S028R  | 975,952 B      | 53 %     |
| `cyd40` | CYD 4.0″ – ESP32-32E        | 972,720 B      | 53 %     |

The slot is `app0` / `app1` of `partitions_custom.csv`, 0x1C0000 (1,835,008 B).
Manifests list four parts at `0x1000 / 0x8000 / 0xe000 / 0x10000`.

---

## Hardware test matrix (Phase 5)

Run against the CI `site-preview` served on `http://localhost:8000`.

| #   | Case                               | Board / MAC | Date | Result  |
|:----|:-----------------------------------|:------------|:-----|:--------|
| 1   | Fresh install, erased – `cyd28`    |             |      | pending |
| 1   | Fresh install, erased – `cyd40`    |             |      | pending |
| 2   | Update on provisioned board        |             |      | pending |
| 3   | Board on `app1` (ArduinoOTA)       |             |      | pending |
| 4   | Wrong board image, then reinstall  |             |      | pending |
| 5   | Web `/update` with `*-firmware.bin` | –          | –    | n/a – no web UI |
| 6   | macOS Chrome                       |             |      | pending |
| 7   | Windows Edge                       |             |      | optional |
