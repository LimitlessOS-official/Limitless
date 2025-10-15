#!/bin/bash

##############################################################################
# LimitlessOS Silent Installation Script
# Fully automated, unattended installation for enterprise deployments
##############################################################################

set -e

# Configuration
SILENT_CONFIG_FILE="/tmp/limitless_silent_config.json"
LOG_FILE="/var/log/limitless_silent_install.log"
INSTALLER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default configuration for silent installation
cat > "$SILENT_CONFIG_FILE" << 'EOF'
{
  "deployment": {
    "mode": "silent",
    "automation_level": "fully_automated",
    "validation_level": "standard",
    "skip_confirmations": true
  },
  "security": {
    "encryption": true,
    "encryption_passphrase": "LimitlessEnterprise2024!",
    "secure_boot": true,
    "firewall": true,
    "audit_logging": true
  },
  "system": {
    "hostname": "limitless-silent",
    "timezone": "UTC",
    "locale": "en_US.UTF-8",
    "auto_updates": true
  },
  "user": {
    "username": "admin",
    "password": "TempPassword123!",
    "fullname": "System Administrator",
    "create_home": true,
    "sudo_access": true,
    "force_password_change": true
  },
  "network": {
    "configure_automatically": true,
    "dns_servers": ["1.1.1.1", "1.0.0.1"],
    "enable_ssh": false
  },
  "features": {
    "development_tools": false,
    "enterprise_tools": true,
    "ai_framework": false,
    "multimedia_codecs": false,
    "virtualization": true
  },
  "partitioning": {
    "auto_partition": true,
    "filesystem": "ext4",
    "swap_size_gb": 4,
    "root_size_percent": 70,
    "use_largest_disk": true
  },
  "validation": {
    "skip_hardware_validation": false,
    "continue_on_warnings": true,
    "require_minimum_ram_gb": 4,
    "require_minimum_disk_gb": 50
  }
}
EOF

# Logging function
log() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') [SILENT] $*" | tee -a "$LOG_FILE"
}

log "LimitlessOS Silent Installation Starting..."
log "Configuration: $SILENT_CONFIG_FILE"

# Set environment variables for silent installation
export DEBIAN_FRONTEND=noninteractive
export NEEDRESTART_MODE=a

# Execute main installer with silent configuration
exec "$INSTALLER_DIR/install_limitless_enterprise.sh" \
    --config "$SILENT_CONFIG_FILE" \
    --mode silent \
    --force \
    --debug