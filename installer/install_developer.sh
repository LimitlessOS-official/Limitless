
#!/usr/bin/env bash

##############################################################################
# LimitlessOS Developer Workstation Installation Script
# Optimized for software developers with development tools and environments
##############################################################################

set -e

INSTALLER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEV_CONFIG_FILE="/tmp/limitless_dev_config.json"

PLATFORM="$(uname -s)"
if [[ "$PLATFORM" == "Linux" ]]; then
  OS_TYPE="linux"
elif [[ "$PLATFORM" == "Darwin" ]]; then
  OS_TYPE="macos"
elif [[ "$PLATFORM" == *"MINGW"* || "$PLATFORM" == *"MSYS"* ]]; then
  OS_TYPE="windows"
else
  OS_TYPE="unknown"
fi

# Developer workstation configuration
cat > "$DEV_CONFIG_FILE" << 'EOF'
{
  "deployment": {
    "mode": "developer",
    "automation_level": "semi_automated",
    "validation_level": "comprehensive"
  },
  "security": {
    "encryption": true,
    "secure_boot": false,
    "firewall": true,
    "audit_logging": false,
    "developer_mode": true
  },
  "system": {
    "hostname": "limitless-dev",
    "timezone": "America/New_York",
    "locale": "en_US.UTF-8",
    "performance_profile": "high_performance"
  },
  "user": {
    "username": "developer",
    "fullname": "LimitlessOS Developer",
    "create_home": true,
    "sudo_access": true,
    "shell": "/bin/zsh"
  },
  "network": {
    "configure_automatically": true,
    "dns_servers": ["8.8.8.8", "8.8.4.4"],
    "enable_ssh": true,
    "ssh_port": 22
  },
  "features": {
    "development_tools": true,
    "enterprise_tools": true,
    "ai_framework": true,
    "multimedia_codecs": true,
    "virtualization": true,
    "containers": true,
    "kubernetes": true,
    "docker": true
  },
  "development": {
    "install_compilers": ["gcc", "clang", "rustc", "go", "nodejs", "python3"],
    "install_editors": ["vim", "emacs", "nano"],
    "install_version_control": ["git", "mercurial", "subversion"],
    "install_databases": ["postgresql", "mysql", "mongodb", "redis"],
    "install_languages": ["python", "nodejs", "java", "rust", "go", "ruby"],
    "install_frameworks": ["django", "flask", "react", "vue", "angular"],
    "setup_development_environment": true
  },
  "partitioning": {
    "auto_partition": true,
    "filesystem": "ext4",
    "swap_size_gb": 16,
    "root_size_percent": 60,
    "home_size_percent": 35,
    "create_separate_home": true
  }
}
EOF

echo "=============================================================================="
echo "                   LimitlessOS Developer Workstation Installer"
echo "=============================================================================="
echo ""
echo "This will install LimitlessOS optimized for software development with:"
echo "• Complete development toolchain (GCC, Clang, Rust, Go, Node.js, Python)"
echo "• Popular editors and IDEs"
echo "• Version control systems (Git, Mercurial, SVN)"
echo "• Database systems (PostgreSQL, MySQL, MongoDB, Redis)"
echo "• Container technologies (Docker, Kubernetes)"
echo "• AI/ML development frameworks"
echo "• Performance optimizations for development workloads"
echo ""
echo "Press Enter to continue or Ctrl+C to abort..."
read -r


# Platform-specific installer execution
if [[ "$OS_TYPE" == "linux" || "$OS_TYPE" == "macos" ]]; then
  exec "$INSTALLER_DIR/install_limitless_enterprise.sh" \
    --config "$DEV_CONFIG_FILE" \
    --mode developer
elif [[ "$OS_TYPE" == "windows" ]]; then
  echo "Windows detected. Please run this installer in WSL or a Linux environment."
  exit 1
else
  echo "Unknown platform. Installer cannot proceed."
  exit 1
fi