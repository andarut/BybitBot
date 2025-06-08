if [ -n "$BASH_VERSION" ]; then
  SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
elif [ -n "$ZSH_VERSION" ]; then
  SCRIPT_DIR="$(cd "$(dirname "${(%):-%x}")" && pwd)"
fi

ROOT_DIR="${SCRIPT_DIR}/.."
PROFILES_DIR="${ROOT_DIR}/profiles"
PROFILE_PATH="${PROFILES_DIR}/mac_arm"

VENV_DIR="${ROOT_DIR}/.venv"
VENV_BIN_DIR="${VENV_DIR}/bin"

# Create venv and install conan
source ${SCRIPT_DIR}/install_conan.sh

# Activate venv
source ${ROOT_DIR}/.venv/bin/activate

# Enable LeakSanitizer
export ASAN_OPTIONS="detect_leaks=1"

# Install with conan
conan install ${ROOT_DIR} --build=missing \
  --profile:host=$PROFILE_PATH \
  --profile:build=$PROFILE_PATH

# Link cmake installed by conan
PRESETS_CONFIG_PATH="${ROOT_DIR}/build/Debug/generators/CMakePresets.json"
PYTHON_EXECUTABLE="${ROOT_DIR}/.venv/bin/python3"
CMAKE_PATH="$($PYTHON_EXECUTABLE -c 'import json, sys; presets = json.load(sys.stdin); print(presets["configurePresets"][0]["cmakeExecutable"])' < "$PRESETS_CONFIG_PATH")"
CMAKE_EXECUTABLE="${ROOT_DIR}/.venv/bin/cmake"

echo "Linking $CMAKE_PATH -> $CMAKE_EXECUTABLE"

rm -f $CMAKE_EXECUTABLE && \
ln -s $CMAKE_PATH $CMAKE_EXECUTABLE