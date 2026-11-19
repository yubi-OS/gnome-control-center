# Releasing `yubi-OS/gnome-control-center` for yubiOS

`yubi-OS/gnome-control-center` follows the org fork standardization rules (`docs/FORKS.md` in `yubi-OS/yubiOS`):

1. `main` is synced 1:1 with upstream `GNOME/gnome-control-center:main` (`0 ahead, 0 behind`).
2. `yubios` is the canonical integration branch carrying:
   - Vendored `subprojects/gvc` (`d2442f455844`) and `subprojects/libgxdp` (`df896e3412b7`) for hermetic offline Meson builds.
   - `panels/system-trust/`: the yubiOS **System Trust** panel (`CcSystemTrustPanel`) displaying the three-layer hardware-to-owner chain (**Firmware**, **Boot image**, **Owner key**) and evidence-bounded **Details** drawer.
   - `shell/`: sidebar placement right below **Displays** and the `yubiOS` header emblem.
   - `packaging/gnome-control-center-yubios.spec` + `packaging/build-rpm.sh`: Fedora 45 RPM packaging (`51.1-2.yubios.fc45`).
3. Multi-arch RPMs (`x86_64` + `aarch64`) are built in CI via `.github/workflows/yubios-release.yml` (and verified on every `yubi-OS/yubiOS` fork CI run via `ci_fork_gnome-control-center.yml`) inside the digest-pinned `quay.io/fedora/fedora-bootc:45` image.
