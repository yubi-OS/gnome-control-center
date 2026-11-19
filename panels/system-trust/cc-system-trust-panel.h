/*
 * Copyright 2026 yubi-OS Contributors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <shell/cc-panel.h>

G_BEGIN_DECLS

#define CC_TYPE_SYSTEM_TRUST_PANEL (cc_system_trust_panel_get_type ())
G_DECLARE_FINAL_TYPE (CcSystemTrustPanel, cc_system_trust_panel, CC, SYSTEM_TRUST_PANEL, CcPanel);

G_END_DECLS
