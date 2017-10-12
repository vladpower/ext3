На вход данной программы подается путь к файлу образа диска и смещение (в секторах).
Компиляция:
mkdir build
cd build
cmake ..
make
Запуск:
./FileIdentifier ../drive_images/test_img_ext3 <Номер сектора>
