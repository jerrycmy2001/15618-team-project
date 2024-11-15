# Compiler settings
CXX = g++ -m64
NVCC = nvcc
CXXFLAGS=-O3 -Wall -g
NVCCFLAGS=-O3 -m64
LIBS = GL glut cudart
LDLIBS  := $(addprefix -l, $(LIBS))

# Directories
SRC_DIRS = renderer scene utils include 3d_objects camera
OBJ_DIR = objs

CXXFLAGS += $(addprefix -I, $(SRC_DIRS))
NVCCFLAGS += $(addprefix -I, $(SRC_DIRS))

# Find all .cpp and .cu files in the source directories
CPP_FILES := $(shell find $(SRC_DIRS) -name '*.cpp')
CU_FILES := $(shell find $(SRC_DIRS) -name '*.cu')

# Generate object files list with objs/ prefix
CPP_OBJ_FILES := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(CPP_FILES))
CU_OBJ_FILES := $(patsubst %.cu, $(OBJ_DIR)/%.o, $(CU_FILES))
OBJ_FILES := $(CPP_OBJ_FILES) $(CU_OBJ_FILES)

# Target executable
TARGET = render

# Main target rule: link all object files into the final executable
$(TARGET): $(OBJ_FILES)
	$(NVCC) $(OBJ_FILES) -o $(TARGET) $(LDLIBS)

# Rule to compile .cpp files
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)  # Create any required subdirectories
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile .cu files
$(OBJ_DIR)/%.o: %.cu | $(OBJ_DIR)
	@mkdir -p $(dir $@)  # Create any required subdirectories
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

# Rule to create the object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean rule to remove object files and the target executable
clean:
	rm -rf $(OBJ_DIR) $(TARGET)