all:
	-cd ./commons/Debug && $(MAKE) clean && $(MAKE) all 
	-cd ./ansisop-parser/Debug && $(MAKE) clean && $(MAKE) all 
	-cd ./Programa/Debug && $(MAKE) clean && $(MAKE) all 
	-cd ./kernel/Debug && $(MAKE) clean && $(MAKE) all 
	-cd ./CPU/Debug && $(MAKE) clean && $(MAKE) all 
	-cd ./UMV/Debug && $(MAKE) clean && $(MAKE) all 
