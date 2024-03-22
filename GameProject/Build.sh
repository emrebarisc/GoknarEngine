isDebugBuild=false
cleanBuild=false
runAfterBuild=false
directoryName=""
gameName="GameProject"

for argument in "$@"
do
	if [[ "$argument" == "debug" ]]; then
		isDebugBuild=true
	else
		if [[ "$argument" == "run" ]]; then
			runAfterBuild=true
		else
			if [[ "$argument" == "clean" ]]; then
				cleanBuild=true
			fi
		fi
	fi
done
if [ "$isDebugBuild" = true ]; then
	directoryName="Build_Debug"
	rm $directoryName/$gameName

	if [ "$cleanBuild" = true ]; then
		rm -rf $directoryName && mkdir $directoryName && cd $directoryName && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j 8
	else
		if ! [ -d $directoryName ]; then
			mkdir $directoryName
		fi
		cd $directoryName && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j 8
	fi
else
	directoryName="Build_Release"
	rm $directoryName/$gameName

	if [ "$cleanBuild" = true ]; then
		rm -rf $directoryName && mkdir $directoryName && cd $directoryName && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j 8
	else
		if ! [ -d $directoryName ]; then
			mkdir $directoryName
		fi
		cd $directoryName && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j 8
	fi
fi
if [ "$runAfterBuild" = true ]; then
	./$gameName
fi