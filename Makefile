ORBITER_PATH=/home/gondos/dev/orbiter
INSTALL_PATH=/home/gondos/orbiter_test

#NOWARN=-fsanitize=address -Werror
ORBITER_SDK_INC=$(ORBITER_PATH)/Orbitersdk/include
SCNEDITOR_PATH=$(ORBITER_PATH)/Src/Plugin/ScnEditor
ORBITER_SDK_LIB=$(ORBITER_PATH)/Src/Orbitersdk/Orbitersdk.o

FRAMEWORK_PATH=XRVessels/framework/framework
XR1_LIB_PATH=XRVessels/DeltaGliderXR1/XR1Lib
XR2_PATH=XRVessels/XR2Ravenstar/XR2Ravenstar
XR5_PATH=XRVessels/XR5Vanguard/XR5Vanguard
XR1_PATH=XRVessels/DeltaGliderXR1/DeltaGliderXR1

XR2_SRC=$(wildcard $(XR2_PATH)/*.cpp)
XR2_OBJ=$(foreach src, $(XR2_SRC), $(src:.cpp=.o))

XR5_SRC=$(wildcard $(XR5_PATH)/*.cpp)
XR5_OBJ=$(foreach src, $(XR5_SRC), $(src:.cpp=.o))

XR1_LIB_SRC=$(wildcard $(XR1_LIB_PATH)/*.cpp)
XR1_LIB_OBJ=$(foreach src, $(XR1_LIB_SRC), $(src:.cpp=.o))

XR1_SRC=$(wildcard $(XR1_PATH)/*.cpp)
XR1_OBJ=$(foreach src, $(XR1_SRC), $(src:.cpp=.o))

FRAMEWORK_SRC=$(wildcard $(FRAMEWORK_PATH)/*.cpp)
FRAMEWORK_OBJ=$(foreach src, $(FRAMEWORK_SRC), $(src:.cpp=.o))

INCLUDES=-I$(XR1_LIB_PATH) -I$(FRAMEWORK_PATH) -I$(ORBITER_PATH)/Sound/XRSound/XRSound/src -I$(ORBITER_PATH)/Extern

%.o: %.cpp
#	$(CXX) -fsanitize=undefined -Wall $(NOWARN) -g -c -std=c++17  $(INCLUDES) $(CXXFLAGS) -fPIC -o $@ $<
	$(CXX) -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-delete-non-virtual-dtor -Wno-unused-variable -Wno-unused-function -Wno-reorder -Wno-unused-but-set-variable $(NOWARN) -g -c -std=c++17  $(INCLUDES) $(CXXFLAGS) -fPIC -o $@ $<

$(XR1_LIB_OBJ): INCLUDES+=-I$(XR1_LIB_PATH) -I$(XR1_PATH) -I$(ORBITER_SDK_INC) -I$(ORBITER_PATH)/Src/Orbiter -I$(XR2_PATH)
$(XR1_OBJ): INCLUDES+=-I$(XR1_PATH) -I$(ORBITER_SDK_INC) -I$(ORBITER_PATH)/Src/Orbiter -I$(XR1_PATH)
$(XR2_OBJ): INCLUDES+=-I$(XR2_PATH) -I$(ORBITER_SDK_INC) -I$(ORBITER_PATH)/Src/Orbiter -I$(XR1_PATH)
$(XR5_OBJ): INCLUDES+=-I$(XR5_PATH) -I$(ORBITER_SDK_INC) -I$(ORBITER_PATH)/Src/Orbiter -I$(XR1_PATH)
$(FRAMEWORK_OBJ): INCLUDES+=-I$(XR2_PATH) -I$(ORBITER_SDK_INC) -I$(ORBITER_PATH)/Src/Orbiter

$(XR1_PATH)/libDeltaGliderXR1.so: $(XR1_OBJ) $(XR1_LIB_OBJ) $(FRAMEWORK_OBJ) $(ORBITER_SDK_LIB)
	$(CXX) -g -std=c++17 -fPIC -g -shared -Wl,-soname,libDeltaGliderXR1.so -o $@ $^ -L$(ORBITER_PATH)/Sound/XRSound/XRSound/src/ -lXRSound -Wl,-rpath='$$ORIGIN:$$ORIGIN/Plugin'

$(XR2_PATH)/libXR2Ravenstar.so: $(XR2_OBJ) $(XR1_LIB_OBJ) $(FRAMEWORK_OBJ) $(ORBITER_SDK_LIB)
	$(CXX) -g -std=c++17 -fPIC -g -shared -Wl,-soname,libXR2Ravenstar.so -o $@ $^ -L$(ORBITER_PATH)/Sound/XRSound/XRSound/src/ -lXRSound -Wl,-rpath='$$ORIGIN:$$ORIGIN/Plugin'

$(XR5_PATH)/libXR5Vanguard.so: $(XR5_OBJ) $(XR1_LIB_OBJ) $(FRAMEWORK_OBJ) $(ORBITER_SDK_LIB)
	$(CXX) -g -std=c++17 -fPIC -g -shared -Wl,-soname,libXR5Vanguard.so -o $@ $^ -L$(ORBITER_PATH)/Sound/XRSound/XRSound/src/ -lXRSound -Wl,-rpath='$$ORIGIN:$$ORIGIN/Plugin'

install: $(XR2_PATH)/libXR2Ravenstar.so $(XR5_PATH)/libXR5Vanguard.so $(XR1_PATH)/libDeltaGliderXR1.so
	mkdir -p $(INSTALL_PATH)/Modules/
	mkdir -p $(INSTALL_PATH)/Config/
	cp $(XR2_PATH)/libXR2Ravenstar.so $(INSTALL_PATH)/Modules/
	cp -aR Orbiter/* $(INSTALL_PATH)/
	cp XRVessels/XR2Ravenstar/XR2RavenstarPrefs.cfg $(INSTALL_PATH)/Config/
	mkdir -p $(INSTALL_PATH)/Bitmaps/DeltaGliderXR1
	cp XRVessels/DeltaGliderXR1/DeltaGliderXR1/Bitmaps/* $(INSTALL_PATH)/Bitmaps/DeltaGliderXR1
	mkdir -p $(INSTALL_PATH)/Bitmaps/XR2Ravenstar
	cp XRVessels/XR2Ravenstar/XR2Ravenstar/Bitmaps/* $(INSTALL_PATH)/Bitmaps/XR2Ravenstar

	cp $(XR5_PATH)/libXR5Vanguard.so $(INSTALL_PATH)/Modules/
	cp XRVessels/XR5Vanguard/XR5VanguardPrefs.cfg $(INSTALL_PATH)/Config/
	mkdir -p $(INSTALL_PATH)/Bitmaps/XR5Vanguard
	cp XRVessels/XR5Vanguard/XR5Vanguard/Bitmaps/* $(INSTALL_PATH)/Bitmaps/XR5Vanguard

	cp $(XR1_PATH)/libDeltaGliderXR1.so $(INSTALL_PATH)/Modules/
	cp XRVessels/DeltaGliderXR1/DeltaGliderXR1Prefs.cfg $(INSTALL_PATH)/Config/
	mkdir -p $(INSTALL_PATH)/Bitmaps/DeltaGliderXR1
	cp XRVessels/DeltaGliderXR1/DeltaGliderXR1/Bitmaps/* $(INSTALL_PATH)/Bitmaps/DeltaGliderXR1

clean:
	find . -name *.o|xargs rm -f
	rm -f $(XR2_PATH)/libXR2Ravenstar.so $(XR5_PATH)/libXR5Vanguard.so $(XR1_PATH)/libDeltaGliderXR1.so
