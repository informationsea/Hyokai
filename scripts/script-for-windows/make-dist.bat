mkdir Hyokai
copy License.txt Hyokai\
copy README.md Hyokai\README.txt
copy release\Hyokai.exe Hyokai\
mkdir Hyokai\sampledata
copy ..\..\sampledata\iris.data.sqlite3 Hyokai\sampledata
copy ..\..\sampledata\iris.data.csv Hyokai\sampledata

del Hyokai.zip

pushd Hyokai
"\Program Files\7-Zip\7z.exe" a -tzip ..\Hyokai.zip *
popd

