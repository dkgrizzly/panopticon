setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
loadProjectFile -file "/home/grizzly/pano_boot/xilinx/pano_boot.ipf"
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
setMode -bs
attachflash -position 1 -spi "M25P80"
setMode -bs
setMode -bs
setMode -bs
setMode -bs
setCable -port ttyS0 -baud -1
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx/pano_boot.ipf"
attachflash -position 1 -spi "M25P80"
assignfiletoattachedflash -position 1 -file "/home/grizzly/pano_boot/fw/firmware/firmware.mcs"
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx/pano_boot.ipf"
Program -p 1 -dataWidth 1 -spionly -e -v 
Program -p 1 -spionly -e -v 
attachflash -position 1 -spi "M25P80"
assignfiletoattachedflash -position 1 -file "/home/grizzly/pano_boot/fw/firmware/firmware.mcs"
Program -p 1 -dataWidth 1 -spionly -e -v 
Program -p 1 -spionly -e -v 
Program -p 1 -spionly -e -v 
Program -p 1 -spionly -e -v 
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx/pano_boot.ipf"
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx//auto_project.ipf"
setMode -bs
deleteDevice -position 1
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
loadProjectFile -file "/home/grizzly/pano_boot/xilinx/pano_boot.ipf"
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
attachflash -position 1 -spi "M25P80"
setMode -bs
setMode -bs
setMode -bs
setMode -bs
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx//auto_project.ipf"
setMode -bs
deleteDevice -position 1
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
loadProjectFile -file "/home/grizzly/pano_boot/xilinx/auto_project.ipf"
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
attachflash -position 1 -spi "M25P80"
setMode -bs
setMode -bs
setMode -bs
setMode -bs
Program -p 1 -spionly -e -v 
attachflash -position 1 -spi "M25PE80"
assignfiletoattachedflash -position 1 -file "/home/grizzly/pano_boot/fw/firmware/firmware.mcs"
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx/auto_project.ipf"
Program -p 1 -dataWidth 1 -spionly -e -v 
Erase -p 1 -spionly 
BlankCheck -p 1 -spionly 
Erase -p 1 -spionly 
Erase -p 1 -spionly 
Erase -p 1 -spionly 
Erase -p 1 -spionly 
Program -p 1 -spionly -e 
Erase -p 1 -spionly 
Erase -p 1 -spionly 
ReadIdcode -p 1 
ReadbackToFile -p 1 -file ".mcs" -spionly 
Checksum -p 1 -spionly 
attachflash -position 1 -spi "M25P80"
assignfiletoattachedflash -position 1 -file "/home/grizzly/pano_boot/fw/firmware/firmware.mcs"
Erase -p 1 -spionly 
Program -p 1 -dataWidth 1 -spionly -e -v 
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx/auto_project.ipf"
Erase -p 1 -spionly 
Program -p 1 -spionly -e -v 
attachflash -position 1 -spi "M25P80"
assignfiletoattachedflash -position 1 -file "/home/grizzly/pano_boot/fw/firmware/firmware.mcs"
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx/auto_project.ipf"
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/home/grizzly/pano_boot/xilinx//auto_project.ipf"
setMode -bs
setMode -bs
deleteDevice -position 1
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
