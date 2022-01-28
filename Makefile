
CC:=g++

#ExecFile:=npshell
ExecFile2_1:=np_simple
ExecFile2_2:=np_single_proc
ExecFile2_3:=np_multi_proc

#MainFile:=npshell.cpp
MainFile2_1:=np_simple.cpp
MainFile2_2:=np_single_proc.cpp
MainFile2_3:=np_multi_proc.cpp

OBJ2_1:=np_simple.o
OBJ2_2:=np_single_proc.o
OBJ2_3:=np_multi_proc.o




add:


	$(CC) -o $(ExecFile2_1) $(MainFile2_1)
	$(CC) -o $(ExecFile2_2) $(MainFile2_2)
	$(CC) -o $(ExecFile2_3) $(MainFile2_3)
%.o:%.cpp
	$(CC) -c $^ -o $@

.PHONY:clean
clean:
	rm *.o






######################
#EXE:=np_multi_proc
#OBJ:=np_multi_proc.o

#all:$(OBJ)
#	$(CC) -o $(EXE) $(OBJ)
#%.o:%.cpp
#	$(CC) -c $^ -o $@

#.PHONY:clean
#clean:
#	rm *.o
