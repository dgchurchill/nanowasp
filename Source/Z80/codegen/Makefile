all: opcodes

mktables: mktables.c
	gcc -g -o mktables mktables.c
	
force: clean opcodes
	
opcodes: mktables opcodes.lst mktables.spec
	./mktables
	cat opcodes_impl.c | grep "void Z80CPU" | sed "s/)/);/g" | sed "s/Z80CPU:://g" >opcodes_decl.h
	cat opcodes_table.c | grep "Z80OpcodeTable" | sed "s/ =.*/;/" | sed "s/^/static /" | sed "s/Z80CPU:://g" > opcodes_table_decl.h
	
clean:
	rm -f mktables.exe opcodes_table.c opcodes_table_decl.h opcodes_impl.c opcodes_decl.h
