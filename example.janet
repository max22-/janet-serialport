(import ./build/serialport :as sp)
(pp (sp/list-ports))
(pp (sp/port-info "/dev/ttyACM0"))