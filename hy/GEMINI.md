# GEMINI.md - Hy (Hylang) Project Context

## Project Overview
Hy is a Lisp dialect that is uniquely embedded in Python. Instead of being a separate language that happens to run on the Python VM, Hy directly transforms Lisp code into Python's Abstract Syntax Tree (AST) objects. This allows for seamless interoperability with the entire Python ecosystem.

### Key Technologies
- **Python**: The host language and target for compilation.
- **funcparserlib**: Used for the Lisp reader/parser.
- **pytest**: The primary testing framework.
- **Sphinx**: Used for generating documentation.

### Core Architecture
- `hy/reader/`: Contains the logic for reading Lisp source and converting it into "models".
- `hy/models.py`: Defines the Hy models (Symbols, Expressions, Lists, Keywords, etc.) that represent the Lisp forms.
- `hy/compiler.py`: The heart of Hy; it compiles Hy models into Python AST nodes.
- `hy/macros.py`: Implements the macro system, allowing for compile-time code transformation.
- `hy/cmdline.py`: Entry points for the `hy` CLI, REPL, and other utilities.

---

## Building and Running

### Installation
To install the project in editable mode with all development dependencies:
```bash
pip install -e .
```

### Running Hy
- **REPL**: Just run `hy`.
- **Script**: Run `hy your_script.hy`.
- **Compile to Python Source**: Use `hy2py your_script.hy` to see the generated Python code.
- **Byte-compile**: Use `hyc your_script.hy` to generate `.pyc` files.

---

## Testing

Tests are managed with `pytest`.

- **Run all tests**:
  ```bash
  pytest
  ```
- **Skip slow tests** (mainly in `test_bin.py`):
  ```bash
  pytest --ignore=tests/test_bin.py
  ```
- **Clean test artifacts**:
  ```bash
  git clean -dfx tests/
  ```

---

## Documentation

The documentation is located in the `docs/` directory.

- **Build HTML documentation**:
  ```bash
  pip install -r docs/requirements.txt
  cd docs
  sphinx-build . _build -b html
  ```

---

## Development Conventions

### Coding Style & Standards
- **Lisp Idioms**: While it's Python under the hood, Hy follows Lisp naming conventions (e.g., kebab-case for symbols, which are mangled to snake_case in Python).
- **No Inline TODOs**: Do not commit "FIXME", "TODO", or "XXX" comments. Use the GitHub issue tracker instead.
- **Commit Messages**: 
    - First line: 50 characters or less, describing the change.
    - Separate body with a blank line if more detail is needed.
    - Avoid merge commits in PRs (prefer rebasing).

### Contribution Workflow
1. **Tests**: New features or bug fixes *must* include tests.
2. **NEWS**: If a change is user-visible, add an entry to the `NEWS.rst` file.
3. **AUTHORS**: Add yourself to the `AUTHORS` file in a separate commit when making your first contribution.
4. **Pull Requests**: PRs require approval from one or two core team members depending on the complexity.

### Project-Specific Files
- `setup.py`: Contains custom installation logic to compile Hy files during setup.
- `.mailmap`: Used for mapping author names/emails in git history.
- `MANIFEST.in`: Specifies non-code files to include in the distribution.
