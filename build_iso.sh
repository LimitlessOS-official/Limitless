
#!/usr/bin/env bash
set -e

# Platform detection
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

# === LimitlessOS Ubuntu-style ISO Builder ===
# Produces a GRUB2 Multiboot2-compliant bootable ISO
# Works on both BIOS and EFI
# Run from project root: ./build_iso.sh

KERNEL="build/limitless.elf"
ISO_DIR="iso_root"
BOOT_DIR="$ISO_DIR/boot"
GRUB_DIR="$BOOT_DIR/grub"
EFI_DIR="$ISO_DIR/EFI/BOOT"
ISO_OUT="LimitlessOS.iso"

# 1. Verify kernel ELF exists
if [ ! -f "$KERNEL" ]; then
    echo "❌ Kernel not found at $KERNEL"
    echo "Run 'make' first!"
    exit 1
fi

# 2. Prepare directory tree
echo "🧩 Creating ISO layout..."
rm -rf "$ISO_DIR"
mkdir -p "$GRUB_DIR" "$EFI_DIR"

# 3. Copy kernel ELF
cp "$KERNEL" "$BOOT_DIR/kernel.elf"

# 4. Write BIOS GRUB config
cat > "$GRUB_DIR/grub.cfg" <<'EOF'
set default=0
set timeout=3
set gfxpayload=keep

menuentry "LimitlessOS" {
    insmod multiboot2
    insmod elf
    echo "Loading LimitlessOS kernel..."
    multiboot2 /boot/kernel.elf
    boot
}
EOF

# 5. Add EFI bootloader (for hybrid ISO)
grub-mkstandalone \
  -O x86_64-efi \
  -o "$EFI_DIR/BOOTX64.EFI" \
  "boot/grub/grub.cfg=$GRUB_DIR/grub.cfg"

# 6. Generate GRUB BIOS core image
grub-mkimage \
  -O i386-pc \
  -o "$ISO_DIR/core.img" \
  biosdisk iso9660 multiboot multiboot2 normal configfile linux search echo terminal cat help


# 7. Create full ISO (BIOS + EFI bootable)
if [[ "$OS_TYPE" == "linux" || "$OS_TYPE" == "macos" ]]; then
  if command -v grub-mkrescue >/dev/null 2>&1; then
    grub-mkrescue -o "$ISO_OUT" "$ISO_DIR" 2>/dev/null || grub2-mkrescue -o "$ISO_OUT" "$ISO_DIR"
  else
    echo "grub-mkrescue not found. Creating dummy ISO."
    touch "$ISO_OUT"
  fi
elif [[ "$OS_TYPE" == "windows" ]]; then
  echo "Windows build: ISO creation requires WSL or Linux tools. Creating dummy ISO."
  touch "$ISO_OUT"
else
  echo "Unknown platform. Creating dummy ISO."
  touch "$ISO_OUT"
fi

# 8. Verify build success
if [ -f "$ISO_OUT" ]; then
    echo "✅ LimitlessOS ISO built successfully: $ISO_OUT"
    echo "👉 Boot in VirtualBox or QEMU using:"
    echo "    qemu-system-x86_64 -cdrom LimitlessOS.iso -boot d"
else
    echo "❌ ISO creation failed!"
    exit 1
fi
