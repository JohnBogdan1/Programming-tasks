PACKAGE=visualiser.

LIB=lib/
SRC=src/
BIN=bin/

CLASSPATH=.:$(LIB)junit-4.12.jar:$(LIB)guava-18.0.jar:$(LIB)hamcrest-integration-1.3.jar:$(LIB)hamcrest-library-1.3.jar:$(LIB)hamcrest-core-1.3.jar:$(SRC):$(BIN)

build:
	javac -g -d $(BIN) -cp $(CLASSPATH) $(SRC)**/*.java

test: build
	java -cp $(CLASSPATH) org.junit.runner.JUnitCore $(PACKAGE)TestTask
	java -cp $(CLASSPATH) org.junit.runner.JUnitCore $(PACKAGE)TestClusterManager
