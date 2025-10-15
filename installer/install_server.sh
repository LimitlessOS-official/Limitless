#!/bin/bash

##############################################################################
# LimitlessOS Server Installation Script  
# Optimized for server deployments with minimal GUI and maximum performance
##############################################################################

set -e

INSTALLER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_CONFIG_FILE="/tmp/limitless_server_config.json"

# Server configuration
cat > "$SERVER_CONFIG_FILE" << 'EOF'
{
  "deployment": {
    "mode": "server",
    "automation_level": "semi_automated",
    "validation_level": "strict"
  },
  "security": {
    "encryption": true,
    "secure_boot": true,
    "firewall": true,
    "audit_logging": true,
    "hardened_kernel": true,
    "disable_usb": true,
    "fail2ban": true
  },
  "system": {
    "hostname": "limitless-server",
    "timezone": "UTC",
    "locale": "en_US.UTF-8",
    "performance_profile": "server",
    "disable_gui": true,
    "minimal_install": true
  },
  "user": {
    "username": "sysadmin",
    "fullname": "System Administrator",
    "create_home": true,
    "sudo_access": true,
    "shell": "/bin/bash",
    "disable_root_login": true
  },
  "network": {
    "configure_automatically": false,
    "static_ip_required": true,
    "dns_servers": ["1.1.1.1", "1.0.0.1"],
    "enable_ssh": true,
    "ssh_port": 2222,
    "ssh_key_auth_only": true
  },
  "features": {
    "development_tools": false,
    "enterprise_tools": true,
    "ai_framework": false,
    "multimedia_codecs": false,
    "virtualization": true,
    "containers": true,
    "web_server": true,
    "database_server": true,
    "monitoring": true
  },
  "server_roles": {
    "web_server": true,
    "database_server": true,
    "application_server": true,
    "monitoring_server": true,
    "backup_server": false,
    "dns_server": false,
    "mail_server": false
  },
  "partitioning": {
    "auto_partition": true,
    "filesystem": "ext4",
    "swap_size_gb": 2,
    "root_size_percent": 30,
    "var_size_percent": 40,
    "home_size_percent": 20,
    "tmp_size_percent": 10,
    "create_separate_partitions": true
  },
  "optimization": {
    "disable_unnecessary_services": true,
    "kernel_optimization": "server",
    "io_scheduler": "deadline",
    "swappiness": 10,
    "transparent_hugepages": "never"
  }
}
EOF

echo "=============================================================================="
echo "                        LimitlessOS Server Installer"
echo "=============================================================================="
echo ""
echo "This will install LimitlessOS optimized for server deployment with:"
echo "• Minimal system footprint (no GUI)"
echo "• Enhanced security hardening"
echo "• Server-optimized kernel and performance settings"
echo "• Web server and database capabilities"
echo "• Advanced monitoring and logging"
echo "• Container and virtualization support"
echo "• Strict firewall and access controls"
echo ""
echo "Server roles to be installed:"
echo "• Web Server (Nginx/Apache)"
echo "• Database Server (PostgreSQL/MySQL)"
echo "• Application Server"  
echo "• Monitoring Server"
echo ""
echo "Security features:"
echo "• Encrypted root filesystem"
echo "• SSH key-based authentication only"
echo "• Fail2ban intrusion prevention"
echo "• Comprehensive audit logging"
echo "• Hardened kernel configuration"
echo ""
echo "Press Enter to continue or Ctrl+C to abort..."
read -r

# Collect network configuration for server
echo ""
echo "Server Network Configuration:"
echo "============================="
echo -n "Enter static IP address (e.g., 192.168.1.100): "
read -r STATIC_IP
echo -n "Enter subnet mask (e.g., 255.255.255.0): "
read -r SUBNET_MASK
echo -n "Enter gateway IP (e.g., 192.168.1.1): "
read -r GATEWAY_IP
echo -n "Enter primary DNS server (default: 1.1.1.1): "
read -r DNS_PRIMARY
DNS_PRIMARY=${DNS_PRIMARY:-1.1.1.1}

# Update configuration with network settings
jq --arg ip "$STATIC_IP" \
   --arg mask "$SUBNET_MASK" \
   --arg gw "$GATEWAY_IP" \
   --arg dns "$DNS_PRIMARY" \
   '.network.static_ip = $ip | .network.subnet_mask = $mask | .network.gateway = $gw | .network.dns_servers[0] = $dns' \
   "$SERVER_CONFIG_FILE" > "${SERVER_CONFIG_FILE}.tmp" && mv "${SERVER_CONFIG_FILE}.tmp" "$SERVER_CONFIG_FILE"

echo ""
echo "SSH Configuration:"
echo "=================="
echo "For enhanced security, SSH will be configured with:"
echo "• Custom port (2222)"
echo "• Key-based authentication only"
echo "• Root login disabled"
echo ""
echo -n "Do you want to add an SSH public key now? [y/N]: "
read -r ADD_SSH_KEY

if [[ "$ADD_SSH_KEY" =~ ^[Yy]$ ]]; then
    echo "Please paste your SSH public key (one line):"
    read -r SSH_PUBLIC_KEY
    echo "$SSH_PUBLIC_KEY" > /tmp/authorized_keys
fi

# Execute main installer with server configuration
exec "$INSTALLER_DIR/install_limitless_enterprise.sh" \
    --config "$SERVER_CONFIG_FILE" \
    --mode server