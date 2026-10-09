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
| 1   | Fresh install, erased – `cyd28`    | 2.8″ `B0:CB:D8:DA:AE:8C` | 09-10-2026 | Pass |
| 1   | Fresh install, erased – `cyd40`    |             |      | pending |
| 2   | Update on provisioned board        | 2.8″        | 09-10-2026 | Blocked – copy-in Improv framing |
| 3   | Board on `app1` (ArduinoOTA)       | 2.8″        | 09-10-2026 | Partial – see below |
| 4   | Wrong board image, then reinstall  |             |      | pending |
| 5   | Web `/update` with `*-firmware.bin` | –          | –    | n/a – no web UI |
| 6   | macOS Chrome                       | 2.8″        | 09-10-2026 | Pass – port found, flash done |
| 7   | Windows Edge                       |             |      | optional |

### Results 09-10-2026

**Case 1, 2.8″.** Erased, installed from the CI preview with erase, WiFi set
through **Configure WiFi** (Improv). Boot log: `CYD_Display_Test v1.1.0-dev`,
`Running from app0`, WiFi and ArduinoOTA up, display check 320×240, no crash.
Core 2.0.17 logs a harmless `addApbChangeCallback(): duplicate` line.

**Case 3, 2.8″.** ArduinoOTA of a `1.1.0-dev.0` build put it on `app1`
(`Running from app1`). The installer then wrote the preview without an erase:
it came back on `app0` at `1.1.0-dev`, the NVS boot counter carried on, and WiFi
rejoined from NVS (the CI image has no `secrets.h`). So the `app1` recovery and
settings survival pass. But Connect offered **Install**, not **Update**.

**Why Connect offered Install – shared copy-in defect.** ESP Web Tools 10.4.0
(improv-wifi-serial-sdk 2.8.0) only parses an Improv packet that starts a line.
When Chrome opens the port the stream often begins with noise (a burst of NULs
on this CH340 board, or part of a debug line), so it discards until the next
`\n`. The vendored `lib/ImprovWiFi` writes packets with no newline, so the reply
is swallowed and Improv is "not detected" within 1.5 s. Chrome does not reset
the board on Connect. Running the SDK in Chrome against a test build that writes
`\n` before each packet: 6/6 detected from the first request (7–31 ms), and the
real dialog showed "Connected to CYD-Scaffold-CBB0". The fix belongs in
cyd-web-installer `copy-in/lib/ImprovWiFi`; this repo keeps the unmodified copy
until it lands, then re-copies it and repeats cases 2 and 3.
