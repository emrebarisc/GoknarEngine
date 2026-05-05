#!/bin/bash

isDebugBuild=true
cleanBuild=false
build=true
runAfterBuild=false
isUnix=true
onlySyncFiles=false
publishBuild=false
publishOutputDir=""

syncedDirectories=("Content" "Config")
outputDirectories=("Output/Content" "Output/Config")

projectName=$(grep "^ProjectName=" Config/Build.ini | cut -d'=' -f2 | tr -d '\r' | tr -d ' ')

directoryName=""

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    gameName="${projectName}.exe"
    isUnix=false
else
    gameName="${projectName}"
fi

read_build_config_bool() {
    local key="$1"
    local value
    value=$(grep "^${key}=" Config/Build.ini | head -n 1 | cut -d'=' -f2- | tr -d '\r' | tr -d ' ')
    value="${value,,}"

    [[ "$value" == "1" || "$value" == "true" || "$value" == "yes" || "$value" == "on" ]]
}

if read_build_config_bool "Publish"; then
    publishBuild=true
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
    elif [[ "$argument" == "onlySync" ]]; then
        onlySyncFiles=true
    elif [[ "$argument" == "publish" ]]; then
        publishBuild=true
    elif [[ "$argument" == publishDir=* ]]; then
        publishOutputDir="${argument#publishDir=}"
    elif [[ "$argument" == outputDir=* ]]; then
        publishOutputDir="${argument#outputDir=}"
    fi
done

if [ "$publishBuild" = true ]; then
    isDebugBuild=false
fi

sync_directory_contents() {
    local src="$1"
    local dest="$2"

    if [ ! -d "$src" ]; then
        return 0
    fi

    mkdir -p "$dest"
    cp -au "$src"/. "$dest/"
}

sync_directories() {
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        gameName="${projectName}.exe"
        isUnix=false
    else
        gameName="${projectName}"
    fi
    
    for i in "${!syncedDirectories[@]}"; do
        local src="../${syncedDirectories[i]}"
        local dest="./${outputDirectories[i]}"

        if [ -d "$src" ]; then
            sync_directory_contents "$src" "$dest"
        else
            echo "Warning: Source directory $src not found."
        fi
    done
}

set_startup_project() {
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        if ! grep -q "VS_STARTUP_PROJECT" "../CMakeLists.txt"; then
            echo "Configuring ${projectName} as default Visual Studio startup project..."
            echo -e "\n# Force Visual Studio Startup Project" >> "../CMakeLists.txt"
            echo "set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT \"${projectName}\")" >> "../CMakeLists.txt"
        fi
    fi
}

stage_directory_for_publish() {
    local directoryToPackage="$1"
    local stagedDirectory="$2"

    rm -rf "$stagedDirectory"
    mkdir -p "$stagedDirectory"

    sync_directory_contents "../${directoryToPackage}" "$stagedDirectory"
    sync_directory_contents "./Output/${directoryToPackage}" "$stagedDirectory"

    if [ "$directoryName" != "Build_Debug" ]; then
        sync_directory_contents "../Build_Debug/Output/${directoryToPackage}" "$stagedDirectory"
    fi

    if [ "$directoryName" != "Build_Release" ]; then
        sync_directory_contents "../Build_Release/Output/${directoryToPackage}" "$stagedDirectory"
    fi
}

package_published_build() {
    local pakToolName="GoknarPakTool"
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        pakToolName="${pakToolName}.exe"
    fi

    local pakToolPath="${BUILD_PATH}/Output/${pakToolName}"
    if [ ! -f "$pakToolPath" ]; then
        echo "Error: Could not find ${pakToolName} at ${pakToolPath}."
        return 1
    fi

    if [ -z "$publishOutputDir" ]; then
        publishOutputDir="${BUILD_PATH}/Published"
    fi

    mkdir -p "$publishOutputDir"

    if compgen -G "./Output/*" > /dev/null; then
        cp -auv ./Output/* "$publishOutputDir/"
    fi

    local stagingRoot="${publishOutputDir}/.PakStaging"
    rm -rf "$stagingRoot"
    mkdir -p "$stagingRoot"

    for directoryToPackage in "${syncedDirectories[@]}"; do
        local stagedSourceDirectory="${stagingRoot}/${directoryToPackage}"
        stage_directory_for_publish "$directoryToPackage" "$stagedSourceDirectory"
        if ! compgen -G "$stagedSourceDirectory/*" > /dev/null; then
            continue
        fi

        local absoluteSourceDirectory
        absoluteSourceDirectory=$(cd "$stagedSourceDirectory" && pwd)
        (
            cd "$publishOutputDir" || exit 1
            "$pakToolPath" pack "$absoluteSourceDirectory" "./${directoryToPackage}.pak" "${directoryToPackage}/"
        ) || {
            rm -rf "$stagingRoot"
            return 1
        }
        rm -rf "$publishOutputDir/${directoryToPackage}"
    done

    rm -rf "$stagingRoot"

    echo "Published build is ready at ${publishOutputDir}"
    return 0
}

run_game_from_directory() {
    local executableDirectory="$1"

    if [ -f "${executableDirectory}/${gameName}" ]; then
        echo "Running $gameName from ${executableDirectory}..."
        (
            cd "${executableDirectory}" || exit 1
            "./$gameName"
        )
        return 0
    fi

    return 1
}

if [ "$isDebugBuild" = true ]; then
    directoryName="Build_Debug"
    configName="Debug"
else
    directoryName="Build_Release"
    configName="Release"
fi

if [ "$cleanBuild" = true ]; then
    rm -rf "$directoryName"
fi

mkdir -p "$directoryName" && cd "$directoryName"
sync_directories
echo "Files are synced."

if [ "$onlySyncFiles" = false ]; then
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

    if [ "$publishBuild" = true ]; then
        publishCMakeOption="ON"
    else
        publishCMakeOption="OFF"
    fi

    cmake -S "$SOURCE_PATH" -B "$BUILD_PATH" -DCMAKE_BUILD_TYPE=$configName -DGOKNAR_PUBLISHED_BUILD=$publishCMakeOption

    if [ "$build" = true ]; then
        cmake --build "$BUILD_PATH" --config $configName --parallel 8
        echo ""
        read -p "Compilation finished. Press [Enter] to continue..."
    fi

    if [ "$publishBuild" = true ]; then
        package_published_build || exit 1
    fi

    if [ "$runAfterBuild" = true ]; then
        if [ "$publishBuild" = true ] && run_game_from_directory "$publishOutputDir"; then
            exit 0
        fi

        # Prefer the Output folder since CMake routes all runtime binaries there.
        if run_game_from_directory "./Output"; then
            exit 0
        elif [ -f "./$gameName" ]; then
            echo "Running $gameName from root build folder..."
            "./$gameName"
        else
            echo "Error: Could not find the executable for $gameName."
            echo "Expected one of: ./$gameName, or ./Output/$gameName."
            echo "Build may have failed."
        fi
    fi
fi
