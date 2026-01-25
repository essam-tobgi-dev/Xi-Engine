import re

def classify_code_block(code_content):
    """Determine the appropriate language class for a code block based on its content."""

    # C++ indicators
    cpp_keywords = [
        'class ', 'struct ', 'template<', 'namespace ', '#pragma once',
        'public:', 'private:', 'protected:', 'virtual ', 'override',
        'constexpr', 'const ', 'void ', 'uint32_t', 'std::', '->',
        'nullptr', '#include', 'using ', 'typename', 'auto ', 'enum ',
        'delete ', 'new ', 'return ', 'if (', 'for (', 'while (',
        'static_cast', 'std::vector', 'std::unordered_map', 'float ',
        'int ', 'bool ', 'size_t', '::',
    ]

    # GLSL indicators
    glsl_keywords = [
        'vec2', 'vec3', 'vec4', 'mat2', 'mat3', 'mat4',
        'uniform ', 'varying ', 'attribute ', 'in ', 'out ',
        'gl_Position', 'gl_FragColor', 'texture2D', 'sampler2D',
        'void main()', 'precision ', 'mediump', 'highp', 'lowp',
    ]

    # Lua indicators
    lua_keywords = [
        'local ', 'function ', ' end', 'then', 'elseif',
        'require ', 'return', '..', 'self:', 'self.',
    ]

    # Plaintext/diagram indicators (ASCII art, file trees, memory layouts, etc.)
    plaintext_indicators = [
        '├──', '└──', '│', '┌', '┐', '└', '┘', '├', '┤', '┴', '┬',
        '[Entity', 'Memory Layout', '/', 'Xi Engine/', 'Pool:',
        'Grammar Rules', 'Execute:', 'Update Phase:', 'Render Phase:',
        '→', '←', '▼', '▲', '...', 'Step ', 'Before ', 'After ',
        'Input:', 'Output:', 'Token{', 'Components:', 'Entities:',
        'Index map:', 'Data flows:', 'requiredMask', 'entityMask',
    ]

    # Count indicators
    cpp_score = sum(1 for keyword in cpp_keywords if keyword in code_content)
    glsl_score = sum(1 for keyword in glsl_keywords if keyword in code_content)
    lua_score = sum(1 for keyword in lua_keywords if keyword in code_content)
    plaintext_score = sum(1 for indicator in plaintext_indicators if indicator in code_content)

    # Check if it looks like a file tree or diagram
    if any(indicator in code_content for indicator in ['├──', '└──', '│   ', 'Xi Engine/']):
        return 'language-plaintext'

    # Check for memory layout or ASCII diagrams
    if 'Pool:' in code_content or 'Layout:' in code_content or '→' in code_content:
        return 'language-plaintext'

    # Check for algorithm/grammar descriptions
    if 'Grammar Rules' in code_content or 'Execute:' in code_content or 'Step ' in code_content:
        return 'language-plaintext'

    # Determine language based on scores
    if glsl_score > 0 and glsl_score >= cpp_score:
        return 'language-glsl'
    elif lua_score > 0 and lua_score > cpp_score:
        return 'language-lua'
    elif cpp_score > plaintext_score and cpp_score > 0:
        return 'language-cpp'
    elif plaintext_score > 0:
        return 'language-plaintext'
    else:
        # Default: if it has code-like structure, assume C++, else plaintext
        if any(indicator in code_content for indicator in ['{', '}', '(', ')', ';']):
            return 'language-cpp'
        return 'language-plaintext'

def fix_code_blocks(content):
    """Add language classes to all code blocks."""

    # Find all <pre><code>...</code></pre> blocks
    def replace_code_block(match):
        # Extract the code content
        code_content = match.group(1)

        # Determine language
        lang_class = classify_code_block(code_content)

        # Return with language class
        return f'<pre><code class="{lang_class}">{code_content}</code></pre>'

    # Pattern to match <pre><code> without class attribute
    pattern = r'<pre><code>(.*?)</code></pre>'

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

    print(f"\nVerification:")
    print(f"  Code blocks without language class: {remaining}")
    print(f"  Comparison boxes updated: {comparison_count}")

if __name__ == '__main__':
    main()
