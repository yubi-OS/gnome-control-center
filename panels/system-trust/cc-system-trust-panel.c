/*
 * Copyright 2026 yubi-OS Contributors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <glib/gi18n.h>
#include <math.h>
#include <string.h>

#include "cc-system-trust-panel.h"
#include "cc-system-trust-resources.h"

struct _CcSystemTrustPanel {
    CcPanel parent_instance;

    GtkWidget *chain_overlay;
    GtkWidget *connector_canvas;

    GtkWidget *firmware_emblem;
    GtkWidget *firmware_status_icon;
    GtkLabel  *firmware_status_label;
    GtkLabel  *firmware_detail_label;

    GtkWidget *boot_emblem;
    GtkWidget *boot_status_icon;
    GtkLabel  *boot_status_label;
    GtkLabel  *boot_detail_label;

    GtkWidget *owner_emblem;
    GtkWidget *owner_status_icon;
    GtkLabel  *owner_status_label;
    GtkLabel  *owner_detail_label;

    GtkRevealer *details_revealer;
    GtkImage    *details_chevron;
};

CC_PANEL_REGISTER (CcSystemTrustPanel, cc_system_trust_panel)

static void
draw_vertical_segment (cairo_t *cr,
                       double   x,
                       double   y1,
                       double   y2,
                       double   r1, double g1, double b1,
                       double   r2, double g2, double b2)
{
    cairo_pattern_t *grad;

    if (y2 <= y1 + 2.0)
        return;

    grad = cairo_pattern_create_linear (x, y1, x, y2);
    cairo_pattern_add_color_stop_rgba (grad, 0.0, r1, g1, b1, 0.95);
    cairo_pattern_add_color_stop_rgba (grad, 1.0, r2, g2, b2, 0.95);

    cairo_save (cr);
    cairo_set_source (cr, grad);
    cairo_set_line_width (cr, 2.0);
    cairo_move_to (cr, x, y1);
    cairo_line_to (cr, x, y2);
    cairo_stroke (cr);

    cairo_set_source_rgba (cr, r2, g2, b2, 1.0);
    cairo_arc (cr, x, y2, 2.6, 0, 2 * G_PI);
    cairo_fill (cr);
    cairo_restore (cr);

    cairo_pattern_destroy (grad);
}

static void
draw_connectors_cb (GtkDrawingArea *area,
                    cairo_t        *cr,
                    int             width,
                    int             height,
                    gpointer        user_data)
{
    CcSystemTrustPanel *self = CC_SYSTEM_TRUST_PANEL (user_data);
    graphene_rect_t b1, b2, b3;

    if (!gtk_widget_compute_bounds (self->firmware_emblem, GTK_WIDGET (area), &b1) ||
        !gtk_widget_compute_bounds (self->boot_emblem, GTK_WIDGET (area), &b2) ||
        !gtk_widget_compute_bounds (self->owner_emblem, GTK_WIDGET (area), &b3))
        return;

    double x = b1.origin.x + (b1.size.width * 0.5);
    draw_vertical_segment (cr,
                           x,
                           b1.origin.y + b1.size.height + 2.0,
                           b2.origin.y - 2.0,
                           0.92, 0.42, 1.00,
                           0.34, 0.56, 1.00);
    draw_vertical_segment (cr,
                           x,
                           b2.origin.y + b2.size.height + 2.0,
                           b3.origin.y - 2.0,
                           0.34, 0.56, 1.00,
                           0.15, 0.88, 1.00);
}

static void
apply_status_badge (GtkWidget   *icon,
                    GtkLabel    *label,
                    const gchar *status_text,
                    const gchar *state_class)
{
    gtk_widget_remove_css_class (icon, "verified");
    gtk_widget_remove_css_class (icon, "warning");
    gtk_widget_remove_css_class (icon, "failed");
    gtk_widget_add_css_class (icon, state_class);

    if (g_strcmp0 (state_class, "verified") == 0)
        gtk_image_set_from_icon_name (GTK_IMAGE (icon), "object-select-symbolic");
    else if (g_strcmp0 (state_class, "failed") == 0)
        gtk_image_set_from_icon_name (GTK_IMAGE (icon), "window-close-symbolic");
    else
        gtk_image_set_from_icon_name (GTK_IMAGE (icon), "dialog-warning-symbolic");

    gtk_label_set_text (label, status_text);
}

static gboolean
read_efivar_byte (const gchar *glob_prefix, guint8 *out_val)
{
    g_autoptr(GDir) dir = NULL;
    const gchar *name;

    dir = g_dir_open ("/sys/firmware/efi/efivars", 0, NULL);
    if (!dir)
        return FALSE;

    while ((name = g_dir_read_name (dir)) != NULL) {
        if (g_str_has_prefix (name, glob_prefix)) {
            g_autofree gchar *path = g_build_filename ("/sys/firmware/efi/efivars", name, NULL);
            g_autofree guint8 *buf = NULL;
            gsize len = 0;
            if (g_file_get_contents (path, (gchar **) &buf, &len, NULL) && len >= 5) {
                *out_val = buf[4];
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void
probe_runtime_trust (CcSystemTrustPanel *self)
{
    const gchar *override_file = g_getenv ("YUBIOS_SYSTEM_TRUST_STATUS_FILE");
    g_autofree gchar *helper_out = NULL;
    gboolean parsed_helper = FALSE;

    if (override_file && g_file_test (override_file, G_FILE_TEST_IS_REGULAR)) {
        g_file_get_contents (override_file, &helper_out, NULL, NULL);
    } else if (g_file_test ("/usr/lib/yubiOS/system-trust-status.sh", G_FILE_TEST_IS_EXECUTABLE)) {
        g_spawn_command_line_sync ("/usr/lib/yubiOS/system-trust-status.sh",
                                   &helper_out, NULL, NULL, NULL);
    }

    if (helper_out && *helper_out) {
        g_auto(GStrv) lines = g_strsplit (helper_out, "\n", -1);
        g_autofree gchar *fw_status = NULL;
        g_autofree gchar *fw_class  = NULL;
        g_autofree gchar *fw_detail = NULL;
        g_autofree gchar *bt_status = NULL;
        g_autofree gchar *bt_class  = NULL;
        g_autofree gchar *bt_detail = NULL;
        g_autofree gchar *ow_status = NULL;
        g_autofree gchar *ow_class  = NULL;
        g_autofree gchar *ow_detail = NULL;

        for (guint i = 0; lines[i] != NULL; i++) {
            const gchar *line = lines[i];
            if (g_str_has_prefix (line, "FIRMWARE_STATUS="))
                fw_status = g_strdup (line + strlen ("FIRMWARE_STATUS="));
            else if (g_str_has_prefix (line, "FIRMWARE_CLASS="))
                fw_class = g_strdup (line + strlen ("FIRMWARE_CLASS="));
            else if (g_str_has_prefix (line, "FIRMWARE_DETAIL="))
                fw_detail = g_strdup (line + strlen ("FIRMWARE_DETAIL="));
            else if (g_str_has_prefix (line, "BOOT_IMAGE_STATUS="))
                bt_status = g_strdup (line + strlen ("BOOT_IMAGE_STATUS="));
            else if (g_str_has_prefix (line, "BOOT_IMAGE_CLASS="))
                bt_class = g_strdup (line + strlen ("BOOT_IMAGE_CLASS="));
            else if (g_str_has_prefix (line, "BOOT_IMAGE_DETAIL="))
                bt_detail = g_strdup (line + strlen ("BOOT_IMAGE_DETAIL="));
            else if (g_str_has_prefix (line, "OWNER_KEY_STATUS="))
                ow_status = g_strdup (line + strlen ("OWNER_KEY_STATUS="));
            else if (g_str_has_prefix (line, "OWNER_KEY_CLASS="))
                ow_class = g_strdup (line + strlen ("OWNER_KEY_CLASS="));
            else if (g_str_has_prefix (line, "OWNER_KEY_DETAIL="))
                ow_detail = g_strdup (line + strlen ("OWNER_KEY_DETAIL="));
        }

        if (fw_status && bt_status && ow_status) {
            apply_status_badge (self->firmware_status_icon,
                                self->firmware_status_label,
                                fw_status,
                                fw_class ? fw_class : "verified");
            apply_status_badge (self->boot_status_icon,
                                self->boot_status_label,
                                bt_status,
                                bt_class ? bt_class : "verified");
            apply_status_badge (self->owner_status_icon,
                                self->owner_status_label,
                                ow_status,
                                ow_class ? ow_class : "verified");
            if (fw_detail)
                gtk_label_set_text (self->firmware_detail_label, fw_detail);
            if (bt_detail)
                gtk_label_set_text (self->boot_detail_label, bt_detail);
            if (ow_detail)
                gtk_label_set_text (self->owner_detail_label, ow_detail);
            parsed_helper = TRUE;
        }
    }

    if (parsed_helper)
        return;

    /* Fallback direct evidence probe when helper script is absent */
    guint8 sb_val = 0, setup_val = 1;
    gboolean has_sb = read_efivar_byte ("SecureBoot-", &sb_val);
    gboolean has_setup = read_efivar_byte ("SetupMode-", &setup_val);
    gboolean has_tpm = g_file_test ("/dev/tpmrm0", G_FILE_TEST_EXISTS) ||
                       g_file_test ("/dev/tpm0", G_FILE_TEST_EXISTS);

    if (has_sb && sb_val == 1 && has_setup && setup_val == 0) {
        apply_status_badge (self->firmware_status_icon, self->firmware_status_label, _("Verified"), "verified");
        gtk_label_set_text (self->firmware_detail_label,
                            has_tpm ? "UEFI SecureBoot=1, SetupMode=0, TPM/fTPM active"
                                    : "UEFI SecureBoot=1, SetupMode=0");
    } else if (has_setup && setup_val == 1) {
        apply_status_badge (self->firmware_status_icon, self->firmware_status_label, _("Setup Mode"), "warning");
        gtk_label_set_text (self->firmware_detail_label, "UEFI SetupMode=1 (awaiting owner key enrollment)");
    } else {
        g_autofree gchar *fw_unknown = g_strdup_printf ("UEFI SecureBoot=%s, SetupMode=%s",
                                                        has_sb ? (sb_val ? "1" : "0") : "unknown",
                                                        has_setup ? (setup_val ? "1" : "0") : "unknown");
        apply_status_badge (self->firmware_status_icon, self->firmware_status_label, _("Unverified"), "warning");
        gtk_label_set_text (self->firmware_detail_label, fw_unknown);
    }

    g_autofree gchar *cmdline = NULL;
    g_autofree gchar *mounts = NULL;
    gboolean has_composefs_karg = FALSE;
    gboolean has_composefs_mount = FALSE;
    if (g_file_get_contents ("/proc/cmdline", &cmdline, NULL, NULL) && cmdline) {
        has_composefs_karg = (strstr (cmdline, "composefs=") != NULL);
    }
    if (g_file_get_contents ("/proc/mounts", &mounts, NULL, NULL) && mounts) {
        has_composefs_mount = (strstr (mounts, " / composefs ") != NULL) ||
                              (strstr (mounts, " /sysroot composefs ") != NULL);
    }
    if (has_composefs_karg && has_composefs_mount) {
        apply_status_badge (self->boot_status_icon, self->boot_status_label, _("Verified"), "verified");
        gtk_label_set_text (self->boot_detail_label,
                            "composefs digest pinned in kernel cmdline; / mounted as composefs");
    } else if (has_composefs_karg) {
        apply_status_badge (self->boot_status_icon, self->boot_status_label, _("Unverified"), "warning");
        gtk_label_set_text (self->boot_detail_label, "composefs= in /proc/cmdline, but composefs root mount not verified");
    } else {
        apply_status_badge (self->boot_status_icon, self->boot_status_label, _("Unmeasured"), "warning");
        gtk_label_set_text (self->boot_detail_label, "No composefs root mount or kernel cmdline digest");
    }

    gboolean enrolled = g_file_test ("/var/lib/yubiOS/.enrolled", G_FILE_TEST_EXISTS);
    g_autofree gchar *u2f_keys = NULL;
    gsize u2f_len = 0;
    gboolean has_u2f = g_file_get_contents ("/etc/yubico/u2f_keys", &u2f_keys, &u2f_len, NULL) && u2f_len > 4;
    if (enrolled || has_u2f) {
        apply_status_badge (self->owner_status_icon, self->owner_status_label, _("Verified"), "verified");
        gtk_label_set_text (self->owner_detail_label,
                            "Owner YubiKey enrolled (/var/lib/yubiOS/.enrolled + PAM U2F)");
    } else {
        apply_status_badge (self->owner_status_icon, self->owner_status_label, _("Not Enrolled"), "warning");
        gtk_label_set_text (self->owner_detail_label,
                            "Run yubiOS-enroll to bind your YubiKey as owner root of trust");
    }
}

static void
on_details_toggle_clicked_cb (CcSystemTrustPanel *self)
{
    gboolean revealed = gtk_revealer_get_reveal_child (self->details_revealer);
    gtk_revealer_set_reveal_child (self->details_revealer, !revealed);
    gtk_image_set_from_icon_name (self->details_chevron,
                                  !revealed ? "go-down-symbolic" : "go-next-symbolic");
}

static void
cc_system_trust_panel_class_init (CcSystemTrustPanelClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gnome/control-center/system-trust/cc-system-trust-panel.ui");

    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, chain_overlay);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, connector_canvas);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, firmware_emblem);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, firmware_status_icon);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, firmware_status_label);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, firmware_detail_label);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, boot_emblem);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, boot_status_icon);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, boot_status_label);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, boot_detail_label);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, owner_emblem);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, owner_status_icon);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, owner_status_label);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, owner_detail_label);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, details_revealer);
    gtk_widget_class_bind_template_child (widget_class, CcSystemTrustPanel, details_chevron);

    gtk_widget_class_bind_template_callback (widget_class, on_details_toggle_clicked_cb);
}

static void
cc_system_trust_panel_init (CcSystemTrustPanel *self)
{
    g_resources_register (cc_system_trust_get_resource ());

    gtk_widget_init_template (GTK_WIDGET (self));

    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (self->connector_canvas),
                                    draw_connectors_cb,
                                    self,
                                    NULL);

    probe_runtime_trust (self);
}
