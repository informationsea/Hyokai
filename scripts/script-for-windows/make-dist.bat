del /S /Q Hyokai
mkdir Hyokai
copy License.txt Hyokai\
copy dist-README.txt Hyokai\README.txt
copy release\Hyokai_x86.exe Hyokai\
copy release\Hyokai_x64.exe Hyokai\
mkdir Hyokai\sampledata
copy ..\..\sampledata\iris.data.sqlite3 Hyokai\sampledata
copy ..\..\sampledata\iris.data.csv Hyokai\sampledata
copy vcredist_x64.exe Hyokai\
copy vcredist_x86.exe Hyokai\

del Hyokai.zip

"\Program Files\7-Zip\7z.exe" a -tzip .\Hyokai.zip Hyokai

