# LimitlessOS Installer Build


all:
	@echo "🚀 Building LimitlessOS Installer..."
	@echo "Compiling installer.c and intelligent_installer.c..."
	@g++ -I. -I../ -c installer.c -o installer.o
	@g++ -I. -I../ -c intelligent_installer.c -o intelligent_installer.o
	@ar rcs installer.a installer.o intelligent_installer.o
	@echo "✅ Installer build complete: installer.a"

clean:
	@rm -f installer.o intelligent_installer.o installer.a
	@echo "🧹 Installer build cleaned."

.PHONY: all clean
