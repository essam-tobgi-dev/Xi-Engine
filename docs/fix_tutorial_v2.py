import re

def classify_code_block(code_content):
    """Determine the appropriate language class for a code block based on its content."""

    # Strip for easier analysis
    code = code_content.strip()

    # Check for simple data structure notation (plaintext)
    if code.startswith('[') and code.endswith(']') and code.count('[') <= 5 and code.count('\n') == 0:
        return 'language-plaintext'

    # Check for pseudo-code patterns
    if 'function ' in code and ':' in code and 'while ' in code and ' != ' in code:
        # This is pseudo-code, not real code
        return 'language-plaintext'

    # GLSL indicators - must be strong matches
    glsl_strong = [
        'vec2', 'vec3', 'vec4', 'mat2', 'mat3', 'mat4',
        'uniform ', 'varying ', 'attribute ',
        'gl_Position', 'gl_FragColor', 'texture2D', 'sampler2D',
        'precision mediump', 'precision highp', 'precision lowp',
    ]

    # Lua indicators - strong matches
    lua_strong = [
        'function ', ' end\n', ' end$', 'local ', 'then\n', 'elseif ',
    ]

    # C++ strong indicators
    cpp_strong = [
        'class ', 'struct ', 'template<', 'namespace ', '#pragma once',
        'public:', 'private:', 'protected:', 'virtual ', 'override',
        '#include <', '#include "', 'std::', '::', 'nullptr',
        'constexpr ', 'static_cast<', 'uint32_t', 'size_t',
    ]

    # Plaintext/diagram indicators (ASCII art, file trees, memory layouts, etc.)
    plaintext_indicators = [
        '├──', '└──', '│   ', '┌─', '┐', '└─', '┘', '├─', '┤', '┴', '┬',
        'Xi Engine/', 'Engine/', 'Pool:', 'Layout:',
        'Grammar Rules:', 'Execute:', 'Update Phase:', 'Render Phase:',
        '→', '←', '▼', '▲',
        'Input:', 'Output:', 'Token{',
        'Before removing', 'After swap',
        'Index map:', 'Data flows:', 'requiredMask =', 'entityMask =',
        'Step 1:', 'Step 2:', 'Step 3:',
        '// Before', '// After',
    ]

    # Check for file tree or directory structure
    if '├──' in code or '└──' in code or code.startswith('Xi Engine/'):
        return 'language-plaintext'

    # Check for ASCII diagrams
    if any(char in code for char in ['┌', '┐', '└', '┘', '├', '┤', '│', '─', '┬', '┴']):
        return 'language-plaintext'

    # Check for memory layout or data structure diagrams
    if 'Pool:' in code or 'Layout:' in code or '→' in code or 'Index map:' in code:
        return 'language-plaintext'

    # Check for algorithm/grammar descriptions
    if any(indicator in code for indicator in ['Grammar Rules:', 'Execute:', 'Step 1:', 'Step 2:']):
        return 'language-plaintext'

    # Check for before/after comparisons or command examples
    if ('Before ' in code and 'After ' in code) or code.startswith('//'):
        if 'class ' not in code and 'void ' not in code:
            return 'language-plaintext'

    # Check for GLSL (strong match required)
    if any(indicator in code for indicator in glsl_strong):
        return 'language-glsl'

    # Check for Lua (strong match required)
    # Must have 'function' or 'local' AND 'end'
    has_function = 'function ' in code or 'function(' in code
    has_end = re.search(r'\bend\b', code) is not None
    has_local = 'local ' in code
    has_then = 'then' in code

    if (has_function or has_local) and has_end:
        return 'language-lua'

    # Check for C++ (strong match required)
    if any(indicator in code for indicator in cpp_strong):
        return 'language-cpp'

    # Check for C-style code patterns
    if ('{' in code and '}' in code and ';' in code):
        # Could be C++ or GLSL - check for more specific patterns
        if 'void ' in code or 'class ' in code or 'struct ' in code:
            return 'language-cpp'

    # Check if it looks like plaintext
    if any(indicator in code for indicator in plaintext_indicators):
        return 'language-plaintext'

    # Default heuristic: if it has braces and semicolons, it's likely C++
    # Otherwise, it's plaintext
    if '{' in code and '}' in code and ';' in code:
        return 'language-cpp'

    return 'language-plaintext'

def fix_code_blocks(content):
    """Add language classes to all code blocks."""

    # Find all <pre><code>...</code></pre> blocks
    def replace_code_block(match):
        # Check if it already has a class
        opening_tag = match.group(1)
        code_content = match.group(2)

        # Skip if already has class
        if 'class=' in opening_tag:
            return match.group(0)

        # Determine language
        lang_class = classify_code_block(code_content)

        # Return with language class
        return f'<pre><code class="{lang_class}">{code_content}</code></pre>'

    # Pattern to match <pre><code ...>...</code></pre>
    pattern = r'<pre><code([^>]*)>(.*?)</code></pre>'

    # Replace all occurrences
    new_content = re.sub(pattern, replace_code_block, content, flags=re.DOTALL)

    return new_content

def fix_comparison_boxes(content):
    """Replace comparison-box with comparison-box-vertical."""
    return content.replace('<div class="comparison-box">', '<div class="comparison-box-vertical">')

def main():
    # Read the file
    with open('tutorial.html', 'r', encoding='utf-8') as f:
        content = f.read()

    print("Processing tutorial.html...")

    # Apply fixes
    print("Fixing code blocks...")
    content = fix_code_blocks(content)

    print("Fixing comparison boxes...")
    content = fix_comparison_boxes(content)

    # Write back
    with open('tutorial.html', 'w', encoding='utf-8') as f:
        f.write(content)

    print("Done! Changes applied successfully.")

    # Verify
    remaining = len(re.findall(r'<pre><code(?![^>]*class=)', content))
    comparison_count = len(re.findall(r'<div class="comparison-box-vertical">', content))

    lang_cpp = len(re.findall(r'language-cpp', content))
    lang_glsl = len(re.findall(r'language-glsl', content))
    lang_lua = len(re.findall(r'language-lua', content))
    lang_plaintext = len(re.findall(r'language-plaintext', content))

    print(f"\nVerification:")
    print(f"  Code blocks without language class: {remaining}")
    print(f"  Comparison boxes updated: {comparison_count}")
    print(f"\nLanguage distribution:")
    print(f"  C++: {lang_cpp}")
    print(f"  GLSL: {lang_glsl}")
    print(f"  Lua: {lang_lua}")
    print(f"  Plaintext: {lang_plaintext}")

if __name__ == '__main__':
    main()
