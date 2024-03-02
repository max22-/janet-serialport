(declare-project :name "serialport")
(declare-native :name "serialport" :source @["serialport.c"] :lflags ["-lserialport"])