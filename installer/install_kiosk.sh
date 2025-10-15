#!/bin/bash

##############################################################################
# LimitlessOS Kiosk Installation Script
# Optimized for public kiosk deployments with locked-down security
##############################################################################

set -e

INSTALLER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KIOSK_CONFIG_FILE="/tmp/limitless_kiosk_config.json"

# Kiosk configuration
cat > "$KIOSK_CONFIG_FILE" << 'EOF'
{
  "deployment": {
    "mode": "kiosk",
    "automation_level": "fully_automated",
    "validation_level": "standard"
  },
  "security": {
    "encryption": true,
    "secure_boot": true,
    "firewall": true,
    "audit_logging": true,
    "kiosk_mode": true,
    "disable_usb": true,
    "disable_removable_media": true,
    "read_only_filesystem": true
  },
  "system": {
    "hostname": "limitless-kiosk",
    "timezone": "UTC",
    "locale": "en_US.UTF-8",
    "auto_login": true,
    "disable_shutdown": true,
    "disable_virtual_terminals": true,
    "watchdog_enabled": true
  },
  "user": {
    "username": "kiosk",
    "fullname": "Kiosk User",
    "create_home": false,
    "sudo_access": false,
    "shell": "/bin/false",
    "locked_account": true
  },
  "network": {
    "configure_automatically": true,
    "dns_servers": ["1.1.1.1", "1.0.0.1"],
    "enable_ssh": false,
    "disable_wireless": false,
    "content_filtering": true
  },
  "features": {
    "development_tools": false,
    "enterprise_tools": false,
    "ai_framework": false,
    "multimedia_codecs": true,
    "web_browser_only": true,
    "touch_screen_support": true,
    "accessibility_features": true
  },
  "kiosk_settings": {
    "browser": "chromium",
    "homepage": "https://limitlessos.org/kiosk",
    "fullscreen_mode": true,
    "disable_navigation": true,
    "disable_downloads": true,
    "disable_printing": false,
    "session_timeout": 300,
    "reset_on_idle": true,
    "allowed_domains": ["limitlessos.org", "example.com"],
    "block_social_media": true
  },
  "partitioning": {
    "auto_partition": true,
    "filesystem": "ext4",
    "swap_size_gb": 1,
    "root_size_percent": 90,
    "overlay_filesystem": true
  },
  "hardware": {
    "disable_bluetooth": true,
    "disable_camera": false,
    "disable_microphone": true,
    "disable_speakers": false,
    "power_management": "kiosk"
  }
}
EOF

echo "=============================================================================="
echo "                         LimitlessOS Kiosk Installer"
echo "=============================================================================="
echo ""
echo "This will install LimitlessOS optimized for kiosk deployment with:"
echo "• Locked-down security (read-only filesystem)"
echo "• Single-application mode (web browser)"
echo "• Automatic session reset"
echo "• Touch screen support"
echo "• Content filtering and domain restrictions"
echo "• Tamper-resistant configuration"
echo "• Accessibility features"
echo ""
echo "Security features:"
echo "• Encrypted storage"
echo "• Disabled USB ports and removable media"
echo "• Firewall with strict rules"
echo "• No administrative access for kiosk user"
echo "• Session timeout and automatic reset"
echo ""
echo "Kiosk configuration:"
echo "• Full-screen web browser"
echo "• Navigation controls disabled"
echo "• Downloads blocked"
echo "• Social media sites blocked"
echo "• Automatic system recovery"
echo ""

# Collect kiosk configuration
echo ""
echo "Kiosk Configuration:"
echo "===================="
echo -n "Enter kiosk homepage URL (default: https://limitlessos.org/kiosk): "
read -r HOMEPAGE_URL
HOMEPAGE_URL=${HOMEPAGE_URL:-https://limitlessos.org/kiosk}

echo -n "Enter session timeout in minutes (default: 5): "
read -r SESSION_TIMEOUT
SESSION_TIMEOUT=${SESSION_TIMEOUT:-5}

echo -n "Allow printing? [y/N]: "
read -r ALLOW_PRINTING
if [[ "$ALLOW_PRINTING" =~ ^[Yy]$ ]]; then
    ALLOW_PRINTING="true"
else
    ALLOW_PRINTING="false"
fi

echo -n "Enable touch screen support? [Y/n]: "
read -r TOUCH_SUPPORT
if [[ "$TOUCH_SUPPORT" =~ ^[Nn]$ ]]; then
    TOUCH_SUPPORT="false"
else
    TOUCH_SUPPORT="true"
fi

echo ""
echo "Domain Whitelist Configuration:"
echo "==============================="
echo "Enter allowed domains (one per line, press Enter on empty line to finish):"
ALLOWED_DOMAINS=()
while true; do
    echo -n "Domain: "
    read -r domain
    if [[ -z "$domain" ]]; then
        break
    fi
    ALLOWED_DOMAINS+=("$domain")
done

# Update configuration with kiosk settings
jq --arg homepage "$HOMEPAGE_URL" \
   --arg timeout "$SESSION_TIMEOUT" \
   --arg printing "$ALLOW_PRINTING" \
   --arg touch "$TOUCH_SUPPORT" \
   --argjson domains "$(printf '%s\n' "${ALLOWED_DOMAINS[@]}" | jq -R . | jq -s .)" \
   '.kiosk_settings.homepage = $homepage | 
    .kiosk_settings.session_timeout = ($timeout | tonumber) |
    .kiosk_settings.disable_printing = ($printing == "false") |
    .features.touch_screen_support = ($touch == "true") |
    .kiosk_settings.allowed_domains = $domains' \
   "$KIOSK_CONFIG_FILE" > "${KIOSK_CONFIG_FILE}.tmp" && mv "${KIOSK_CONFIG_FILE}.tmp" "$KIOSK_CONFIG_FILE"

echo ""
echo "Final kiosk configuration:"
echo "• Homepage: $HOMEPAGE_URL"
echo "• Session timeout: $SESSION_TIMEOUT minutes"
echo "• Printing: $ALLOW_PRINTING"
echo "• Touch screen: $TOUCH_SUPPORT"
echo "• Allowed domains: ${ALLOWED_DOMAINS[*]}"
echo ""
echo "Press Enter to begin installation or Ctrl+C to abort..."
read -r

# Execute main installer with kiosk configuration
exec "$INSTALLER_DIR/install_limitless_enterprise.sh" \
    --config "$KIOSK_CONFIG_FILE" \
    --mode kiosk \
    --force