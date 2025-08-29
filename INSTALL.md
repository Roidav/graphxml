📄 INSTALL.md

# ⚙️ Installation Guide — GraphXML

GraphXML is written in **C + SDL2**.  
To build the project you will need:

- A C compiler (GCC / Clang / MSVC)
- SDL2 development libraries + SDL2_ttf
- (Optional) Python 3 or Node.js if you want to test FFI bindings

---

## 1. Clone Repository

```bash
git clone https://github.com/Ferki-git-creator/graphxml.git
cd graphxml


---

2. Install Dependencies

Linux (Debian/Ubuntu)

sudo apt update
sudo apt install build-essential libsdl2-dev libsdl2-ttf-dev

macOS (Homebrew)

brew install sdl2 sdl2_ttf

Windows (MSYS2)

pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf

Or download SDL2 and SDL2_ttf from libsdl.org and configure manually.


---

3. Build Project

make

This will create the binary:

graphxml


---

4. Run Example

./graphxml examples/example.graphxml

You should see a simple line chart rendered in an SDL2 window.
