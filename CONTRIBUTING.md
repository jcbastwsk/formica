# Contributing to Formica

Thanks for your interest in contributing to Formica! This guide will help you get started.

## 🚀 Quick Setup

```bash
# Clone and build
git clone https://github.com/jcbastwsk/formica.git
cd formica
chmod +x build.sh
./build.sh
```

## 🛠️ Development Environment

### Prerequisites
- Qt6 (base + tools)
- CMake 3.16+
- C++17 compiler
- Git

### Code Style
- Use Qt naming conventions (camelCase for functions, PascalCase for classes)
- Include headers: Qt includes first, then local includes
- 4-space indentation
- Follow existing patterns in the codebase

## 🐛 Reporting Bugs

Please include:
1. **System info**: FreeBSD/Linux version, Qt version
2. **Steps to reproduce** the issue
3. **Expected vs actual behavior**
4. **Screenshots** if UI-related

## 💡 Feature Requests

Before submitting:
1. Check existing issues for duplicates
2. Consider if it fits Formica's philosophy (native, fast, simple)
3. Provide clear use cases and benefits

## 🔧 Pull Requests

### Process
1. Fork the repository
2. Create a descriptive branch name (`feature/vault-encryption`, `fix/search-crash`)
3. Make your changes
4. Test on FreeBSD and/or Linux
5. Update documentation if needed
6. Submit PR with clear description

### Guidelines
- **One feature per PR** - keep changes focused
- **Test thoroughly** - especially file operations and cross-platform compatibility
- **Update README** if adding user-facing features
- **Include motivation** - explain why the change is needed

## 🧪 Testing

### Manual Testing
- Test on different platforms (FreeBSD, Linux)
- Verify vault operations (create, switch, delete)
- Check wiki link functionality
- Test with various markdown files
- Verify themes and font changes

### Areas that need testing
- File manager integration on different desktop environments
- Large vault performance
- Unicode handling in notes
- Edge cases in Zettel ID parsing

## 📁 Project Structure

```
src/
├── main.cpp           # Entry point
├── mainwindow.*       # Main application window
├── vaultmanager.*     # Multi-vault system
├── editor.*           # Markdown editor with syntax highlighting
├── linkparser.*       # Wiki links and Zettelkasten ID parsing
├── filetree.*         # File browser with context menus
├── search.*           # Full-text search functionality
├── settings.*         # Application settings and themes
├── preferencesdialog.* # Settings UI
└── vaultdialog.*      # Vault selection and creation
```

## 🎯 Priority Areas

High impact contributions:
- **Performance improvements** for large vaults
- **Cross-platform compatibility** fixes
- **Accessibility** improvements
- **Plugin system** foundation
- **Export functionality** (PDF, HTML)
- **Graph view** of note connections

## 📝 Documentation

When adding features:
- Update README.md with usage instructions
- Add docstrings for complex functions
- Update keyboard shortcuts in documentation
- Consider adding examples

## 🤝 Community

- Be respectful and constructive
- Help others in issues and discussions
- Share your use cases and workflows
- Consider writing blog posts or tutorials

## 📊 Release Process

Maintainer responsibilities:
1. Review and test PRs
2. Update version numbers
3. Create release notes
4. Tag releases
5. Update documentation

---

**Happy contributing! 🐜**