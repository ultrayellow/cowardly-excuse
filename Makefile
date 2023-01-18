## ******************************* ##
##            @COPYLEFT            ##
##       ALL WRONGS RESERVED       ##
## ******************************* ##

TARGET = cowircd.out
OBJECTS_DIR = objs/
CXXFLAGS += -Ishared_ptr

CXXFLAGS += -Wall -Wextra -Werror -std=c++98 -pedantic

SOURCES = $(wildcard *.cpp)
OBJECTS = $(addprefix $(OBJECTS_DIR), $(SOURCES:.cpp=.o))

CXXFLAGS += -fsanitize=address -g3

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(OBJECTS_DIR):
	mkdir $(OBJECTS_DIR)

$(OBJECTS_DIR)%.o: %.cpp | $(OBJECTS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MF $(@:.o=.d) -MT $@ -MP

.PHONY: clean
clean:
	rm -rf $(OBJECTS_DIR)

-include $(OBJECTS:.o=.d)
