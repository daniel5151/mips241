OBJDIR = .o/
DEPDIR = .d/

$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(DEPDIR))

src = $(wildcard *.cc)
obj = $(addprefix $(OBJDIR), $(src:.cc=.o))
# one dependency file for each source
dep = $(addprefix $(DEPDIR), $(src:.cc=.d))

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wno-dollar-in-identifier-extension -pedantic

mips241: $(obj)
	$(CXX) -o $@ $^

$(OBJDIR)%.o : %.cc
	$(CXX) $(CXXFLAGS) $< -c -o $@

# include all dep files in the makefile
-include $(dep)
# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
$(DEPDIR)%.d: %.cc
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -rf $(OBJDIR) mips241

.PHONY: cleandep
cleandep:
	rm -rf $(DEPDIR)

.PHONY: clean
cleaner:
	rm -rf $(DEPDIR)
	rm -rf $(OBJDIR) mips241