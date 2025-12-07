# 🐜 Formica - Native Zettelkasten Note-Taking

A lightweight, native alternative to Obsidian built with Qt6/C++. Designed for FreeBSD and Unix systems with true Zettelkasten methodology.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Platform](https://img.shields.io/badge/platform-FreeBSD%20%7C%20Linux-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Features

### 🗂️ **Multi-Vault System**
- Create separate, isolated knowledge bases
- Switch between vaults seamlessly
- Auto-discovery of existing vaults

### 📝 **True Zettelkasten**
- Traditional numbering: `1` → `1a` → `1a1` → `1a1a`
- Automatic ID generation with proper branching
- Create child notes with intelligent numbering

### 🔗 **Wiki-Style Linking**
- `[[Note Name]]` linking between notes
- Click to navigate or create missing notes
- Syntax highlighting for links

### 📅 **Daily Notes**
- Press `Ctrl+D` for today's note
- Automatic date-based file creation
- Template with structured sections

### 🔍 **Powerful Search**
- Full-text search across all notes
- Real-time file filtering
- Context-aware results

### 🎨 **Customization**
- Dark/Light themes
- Font selection (monospace optimized)
- Adjustable font sizes
- Line wrapping options

### ⚡ **Native Performance**
- Pure Qt6/C++ - no Electron bloat
- Fast startup and file operations
- Minimal memory usage
- Cross-platform file manager integration

## 🚀 Quick Start

### Prerequisites
- Qt6 (base + tools)
- CMake 3.16+
- C++17 compiler

### Build on FreeBSD
```bash
# Install dependencies
pkg install qt6-base qt6-tools cmake

# Build
git clone https://github.com/jcbastwsk/formica.git
cd formica
chmod +x build.sh
./build.sh

# Run
./build/formica
```

### Build on Linux
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-tools-dev cmake

# Arch Linux
sudo pacman -S qt6-base qt6-tools cmake

# Build and run
./build.sh
./build/formica
```

## 📖 Usage

### First Launch
1. Formica creates your first vault automatically
2. Opens with a Welcome note containing documentation
3. Start creating notes immediately!

### Creating Notes
- **New Zettel**: `Ctrl+Shift+N` - Auto-numbered Zettelkasten note
- **Daily Note**: `Ctrl+D` - Today's journal entry
- **Regular Note**: `Ctrl+N` - Standard markdown file

### Navigation
- **Wiki Links**: Type `[[Note Name]]` to link between notes
- **Vault Switching**: `Ctrl+Shift+O` to change vaults
- **Search**: `Ctrl+F` to search all files

### Zettelkasten Workflow
1. Create main topic: `1 Main Idea`
2. Add subtopic: `[[1a]]` → creates `1a Subtopic`
3. Branch further: `[[1a1]]` → creates `1a1 Detail`
4. Cross-reference: `[[2a]]` to link to other topics

## 🏗️ Project Structure

```
formica/
├── src/
│   ├── main.cpp           # Application entry point
│   ├── mainwindow.*       # Main UI window
│   ├── vaultmanager.*     # Vault system
│   ├── editor.*           # Markdown editor with syntax highlighting
│   ├── linkparser.*       # Wiki link and Zettel ID parsing
│   ├── filetree.*         # File browser with context menus
│   ├── search.*           # Full-text search
│   ├── settings.*         # Theme and preferences
│   └── preferencesdialog.* # Settings UI
├── CMakeLists.txt         # Build configuration
├── build.sh               # Build script
└── README.md              # This file
```

## 🛠️ Development

### Architecture
- **Qt6 Widgets** for native UI
- **Model-View pattern** for file management
- **Settings system** with persistent preferences
- **Plugin-ready** architecture for future extensions

### Contributing
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on FreeBSD/Linux
5. Submit a pull request

## 📊 Roadmap

- [ ] Graph view of note connections
- [ ] Plugin system
- [ ] Export to PDF/HTML
- [ ] Note templates
- [ ] Tag system
- [ ] Backlinks panel
- [ ] vim key bindings

## 🐛 Troubleshooting

### Display Issues on Headless Systems
```bash
# Use alternative display backend
export QT_QPA_PLATFORM=wayland
# or
export QT_QPA_PLATFORM=offscreen
```

### Missing File Manager Integration
```bash
# Install a supported file manager
pkg install dolphin  # KDE
pkg install nautilus # GNOME
pkg install thunar   # XFCE
```

## 📄 License

MIT License - see LICENSE file for details.

## 🙏 Acknowledgments

- Inspired by Niklas Luhmann's Zettelkasten method
- Built for researchers, writers, and knowledge workers
- Designed with FreeBSD and Unix philosophy in mind

---

**Built with ❤️ for the note-taking community**