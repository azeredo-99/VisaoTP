# Nome do executável final
TARGET = resistencias

# Diretórios
SRCDIR = src
INCDIR = include
VIDDIR = video

# Arquivos fonte e objeto
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(SRC:.cpp=.o)

# Flags de compilação e linking
CXX = g++
CXXFLAGS = -I$(INCDIR) -std=c++11
LDFLAGS = `pkg-config --cflags --libs opencv4`

# Regra principal
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Regra para compilar arquivos fonte em objetos
%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# Limpeza dos arquivos compilados
clean:
	rm -f $(OBJ) $(TARGET)

# Regra para executar o programa
run: $(TARGET)
	./$(TARGET) $(VIDDIR)/video_resistors.mp4
