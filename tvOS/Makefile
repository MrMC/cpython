# This is a "Meta-makefile" for building an tvOS-compatible Python.framework.
# It invokes the base makefile multiple times, once for each hardware platform
# that needs to be supported; then it combines the products into a single "fat"
# binary.

TVOS_DIR=$(shell pwd)

# Extract the Python version from the master configure.ac
PYTHON_VERSION=$(shell grep "m4_define.PYTHON_VERSION," ../configure.ac | sed "s/m4_define(PYTHON_VERSION, \(.*\))/\1/")

# Build identifier of the build OS.
BUILD_OS_ID=x86_64-apple-darwin$(shell uname -r)

# 64 bit tvOS Simulator build commands and flags
TVOS_SIMULATOR_64_SDK_ROOT=$(shell xcrun --sdk appletvsimulator --show-sdk-path)
TVOS_SIMULATOR_64_CC=$(shell xcrun -find -sdk appletvsimulator clang) -arch x86_64 --sysroot=$(TVOS_SIMULATOR_64_SDK_ROOT) -mtvos-version-min=9.0
TVOS_SIMULATOR_64_LD=$(shell xcrun -find -sdk appletvsimulator ld) -arch x86_64 --sysroot=$(TVOS_SIMULATOR_64_SDK_ROOT) -mtvos-version-min=9.0

# tvOS ARM64 build commands and flags
TVOS_ARM64_SDK_ROOT=$(shell xcrun --sdk appletvos --show-sdk-path)
TVOS_ARM64_CC=$(shell xcrun -find -sdk appletvos clang) -arch arm64 --sysroot=$(TVOS_ARM64_SDK_ROOT) -mtvos-version-min=9.0
TVOS_ARM64_LD=$(shell xcrun -find -sdk appletvos ld) -arch arm64 --sysroot=$(TVOS_ARM64_SDK_ROOT) -mtvos-version-min=9.0


.PHONY: libs test all clean distclean _framework

all: libs test

libs: ../Python.framework

test: ../Tools/tvOS-test/Python.framework

clean:
	rm -rf build
	rm -rf ../Python.framework
	rm -rf ../Tools/tvOS-test/Python.framework

distclean: clean
	rm -rf XCode-sample/sample.xcodeproj/project.xcworkspace
	rm -rf XCode-sample/sample.xcodeproj/xcuserdata
	rm -rf XCode-sample/Python.framework
	rm -rf ../Tools/tvOS-test/sample.xcodeproj/project.xcworkspace
	rm -rf ../Tools/tvOS-test/sample.xcodeproj/xcuserdata
	rm -rf ../Tools/tvOS-test/Python.framework

build/host/bin/python$(PYTHON_VERSION):
	# Configure and make the local build, providing compiled resources.
	if test -f ../Makefile; then cd .. && make distclean; fi
	cd .. && ./configure --prefix=$(TVOS_DIR)/build/host --without-ensurepip
	cd .. && make
	cd .. && make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf host.tar.gz host && rm -rf host

build/tvOS-simulator-x86_64.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build Simulator library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(TVOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=x86_64-apple-tvos --build=$(BUILD_OS_ID) \
			CC="$(TVOS_SIMULATOR_64_CC)" LD="$(TVOS_SIMULATOR_64_LD)" \
			--prefix=$(TVOS_DIR)/build/tvOS-simulator-x86_64 \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(TVOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(TVOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf tvOS-simulator-x86_64.tar.gz tvOS-simulator-x86_64 && rm -rf tvOS-simulator-x86_64

build/tvOS-arm64.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build ARM64 library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(TVOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=aarch64-apple-tvos --build=$(BUILD_OS_ID) \
			CC="$(TVOS_ARM64_CC)" LD="$(TVOS_ARM64_LD)" \
			--prefix=$(TVOS_DIR)/build/tvOS-arm64 \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(TVOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(TVOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf tvOS-arm64.tar.gz tvOS-arm64 && rm -rf tvOS-arm64

../Python.framework: build/tvOS-simulator-x86_64.tar.gz build/tvOS-arm64.tar.gz
	FRAMEWORK_DIR=../Python.framework PRUNED=yes $(MAKE) _framework

../Tools/tvOS-test/Python.framework: build/tvOS-simulator-x86_64.tar.gz build/tvOS-arm64.tar.gz
	FRAMEWORK_DIR=../Tools/tvOS-test/Python.framework $(MAKE) _framework

##########################################################################
# A meta-task for building an tvOS framework.
#
# The test system framework and the production framework are very similar,
# but the production framework can be significantly pruned and compressed.
# However, other than that detail, the process of creating the respective
# frameworks is identical.
#
# This meta target allows you to invoke a recursive make call with a
# _framework target. The framework will be built in the $(FRAMEWORK_DIR)
# directory; if $(PRUNED) is defined, it will be pruned and compressed.
##########################################################################
_framework:
	# Unpack the archived install directories
	cd build && tar zxf tvOS-simulator-x86_64.tar.gz
	cd build && tar zxf tvOS-arm64.tar.gz

	# Create the framework directory
	mkdir -p $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources

	# Copy the headers. The headers are the same for every platform, except for pyconfig.h;
	# use the x86_64 simulator build because reasons.
	cp -r build/tvOS-simulator-x86_64/include/python$(PYTHON_VERSION)m $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers

	# The only headers that change between versions is pyconfig.h; copy each supported version...
	cp build/tvOS-simulator-x86_64/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-x86_64.h
	cp build/tvOS-arm64/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-arm64.h
	# ... and then copy in a master pyconfig.h to unify them all.
	cp include/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig.h

	# Copy the standard library from the simulator build. Again, the
	# pure Python standard library is the same on every platform;
	# use the simulator version because reasons.
	mkdir -p $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources
	cp -r build/tvOS-simulator-x86_64/lib $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources

	# Copy Python.h and pyconfig.h into the resources include directory
	mkdir -p $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources/include/python$(PYTHON_VERSION)m
	cp -r $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig*.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources/include/python$(PYTHON_VERSION)m
	cp -r $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/Python.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources/include/python$(PYTHON_VERSION)m

	# Remove the pieces of the resources directory that aren't needed:
	# libpython.a isn't needed in the lib directory
	rm -f $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources/lib/libpython$(PYTHON_VERSION)m.a
	# pkgconfig isn't needed on the device
	rm -rf $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources/lib/pkgconfig

	# If a pruned framework has been requested, do the pruning and compression.
	if test -n "$(PRUNED)"; then \
		cd $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources/lib/python$(PYTHON_VERSION); \
		rm -rf *test* lib* bsddb curses ensurepip hotshot idlelib tkinter turtledemo wsgiref \
			config-$(PYTHON_VERSION) ctypes/test distutils/tests site-packages sqlite3/test; \
		find . -name "*.pyc" -exec rm -rf {} \; ; \
		zip -r ../python$(subst .,,$(PYTHON_VERSION)).zip *; \
		rm -rf *; \
	fi

	# Create a fat binary for the libPython library
	xcrun lipo -create -output \
		$(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Python \
		build/tvOS-simulator-x86_64/lib/libpython$(PYTHON_VERSION)m.a \
		build/tvOS-arm64/lib/libpython$(PYTHON_VERSION)m.a \
		;

	# Link the current headers, Resources and Python version
	cd $(FRAMEWORK_DIR)/Versions && ln -fs $(PYTHON_VERSION) Current
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Headers
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Resources
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Python

	# Create the Info.plist file
	sed -e "s/xxxVERSIONxxx/$(PYTHON_VERSION)/g" Info.plist > $(FRAMEWORK_DIR)/Resources/Info.plist
