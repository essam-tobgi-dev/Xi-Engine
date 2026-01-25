// Xi Engine Documentation Interactive Features

document.addEventListener('DOMContentLoaded', () => {
    initSmoothScrolling();
    initActiveNavigation();
    initCodeHighlighting();
    initScrollToTop();
    initMobileMenu();
});

// Smooth scrolling for anchor links
function initSmoothScrolling() {
    const links = document.querySelectorAll('a[href^="#"]');

    links.forEach(link => {
        link.addEventListener('click', (e) => {
            const href = link.getAttribute('href');

            // Skip empty anchors
            if (href === '#') {
                e.preventDefault();
                return;
            }

            const target = document.querySelector(href);

            if (target) {
                e.preventDefault();

                const offset = 80; // Offset for fixed headers
                const targetPosition = target.getBoundingClientRect().top + window.pageYOffset - offset;

                window.scrollTo({
                    top: targetPosition,
                    behavior: 'smooth'
                });

                // Update URL without jumping
                history.pushState(null, null, href);
            }
        });
    });
}

// Highlight active navigation item based on scroll position
function initActiveNavigation() {
    const sections = document.querySelectorAll('.section, .subsection[id]');
    const navLinks = document.querySelectorAll('.nav-link, .nav-sublink');

    function updateActiveNav() {
        let current = '';

        sections.forEach(section => {
            const sectionTop = section.offsetTop;
            const sectionHeight = section.clientHeight;

            if (window.pageYOffset >= sectionTop - 150) {
                current = section.getAttribute('id');
            }
        });

        navLinks.forEach(link => {
            link.classList.remove('active');

            const href = link.getAttribute('href');
            if (href === `#${current}`) {
                link.classList.add('active');

                // Also mark parent nav-link as active
                const parent = link.closest('.nav-section');
                if (parent) {
                    const parentLink = parent.querySelector('.nav-link');
                    if (parentLink) {
                        parentLink.classList.add('active');
                    }
                }
            }
        });
    }

    // Throttle scroll event for performance
    let ticking = false;
    window.addEventListener('scroll', () => {
        if (!ticking) {
            window.requestAnimationFrame(() => {
                updateActiveNav();
                ticking = false;
            });

            ticking = true;
        }
    });

    // Initial call
    updateActiveNav();
}

// Simple code syntax highlighting
function initCodeHighlighting() {
    const codeBlocks = document.querySelectorAll('pre code');

    codeBlocks.forEach(block => {
        const language = Array.from(block.classList).find(cls => cls.startsWith('language-'));

        if (!language) return;

        const code = block.textContent;
        const highlighted = highlightCode(code, language);
        block.innerHTML = highlighted;
    });
}

function highlightCode(code, language) {
    if (language.includes('cpp')) {
        return highlightCpp(code);
    } else if (language.includes('glsl')) {
        return highlightGLSL(code);
    } else if (language.includes('lua')) {
        return highlightLua(code);
    }
    return escapeHtml(code);
}

function highlightCpp(code) {
    const keywords = [
        'alignas', 'alignof', 'and', 'and_eq', 'asm', 'atomic_cancel', 'atomic_commit',
        'atomic_noexcept', 'auto', 'bitand', 'bitor', 'bool', 'break', 'case', 'catch',
        'char', 'char8_t', 'char16_t', 'char32_t', 'class', 'compl', 'concept', 'const',
        'consteval', 'constexpr', 'constinit', 'const_cast', 'continue', 'co_await',
        'co_return', 'co_yield', 'decltype', 'default', 'delete', 'do', 'double',
        'dynamic_cast', 'else', 'enum', 'explicit', 'export', 'extern', 'false', 'float',
        'for', 'friend', 'goto', 'if', 'inline', 'int', 'long', 'mutable', 'namespace',
        'new', 'noexcept', 'not', 'not_eq', 'nullptr', 'operator', 'or', 'or_eq',
        'private', 'protected', 'public', 'reflexpr', 'register', 'reinterpret_cast',
        'requires', 'return', 'short', 'signed', 'sizeof', 'static', 'static_assert',
        'static_cast', 'struct', 'switch', 'synchronized', 'template', 'this',
        'thread_local', 'throw', 'true', 'try', 'typedef', 'typeid', 'typename', 'union',
        'unsigned', 'using', 'virtual', 'void', 'volatile', 'wchar_t', 'while', 'xor',
        'xor_eq', 'override', 'final', 'import', 'module'
    ];

    const types = [
        'std', 'string', 'vector', 'map', 'set', 'unordered_map', 'unordered_set',
        'shared_ptr', 'unique_ptr', 'weak_ptr', 'function', 'size_t', 'uint32_t',
        'uint64_t', 'int32_t', 'int64_t', 'glm', 'vec2', 'vec3', 'vec4', 'mat3', 'mat4'
    ];

    let result = escapeHtml(code);

    // Highlight strings
    result = result.replace(/("(?:\\.|[^"\\])*")/g, '<span style="color: #98c379;">$1</span>');
    result = result.replace(/('(?:\\.|[^'\\])*')/g, '<span style="color: #98c379;">$1</span>');

    // Highlight numbers
    result = result.replace(/\b(\d+\.?\d*[fFuUlL]*)\b/g, '<span style="color: #d19a66;">$1</span>');

    // Highlight keywords
    keywords.forEach(keyword => {
        const regex = new RegExp(`\\b(${keyword})\\b`, 'g');
        result = result.replace(regex, '<span style="color: #c678dd;">$1</span>');
    });

    // Highlight types
    types.forEach(type => {
        const regex = new RegExp(`\\b(${type})\\b`, 'g');
        result = result.replace(regex, '<span style="color: #e5c07b;">$1</span>');
    });

    // Highlight preprocessor directives
    result = result.replace(/^(#.*$)/gm, '<span style="color: #c678dd;">$1</span>');

    return result;
}

function highlightGLSL(code) {
    const keywords = [
        'attribute', 'const', 'uniform', 'varying', 'break', 'continue', 'do', 'for',
        'while', 'if', 'else', 'in', 'out', 'inout', 'true', 'false', 'discard',
        'return', 'struct', 'layout', 'precision', 'highp', 'mediump', 'lowp'
    ];

    const types = [
        'float', 'int', 'void', 'bool', 'mat2', 'mat3', 'mat4', 'vec2', 'vec3', 'vec4',
        'ivec2', 'ivec3', 'ivec4', 'bvec2', 'bvec3', 'bvec4', 'sampler2D', 'samplerCube'
    ];

    const functions = [
        'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'pow', 'exp', 'log', 'exp2', 'log2',
        'sqrt', 'inversesqrt', 'abs', 'sign', 'floor', 'ceil', 'fract', 'mod', 'min',
        'max', 'clamp', 'mix', 'step', 'smoothstep', 'length', 'distance', 'dot', 'cross',
        'normalize', 'reflect', 'refract', 'texture', 'texture2D', 'textureCube'
    ];

    let result = escapeHtml(code);

    // Highlight preprocessor
    result = result.replace(/^(#.*$)/gm, '<span style="color: #c678dd;">$1</span>');

    // Highlight keywords
    keywords.forEach(keyword => {
        const regex = new RegExp(`\\b(${keyword})\\b`, 'g');
        result = result.replace(regex, '<span style="color: #c678dd;">$1</span>');
    });

    // Highlight types
    types.forEach(type => {
        const regex = new RegExp(`\\b(${type})\\b`, 'g');
        result = result.replace(regex, '<span style="color: #e5c07b;">$1</span>');
    });

    // Highlight functions
    functions.forEach(func => {
        const regex = new RegExp(`\\b(${func})\\b`, 'g');
        result = result.replace(regex, '<span style="color: #61afef;">$1</span>');
    });

    // Highlight numbers
    result = result.replace(/\b(\d+\.?\d*[fF]?)\b/g, '<span style="color: #d19a66;">$1</span>');

    return result;
}

function highlightLua(code) {
    const keywords = [
        'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for', 'function',
        'if', 'in', 'local', 'nil', 'not', 'or', 'repeat', 'return', 'then', 'true',
        'until', 'while'
    ];

    let result = escapeHtml(code);

    // Highlight strings
    result = result.replace(/("(?:\\.|[^"\\])*")/g, '<span style="color: #98c379;">$1</span>');
    result = result.replace(/('(?:\\.|[^'\\])*')/g, '<span style="color: #98c379;">$1</span>');

    // Highlight numbers
    result = result.replace(/\b(\d+\.?\d*)\b/g, '<span style="color: #d19a66;">$1</span>');

    // Highlight keywords
    keywords.forEach(keyword => {
        const regex = new RegExp(`\\b(${keyword})\\b`, 'g');
        result = result.replace(regex, '<span style="color: #c678dd;">$1</span>');
    });

    // Highlight function names
    result = result.replace(/\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(/g, '<span style="color: #61afef;">$1</span>(');

    return result;
}

function escapeHtml(text) {
    const map = {
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#039;'
    };
    return text.replace(/[&<>"']/g, m => map[m]);
}

// Scroll to top functionality
function initScrollToTop() {
    // Create scroll to top button
    const button = document.createElement('button');
    button.innerHTML = '↑';
    button.className = 'scroll-to-top';
    button.style.cssText = `
        position: fixed;
        bottom: 2rem;
        right: 2rem;
        width: 3rem;
        height: 3rem;
        border-radius: 50%;
        background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
        color: white;
        border: none;
        font-size: 1.5rem;
        cursor: pointer;
        opacity: 0;
        transition: opacity 0.3s, transform 0.3s;
        z-index: 1000;
        box-shadow: 0 4px 12px rgba(99, 102, 241, 0.4);
    `;

    document.body.appendChild(button);

    // Show/hide button based on scroll
    window.addEventListener('scroll', () => {
        if (window.pageYOffset > 300) {
            button.style.opacity = '1';
            button.style.transform = 'translateY(0)';
        } else {
            button.style.opacity = '0';
            button.style.transform = 'translateY(20px)';
        }
    });

    // Scroll to top on click
    button.addEventListener('click', () => {
        window.scrollTo({
            top: 0,
            behavior: 'smooth'
        });
    });

    // Hover effect
    button.addEventListener('mouseenter', () => {
        button.style.transform = 'translateY(-4px)';
    });

    button.addEventListener('mouseleave', () => {
        button.style.transform = 'translateY(0)';
    });
}

// Mobile menu toggle
function initMobileMenu() {
    if (window.innerWidth > 768) return;

    const sidebar = document.querySelector('.sidebar');
    const menuButton = document.createElement('button');

    menuButton.innerHTML = '☰';
    menuButton.className = 'mobile-menu-toggle';
    menuButton.style.cssText = `
        position: fixed;
        top: 1rem;
        left: 1rem;
        width: 3rem;
        height: 3rem;
        border-radius: 0.5rem;
        background: var(--bg-secondary);
        border: 1px solid var(--border-color);
        color: var(--text-primary);
        font-size: 1.5rem;
        cursor: pointer;
        z-index: 1001;
        display: none;
    `;

    // Show button on mobile
    if (window.innerWidth <= 768) {
        menuButton.style.display = 'block';
    }

    document.body.appendChild(menuButton);

    menuButton.addEventListener('click', () => {
        sidebar.classList.toggle('open');
    });

    // Close sidebar when clicking outside on mobile
    document.addEventListener('click', (e) => {
        if (window.innerWidth <= 768 &&
            sidebar.classList.contains('open') &&
            !sidebar.contains(e.target) &&
            e.target !== menuButton) {
            sidebar.classList.remove('open');
        }
    });

    // Close sidebar when clicking a link on mobile
    const navLinks = sidebar.querySelectorAll('a');
    navLinks.forEach(link => {
        link.addEventListener('click', () => {
            if (window.innerWidth <= 768) {
                sidebar.classList.remove('open');
            }
        });
    });
}

// Add copy button to code blocks
document.querySelectorAll('pre').forEach(pre => {
    const button = document.createElement('button');
    button.textContent = 'Copy';
    button.className = 'copy-button';
    button.style.cssText = `
        position: absolute;
        top: 0.5rem;
        right: 0.5rem;
        padding: 0.5rem 1rem;
        background: var(--bg-tertiary);
        border: 1px solid var(--border-color);
        border-radius: 0.375rem;
        color: var(--text-secondary);
        font-size: 0.875rem;
        cursor: pointer;
        opacity: 0;
        transition: opacity 0.3s, background 0.3s;
    `;

    pre.style.position = 'relative';
    pre.appendChild(button);

    pre.addEventListener('mouseenter', () => {
        button.style.opacity = '1';
    });

    pre.addEventListener('mouseleave', () => {
        button.style.opacity = '0';
    });

    button.addEventListener('click', () => {
        const code = pre.querySelector('code').textContent;
        navigator.clipboard.writeText(code).then(() => {
            button.textContent = 'Copied!';
            button.style.background = 'var(--success-color)';
            setTimeout(() => {
                button.textContent = 'Copy';
                button.style.background = 'var(--bg-tertiary)';
            }, 2000);
        });
    });
});

// Progress bar
const progressBar = document.createElement('div');
progressBar.style.cssText = `
    position: fixed;
    top: 0;
    left: 0;
    height: 3px;
    background: linear-gradient(90deg, var(--primary-color), var(--secondary-color), var(--accent-color));
    z-index: 9999;
    transition: width 0.1s;
`;
document.body.appendChild(progressBar);

window.addEventListener('scroll', () => {
    const winScroll = document.body.scrollTop || document.documentElement.scrollTop;
    const height = document.documentElement.scrollHeight - document.documentElement.clientHeight;
    const scrolled = (winScroll / height) * 100;
    progressBar.style.width = scrolled + '%';
});
