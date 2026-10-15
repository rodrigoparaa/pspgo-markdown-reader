# ==============================================================================
# Isolated PSP Virtual Development Environment (PSPDEV)
# Usage: source activate.sh
# To exit: deactivate_psp
# ==============================================================================

# Determine directory of this script
if [ -n "$BASH_SOURCE" ]; then
    _SCRIPT_PATH="${BASH_SOURCE[0]}"
elif [ -n "$ZSH_VERSION" ]; then
    _SCRIPT_PATH="${(%):-%x}"
else
    _SCRIPT_PATH="$0"
fi

_PSP_ROOT="$(cd "$(dirname "$_SCRIPT_PATH")" && pwd)"

# Check local pspdev folder or existing $PSPDEV
if [ -d "$_PSP_ROOT/pspdev/bin" ]; then
    export PSPDEV="$_PSP_ROOT/pspdev"
elif [ -n "$PSPDEV" ] && [ -d "$PSPDEV/bin" ]; then
    export PSPDEV="$PSPDEV"
else
    echo "❌ Error: PSPDEV toolchain not found in $_PSP_ROOT/pspdev or \$PSPDEV"
    return 1 2>/dev/null || exit 1
fi

# Save previous state for clean deactivation
_OLD_PSP_PATH="$PATH"
_OLD_PSP_PS1="$PS1"
export _OLD_PSP_PATH _OLD_PSP_PS1

# Add binaries to PATH
export PATH="$PSPDEV/bin:$PATH"

# Deactivation function
deactivate_psp() {
    if [ -n "$_OLD_PSP_PATH" ]; then
        export PATH="$_OLD_PSP_PATH"
        unset _OLD_PSP_PATH
    fi
    if [ -n "$_OLD_PSP_PS1" ]; then
        export PS1="$_OLD_PSP_PS1"
        unset _OLD_PSP_PS1
    fi
    unset PSPDEV
    unset -f deactivate_psp
    echo "👋 PSPDEV environment deactivated."
}

# Update prompt
export PS1="(psp-env) $PS1"

echo "=========================================================="
echo "🎮 PSPDEV Environment Activated"
echo "   Compiler: $(psp-gcc --version 2>/dev/null | head -n 1)"
echo "   SDK Path: $PSPDEV"
echo "   Exit:     deactivate_psp"
echo "=========================================================="
