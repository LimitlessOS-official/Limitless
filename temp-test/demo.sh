#!/bin/sh
# LimitlessOS Demo Script
# Shows installer, desktop, and bootsplash mockups

show_bootsplash() {
    clear
    echo "=================================================="
    echo "        LimitlessOS Boot Splash Demo              "
    echo "=================================================="
    echo "   [Logo]   LimitlessOS - Enterprise Edition      "
    echo "   Fast. Secure. Limitless.                       "
    echo "=================================================="
    sleep 2
}

show_installer() {
    clear
    echo "=================================================="
    echo "        LimitlessOS Installer Demo                "
    echo "=================================================="
    echo "Welcome to LimitlessOS!"
    echo "[1] Install Enterprise Edition"
    echo "[2] Install Developer Edition"
    echo "[3] Install Gaming Edition"
    echo "[4] Custom/Advanced Install"
    echo "[ ] Optional Features: Pentest, Dev, Gaming, etc."
    echo "--------------------------------------------------"
    echo "[C] Continue   [Q] Quit"
    echo "=================================================="
    sleep 3
}

show_desktop() {
    clear
    echo "=================================================="
    echo "        LimitlessOS Desktop Demo                  "
    echo "=================================================="
    echo "[Menu]  [Apps]  [Terminal]  [Settings]  [Monitor] "
    echo "--------------------------------------------------"
    echo "[Wallpaper: LimitlessOS Abstract]"
    echo "[Taskbar: Time | Network | Notifications]"
    echo "[Desktop Icons: Home, App Store, Pentest Suite]"
    echo "=================================================="
    sleep 3
}

show_bootsplash
show_installer
show_desktop
clear
echo "Demo complete."
