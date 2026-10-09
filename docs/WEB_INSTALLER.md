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

## Tests owed

Nothing owed: the full matrix (RUNBOOK 5b) passed for 1.1.0. A project copied
from this scaffold resets this list after its smoke test (RUNBOOK 5a), and
clears it before its next release or real piece of work.

- [x] Case 1 – fresh install, erased, on each board
- [x] Case 2 – Update on a provisioned board (settings kept)
- [x] Case 3 – Update from `app1` (ArduinoOTA)
- [x] Case 4 – wrong board image, then reinstall
- [ ] Case 7 – Windows Edge (optional)

---

## Hardware test matrix (Phase 5)

Run against the CI `site-preview` served on `http://localhost:8000`.

| #   | Case                               | Board / MAC | Date | Result  |
|:----|:-----------------------------------|:------------|:-----|:--------|
| 1   | Fresh install, erased – `cyd28`    | 2.8″ `B0:CB:D8:DA:AE:8C` | 09-10-2026 | Pass |
| 1   | Fresh install, erased – `cyd40`    | 4.0″ `A4:F0:0F:68:95:5C` | 09-10-2026 | Pass |
| 2   | Update on provisioned board        | 4.0″        | 09-10-2026 | Pass (after newline fix) |
| 3   | Board on `app1` (ArduinoOTA)       | 4.0″        | 09-10-2026 | Pass (after newline fix) |
| 4   | Wrong board image, then reinstall  | 2.8″        | 09-10-2026 | Pass – dark, then recovers |
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

**Case 4, 2.8″.** The `cyd40` preview parts written to the 2.8″ as the
installer would (four parts, esptool, no erase) boot without a crash. It
reports `ESP32-32E 4.0in` and keeps its NVS device name, boot counter and WiFi;
the screen stays dark (backlight GPIO 27 vs 21). Writing the `cyd28` parts back
restores it with settings and WiFi intact.

**Case 1, 4.0″.** Erased, installed from the CI preview with erase, WiFi
through **Configure WiFi**. Boot log: `v1.1.0-dev`, `Running from app0`,
ST7796S 480×320 as expected, WiFi and ArduinoOTA up, no crash.

**Why Connect offered Install – shared copy-in defect.** ESP Web Tools 10.4.0
(improv-wifi-serial-sdk 2.8.0) only parses an Improv packet that starts a line.
When Chrome opens the port the stream often begins with noise (a burst of NULs
on this CH340 board, or part of a debug line), so it discards until the next
`\n`. The vendored `lib/ImprovWiFi` writes packets with no newline, so the reply
is swallowed and Improv is "not detected" within 1.5 s. Chrome does not reset
the board on Connect. Running the SDK in Chrome against a test build that writes
`\n` before each packet: 6/6 detected from the first request (7–31 ms), and the
real dialog showed "Connected to CYD-Scaffold-CBB0". The fix belongs in
cyd-web-installer `copy-in/lib/ImprovWiFi`; it landed there in 1.0.1 and this
repo's `lib/ImprovWiFi` matches that copy.

**Case 2, 4.0″, with the patch.** A USB-flashed `1.1.0-dev.0` build on `app0`.
Connect showed "Connected to CYD-Scaffold-F0A4 · CYD_Display_Test 1.1.0-dev.0"
and offered **Update CYD_Display_Test**, with no erase question. Afterwards:
`v1.1.0-dev`, `Running from app0`, the NVS boot counter carried on (4 → 6), and
WiFi rejoined from NVS. (An accidental Update with the 2.8″ image selected was
also offered and completed on the 4.0″ – the reverse of case 4. A reinstall of
the 4.0″ image with erase recovered it.)

**Case 3, 4.0″, with the patch.** ArduinoOTA of `1.1.0-dev.0` put it on `app1`.
The first two OTA attempts timed out with the device never connecting back
(ping 112–150 ms on this unit's WiFi); the third succeeded. Connect offered
**Update**, no erase. Afterwards: `v1.1.0-dev`, `Running from app0`, boot counter
carried on (10 → 12), WiFi from NVS.

The tested release image is the CI preview from run 37846382404, which predates
the patch. The images that answered Connect were local builds with the patch.

---

## Release v1.1.0 (09-10-2026)

Tag `v1.1.0` on `main` (`0deab15`); release run 37878520628 built and
published. The live page and `index.json` show 1.1.0 for both boards; each
manifest lists four parts; the release carries both boards' `*-firmware.bin`
and `*-merged.bin` and `SHA256SUMS.txt`. The release notes open with the
erase-on-first-install note for 1.0.0 boards.

**Live Update, 2.8″ `B0:CB:D8:DA:AE:8C`.** Prepared with the CI `1.1.0-dev`
image that carries the Improv newline fix (four parts, no erase). From
https://anthonyjclarke.github.io/CYD_Display_Test/ Connect offered **Update**,
no erase question. Afterwards: `v1.1.0`, `Running from app0`, boot counter
carried on (32 → 35), WiFi from NVS. Pass.
