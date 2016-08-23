del /S /Q Hyokai
rmdir /Q /S Hyokai

mkdir Hyokai
copy License.txt Hyokai\
copy dist-README.txt Hyokai\README.txt
robocopy /S build32\deploy Hyokai\Hyokai32
robocopy /S build64\deploy Hyokai\Hyokai64
mkdir Hyokai\sampledata
copy ..\..\sampledata\iris.data.sqlite3 Hyokai\sampledata
copy ..\..\sampledata\iris.data.csv Hyokai\sampledata

del Hyokai.zip

"\Program Files\7-Zip\7z.exe" a -tzip .\Hyokai.zip Hyokai

