# Koso2_M5Stack

Koso2_M5Stackは[ROSではじめるホビーロボット番外編](https://koso2-dan.booth.pm/items/2026421)で紹介する[ros2arduino](https://github.com/ROBOTIS-GIT/ros2arduino)とM5StackでROS2の学習を行うためのサンプルコードです。

## 動作環境
ArduinoIDE 1.8.12  
ros2arduino 0.1.4

ROS2 Dashing Diademata  
Ubuntu 18.04  

## インストール方法
ArduinoIDEのsketchフォルダへcloneすると、スケッチメニューから選択できるようになります。サンプルの無線LAN設定値には実行環境の値を入力して下さい。

```
$ mkdir -p ~/Arduino/sketch
$ cd ~/Arduino/sketch
~/Arduino/sketch$ git clone https://github.com/nomumu/Koso2_M5Stack.git
```

このスケッチをコンパイルするにはros2arduinoライブラリを編集する必要があります。ファイルを用意してあるので、次のコマンドを実行して下さい。
```
$ cd ~/Arduino/sketch/Koso2_M5Stack/msg_files
~/Arduino/sketch/Koso2_M5Stack/msg_files$ patch ~/Arduino/libraries/ros2arduino/src/ros2/topic_id_number.h < topic_id_number.h.patch
~/Arduino/sketch/Koso2_M5Stack/msg_files$ patch ~/Arduino/libraries/ros2arduino/src/ros2/msg_list.hpp < msg_list.hpp.patch 
~/Arduino/sketch/Koso2_M5Stack/msg_files$ cp m5stack_msgs ~/Arduino/libraries/ros2arduino/src/ros2/ -ar
```

## 実行方法
スケッチをM5Stackへ転送して実行します。
