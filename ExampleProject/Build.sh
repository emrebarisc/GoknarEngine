#!/bin/bash

isDebugBuild=true
cleanBuild=false
build=true
runAfterBuild=false
isUnix=true

projectName="ExampleProject"
directoryName=""

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    gameName="${projectName}.exe"
    isUnix=false
else
    gameName="${projectName}"
fi

for argument in "$@"
do
    if [[ "$argument" == "nobuild" ]]; then
        build=false
    elif [[ "$argument" == "release" ]]; then
        isDebugBuild=false
    elif [[ "$argument" == "run" ]]; then
        runAfterBuild=true
    elif [[ "$argument" == "clean" ]]; then
        cleanBuild=true
    fi
done

sync_directories() {
    local dirs=("EditorContent" "Config")
    local destinations=("Output/EditorContent" "Output/Config")

    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        gameName="${projectName}.exe"
        isUnix=false
    else
        gameName="${projectName}"
    fi
    
    for i in "${!dirs[@]}"; do
        local src="../${dirs[i]}"
        local dest="./${destinations[i]}"

        if [ -d "$src" ]; then
            mkdir -p "$dest"
            cp -auv "$src/"* "$dest/"
        else
            echo "Warning: Source directory $src not found."
        fi
    done
}

# Function to inject the default startup project for Visual Studio (Only runs on Windows)
set_startup_project() {
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        if ! grep -q "VS_STARTUP_PROJECT" "../CMakeLists.txt"; then
            echo "Configuring ${projectName} as default Visual Studio startup project..."
            echo -e "\n# Force Visual Studio Startup Project" >> "../CMakeLists.txt"
            echo "set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT \"${projectName}\")" >> "../CMakeLists.txt"
        fi
    fi
}

if [ "$isDebugBuild" = true ]; then
    directoryName="Build_Debug"
    configName="Debug"
else
    directoryName="Build_Release"
    configName="Release"
fi

# Clean up binaries based on the exact gameName for the current OS
rm -f "$directoryName/$configName/$gameName"
rm -f "$directoryName/$gameName"

if [ "$cleanBuild" = true ]; then
    rm -rf "$directoryName"
fi

mkdir -p "$directoryName" && cd "$directoryName"
sync_directories
echo "Files are synced."

set_startup_project

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    # pwd -W forces Git Bash/MinGW to return a C:/ style Windows path
    BUILD_PATH=$(pwd -W)
    SOURCE_PATH=$(cd .. && pwd -W)
else
    # Standard Unix paths for Linux/macOS
    BUILD_PATH="$PWD"
    SOURCE_PATH=$(cd .. && pwd)
fi

cmake -S "$SOURCE_PATH" -B "$BUILD_PATH" -DCMAKE_BUILD_TYPE=$configName

if [ "$build" = true ]; then
    cmake --build "$BUILD_PATH" --config $configName --parallel 8
    echo ""
    read -p "Compilation finished. Press [Enter] to continue..."
fi

if [ "$runAfterBuild" = true ]; then
    # Check standard MSVC subfolder first, then fallback to root build folder (Linux/Make)
    if [ -f "./$configName/$gameName" ]; then
        echo "Running $gameName from $configName folder..."
        "./$configName/$gameName"
    elif [ -f "./$gameName" ]; then
        echo "Running $gameName from root build folder..."
        "./$gameName"
    else
        echo "Error: Could not find the executable for $gameName."
        echo "Build may have failed."
    fi
fi