# specify location of libraries
BOOST = /usr/include/boost
EIGEN = /usr/include/eigen
BSO = ..
ALL_LIB = -I$(BOOST) -I$(EIGEN) -I$(BSO)

# compiler settings
CPP = g++ -std=c++14
FLAGS = -O3 -march=native -lglut -lGL -lGLU -lpthread

# specify file(s) to be compiled
BSDFILE  = generateBSDs_ML.cpp
DATAFILE = generateDataset_ML.cpp
VIMLFILE = visualisation_ML.cpp

GENETICF = GA1and2.cpp
VIGAFILE = visualisation_GA.cpp

# specify name of executables
EXE = bsd
EXE = dataset
EXE = visualML
EXE = algo
EXE = visualGA

.PHONY: all clean

# definition of arguments for make command
# MACHINE LEARNING
design:
	$(CPP) -o bsd $(ALL_LIB) $(BSDFILE) $(FLAGS)
data:
	$(CPP) -o dataset $(ALL_LIB) $(DATAFILE) $(FLAGS)
viml:
	$(CPP) -o visualML $(ALL_LIB) $(VIMLFILE) $(FLAGS)

# GENETIC ALGORITHM
ga:
	$(CPP) -o algo $(ALL_LIB) $(GENETICF) $(FLAGS)
viga:
	$(CPP) -o visualGA $(ALL_LIB) $(VIGAFILE) $(FLAGS)



# remove previously compiled executable
clean:
	@rm -f $(EXE)
