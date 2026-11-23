#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Build gnome-control-center 51.2.1-2.yubios RPMs inside a Fedora 45 environment.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="51.2.1"
RPM_TOPDIR="${RPM_TOPDIR:-/tmp/rpmbuild-gnome-control-center}"

rm -rf "${RPM_TOPDIR}"
mkdir -p "${RPM_TOPDIR}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

STAGE_DIR="$(mktemp -d)"
trap 'rm -rf "${STAGE_DIR}"' EXIT

mkdir -p "${STAGE_DIR}/gnome-control-center-${VERSION}"
tar -C "${REPO_ROOT}" \
  --exclude=.git \
  --exclude=_build \
  -cf - . | tar -C "${STAGE_DIR}/gnome-control-center-${VERSION}" -xf -

tar -C "${STAGE_DIR}" -czf "${RPM_TOPDIR}/SOURCES/gnome-control-center-${VERSION}.tar.gz" \
  "gnome-control-center-${VERSION}"

cp "${REPO_ROOT}/packaging/gnome-control-center-yubios.spec" "${RPM_TOPDIR}/SPECS/"

rpmbuild -bb \
  --define "_topdir ${RPM_TOPDIR}" \
  --define "_DisableDebuginfo 1" \
  --without debuginfo \
  "${RPM_TOPDIR}/SPECS/gnome-control-center-yubios.spec"

mkdir -p "${REPO_ROOT}/dist-rpms"
find "${RPM_TOPDIR}/RPMS" -name '*.rpm' ! -name '*debug*' -exec cp -v {} "${REPO_ROOT}/dist-rpms/" \;
sha256sum "${REPO_ROOT}/dist-rpms/"*.rpm
sha512sum "${REPO_ROOT}/dist-rpms/"*.rpm
