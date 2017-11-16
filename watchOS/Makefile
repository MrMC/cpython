# This is a "Meta-makefile" for building an watchOS-compatible Python.framework.
# It invokes the base makefile multiple times, once for each hardware platform
# that needs to be supported; then it combines the products into a single "fat"
# binary.

WATCHOS_DIR=$(shell pwd)

# Extract the Python version from the master configure.ac
PYTHON_VERSION=$(shell grep "m4_define.PYTHON_VERSION," ../configure.ac | sed "s/m4_define(PYTHON_VERSION, \(.*\))/\1/")

# Build identifier of the build OS.
BUILD_OS_ID=x86_64-apple-darwin$(shell uname -r)

# watchOS Simulator build commands and flags
WATCHOS_SIMULATOR_SDK_ROOT=$(shell xcrun --sdk watchsimulator --show-sdk-path)
WATCHOS_SIMULATOR_CC=$(shell xcrun -find -sdk watchsimulator clang) -arch i386 --sysroot=$(WATCHOS_SIMULATOR_SDK_ROOT) -mwatchos-version-min=2.0
WATCHOS_SIMULATOR_LD=$(shell xcrun -find -sdk watchsimulator ld) -arch i386 --sysroot=$(WATCHOS_SIMULATOR_SDK_ROOT) -mwatchos-version-min=2.0

# watchOS ARMv7k build commands and flags
WATCHOS_ARM7K_SDK_ROOT=$(shell xcrun --sdk watchos --show-sdk-path)
WATCHOS_ARM7K_CC=$(shell xcrun -find -sdk watchos clang) -arch armv7k --sysroot=$(WATCHOS_ARM7K_SDK_ROOT) -mwatchos-version-min=4.0
WATCHOS_ARM7K_LD=$(shell xcrun -find -sdk watchos ld) -arch armv7k --sysroot=$(WATCHOS_ARM7K_SDK_ROOT) -mwatchos-version-min=4.0


.PHONY: libs test all clean distclean _framework

all: libs test

libs: ../Python.framework

test: ../Tools/watchOS-test/Python.framework

clean:
	rm -rf build
	rm -rf ../Python.framework
	rm -rf ../Tools/watchOS-test/Python.framework

distclean: clean
	rm -rf XCode-sample/sample.xcodeproj/project.xcworkspace
	rm -rf XCode-sample/sample.xcodeproj/xcuserdata
	rm -rf XCode-sample/Python.framework
	rm -rf ../Tools/watchOS-test/sample.xcodeproj/project.xcworkspace
	rm -rf ../Tools/watchOS-test/sample.xcodeproj/xcuserdata
	rm -rf ../Tools/watchOS-test/Python.framework

build/host/bin/python$(PYTHON_VERSION):
	# Configure and make the local build, providing compiled resources.
	if test -f ../Makefile; then cd .. && make distclean; fi
	cd .. && ./configure --prefix=$(WATCHOS_DIR)/build/host --without-ensurepip
	cd .. && make
	cd .. && make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf host.tar.gz host && rm -rf host

build/watchOS-simulator-i386.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build Simulator library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(WATCHOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=i386-apple-watchos --build=$(BUILD_OS_ID) \
			CC="$(WATCHOS_SIMULATOR_CC)" LD="$(WATCHOS_SIMULATOR_LD)" \
			--prefix=$(WATCHOS_DIR)/build/watchOS-simulator-i386 \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(WATCHOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(WATCHOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf watchOS-simulator-i386.tar.gz watchOS-simulator-i386 && rm -rf watchOS-simulator-i386

build/watchOS-arm7k.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build ARM7K library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(WATCHOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=armv7k-apple-watchos --build=$(BUILD_OS_ID) \
			CC="$(WATCHOS_ARM7K_CC)" LD="$(WATCHOS_ARM7K_LD)" \
			--prefix=$(WATCHOS_DIR)/build/watchOS-arm7k \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(WATCHOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(WATCHOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf watchOS-arm7k.tar.gz watchOS-arm7k && rm -rf watchOS-arm7k

../Python.framework: build/watchOS-simulator-i386.tar.gz build/watchOS-arm7k.tar.gz
	FRAMEWORK_DIR=../Python.framework PRUNED=yes $(MAKE) _framework

../Tools/watchOS-test/Python.framework: build/watchOS-simulator-i386.tar.gz build/watchOS-arm7k.tar.gz
	FRAMEWORK_DIR=../Tools/watchOS-test/Python.framework $(MAKE) _framework

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
	cd build && tar zxf watchOS-simulator-i386.tar.gz
	cd build && tar zxf watchOS-arm7k.tar.gz

	# Create the framework directory
	mkdir -p $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources

	# Copy the headers. The headers are the same for every platform, except for pyconfig.h;
	# use the i386 simulator build because reasons.
	cp -r build/watchOS-simulator-i386/include/python$(PYTHON_VERSION)m $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers

	# The only headers that change between versions is pyconfig.h; copy each supported version...
	cp build/watchOS-simulator-i386/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-i386.h
	cp build/watchOS-arm7k/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-arm7k.h
	# ... and then copy in a master pyconfig.h to unify them all.
	cp include/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig.h

	# Copy the standard library from the simulator build. Again, the
	# pure Python standard library is the same on every platform;
	# use the simulator version because reasons.
	mkdir -p $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources
	cp -r build/watchOS-simulator-i386/lib $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources

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
		build/watchOS-simulator-i386/lib/libpython$(PYTHON_VERSION)m.a \
		build/watchOS-arm7k/lib/libpython$(PYTHON_VERSION)m.a \
		;

	# Link the current headers, Resources and Python version
	cd $(FRAMEWORK_DIR)/Versions && ln -fs $(PYTHON_VERSION) Current
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Headers
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Resources
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Python

	# Create the Info.plist file
	sed -e "s/xxxVERSIONxxx/$(PYTHON_VERSION)/g" Info.plist > $(FRAMEWORK_DIR)/Resources/Info.plist
