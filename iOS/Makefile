# This is a "Meta-makefile" for building an iOS-compatible Python.framework.
# It invokes the base makefile multiple times, once for each hardware platform
# that needs to be supported; then it combines the products into a single "fat"
# binary.

IOS_DIR=$(shell pwd)

# Extract the Python version from the master configure.ac
PYTHON_VERSION=$(shell grep "m4_define.PYTHON_VERSION," ../configure.ac | sed "s/m4_define(PYTHON_VERSION, \(.*\))/\1/")

# Build identifier of the build OS.
BUILD_OS_ID=x86_64-apple-darwin$(shell uname -r)

# 32 bit iOS Simulator build commands and flags
IOS_SIMULATOR_SDK_ROOT=$(shell xcrun --sdk iphonesimulator --show-sdk-path)
IOS_SIMULATOR_CC=$(shell xcrun -find -sdk iphonesimulator clang) -arch i386 -isysroot $(IOS_SIMULATOR_SDK_ROOT) -mios-version-min=7.0
IOS_SIMULATOR_LD=$(shell xcrun -find -sdk iphonesimulator ld) -arch i386 -isysroot $(IOS_SIMULATOR_SDK_ROOT) -mios-version-min=7.0

# 64 bit iOS Simulator build commands and flags
IOS_SIMULATOR_64_SDK_ROOT=$(shell xcrun --sdk iphonesimulator --show-sdk-path)
IOS_SIMULATOR_64_CC=$(shell xcrun -find -sdk iphonesimulator clang) -arch x86_64 -isysroot $(IOS_SIMULATOR_64_SDK_ROOT) -mios-version-min=7.0
IOS_SIMULATOR_64_LD=$(shell xcrun -find -sdk iphonesimulator ld) -arch x86_64 -isysroot $(IOS_SIMULATOR_64_SDK_ROOT) -mios-version-min=7.0

# iOS ARMV7 build commands and flags
IOS_ARMV7_SDK_ROOT=$(shell xcrun --sdk iphoneos --show-sdk-path)
IOS_ARMV7_CC=$(shell xcrun -find -sdk iphoneos clang) -arch armv7 -isysroot $(IOS_ARMV7_SDK_ROOT) -mios-version-min=7.0
IOS_ARMV7_LD=$(shell xcrun -find -sdk iphoneos ld) -arch armv7 -isysroot $(IOS_ARMV7_SDK_ROOT) -mios-version-min=7.0

# iOS ARMV7S build commands and flags
IOS_ARMV7S_SDK_ROOT=$(shell xcrun --sdk iphoneos --show-sdk-path)
IOS_ARMV7S_CC=$(shell xcrun -find -sdk iphoneos clang) -arch armv7s -isysroot $(IOS_ARMV7S_SDK_ROOT) -mios-version-min=7.0
IOS_ARMV7S_LD=$(shell xcrun -find -sdk iphoneos ld) -arch armv7s -isysroot $(IOS_ARMV7S_SDK_ROOT) -mios-version-min=7.0

# iOS ARM64 build commands and flags
IOS_ARM64_SDK_ROOT=$(shell xcrun --sdk iphoneos --show-sdk-path)
IOS_ARM64_CC=$(shell xcrun -find -sdk iphoneos clang) -arch arm64 -isysroot $(IOS_ARM64_SDK_ROOT) -mios-version-min=7.0
IOS_ARM64_LD=$(shell xcrun -find -sdk iphoneos ld) -arch arm64 -isysroot $(IOS_ARM64_SDK_ROOT) -mios-version-min=7.0


.PHONY: libs test all clean distclean _framework

all: libs test

libs: ../Python.framework

test: ../Tools/iOS-test/Python.framework

clean:
	rm -rf build
	rm -rf ../Python.framework
	rm -rf ../Tools/iOS-test/Python.framework

distclean: clean
	rm -rf XCode-sample/sample.xcodeproj/project.xcworkspace
	rm -rf XCode-sample/sample.xcodeproj/xcuserdata
	rm -rf XCode-sample/Python.framework
	rm -rf ../Tools/iOS-test/sample.xcodeproj/project.xcworkspace
	rm -rf ../Tools/iOS-test/sample.xcodeproj/xcuserdata
	rm -rf ../Tools/iOS-test/Python.framework

build/host/bin/python$(PYTHON_VERSION):
	# Configure and make the local build, providing compiled resources.xxx
	if test -f ../Makefile; then cd .. && make distclean; fi
	cd .. && ./configure --prefix=$(IOS_DIR)/build/host --without-ensurepip
	cd .. && make
	cd .. && make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf host.tar.gz host && rm -rf host

build/iOS-simulator-i386.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build Simulator library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(IOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=i386-apple-ios --build=$(BUILD_OS_ID) \
			CC="$(IOS_SIMULATOR_CC)" LD="$(IOS_SIMULATOR_LD)" \
			--prefix=$(IOS_DIR)/build/iOS-simulator-i386 \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf iOS-simulator-i386.tar.gz iOS-simulator-i386 && rm -rf iOS-simulator-i386

build/iOS-simulator-x86_64.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build Simulator library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(IOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=x86_64-apple-ios --build=$(BUILD_OS_ID) \
			CC="$(IOS_SIMULATOR_64_CC)" LD="$(IOS_SIMULATOR_64_LD)" \
			--prefix=$(IOS_DIR)/build/iOS-simulator-x86_64 \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf iOS-simulator-x86_64.tar.gz iOS-simulator-x86_64 && rm -rf iOS-simulator-x86_64

build/iOS-armv7.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build ARMv7 library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(IOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=armv7-apple-ios --build=$(BUILD_OS_ID) \
			CC="$(IOS_ARMV7_CC)" LD="$(IOS_ARMV7_LD)" \
			--prefix=$(IOS_DIR)/build/iOS-armv7 \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf iOS-armv7.tar.gz iOS-armv7 && rm -rf iOS-armv7

build/iOS-armv7s.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build ARMv7s library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(IOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=armv7s-apple-ios --build=$(BUILD_OS_ID) \
			CC="$(IOS_ARMV7S_CC)" LD="$(IOS_ARMV7S_LD)" \
			--prefix=$(IOS_DIR)/build/iOS-armv7s \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf iOS-armv7s.tar.gz iOS-armv7s && rm -rf iOS-armv7s

build/iOS-arm64.tar.gz: build/host/bin/python$(PYTHON_VERSION)
	# Configure and build ARM64 library
	if test -f ../Makefile; then cd .. && make distclean; fi
	# Restore host Python
	cd build && tar zxf host.tar.gz
	# Configure and build target Python
	cd .. && \
		PATH=$(IOS_DIR)/build/host/bin:$(PATH) ./configure \
			--host=aarch64-apple-ios --build=$(BUILD_OS_ID) \
			CC="$(IOS_ARM64_CC)" LD="$(IOS_ARM64_LD)" \
			--prefix=$(IOS_DIR)/build/iOS-arm64 \
			--without-doc-strings --enable-ipv6 --without-ensurepip \
			ac_cv_file__dev_ptmx=no ac_cv_file__dev_ptc=no
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make
	cd .. && PATH=$(IOS_DIR)/build/host/bin:$(PATH) make install
	# Now pack away all the built artefacts so that the top level distclean doesn't touch them
	cd build && tar zcf iOS-arm64.tar.gz iOS-arm64 && rm -rf iOS-arm64

../Python.framework: build/iOS-simulator-i386.tar.gz build/iOS-simulator-x86_64.tar.gz build/iOS-armv7.tar.gz build/iOS-armv7s.tar.gz build/iOS-arm64.tar.gz
	FRAMEWORK_DIR=../Python.framework PRUNED=yes $(MAKE) _framework

../Tools/iOS-test/Python.framework: build/iOS-simulator-i386.tar.gz build/iOS-simulator-x86_64.tar.gz build/iOS-armv7.tar.gz build/iOS-armv7s.tar.gz build/iOS-arm64.tar.gz
	FRAMEWORK_DIR=../Tools/iOS-test/Python.framework $(MAKE) _framework

##########################################################################
# A meta-task for building an iOS framework.
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
	cd build && tar zxf iOS-simulator-i386.tar.gz
	cd build && tar zxf iOS-simulator-x86_64.tar.gz
	cd build && tar zxf iOS-armv7.tar.gz
	cd build && tar zxf iOS-armv7s.tar.gz
	cd build && tar zxf iOS-arm64.tar.gz

	# Create the framework directory
	mkdir -p $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources

	# Copy the headers. The headers are the same for every platform, except for pyconfig.h;
	# use the x86_64 simulator build because reasons.
	cp -r build/iOS-simulator-x86_64/include/python$(PYTHON_VERSION)m $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers

	# The only headers that change between versions is pyconfig.h; copy each supported version...
	cp build/iOS-simulator-i386/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-i386.h
	cp build/iOS-simulator-x86_64/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-x86_64.h
	# ARMv7 and ARMv7S headers are the same; don't copy this one.
	# cp build/iOS-armv7s/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-armv7s.h
	cp build/iOS-armv7/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-armv7.h
	cp build/iOS-arm64/include/python$(PYTHON_VERSION)m/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig-arm64.h
	# ... and then copy in a master pyconfig.h to unify them all.
	cp include/pyconfig.h $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Headers/pyconfig.h

	# Copy the standard library from the simulator build. Again, the
	# pure Python standard library is the same on every platform;
	# use the simulator version because reasons.
	mkdir -p $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources
	cp -r build/iOS-simulator-x86_64/lib $(FRAMEWORK_DIR)/Versions/$(PYTHON_VERSION)/Resources

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
		build/iOS-simulator-i386/lib/libpython$(PYTHON_VERSION)m.a \
		build/iOS-simulator-x86_64/lib/libpython$(PYTHON_VERSION)m.a \
		build/iOS-armv7/lib/libpython$(PYTHON_VERSION)m.a \
		build/iOS-armv7s/lib/libpython$(PYTHON_VERSION)m.a \
		build/iOS-arm64/lib/libpython$(PYTHON_VERSION)m.a \
		;

	# Link the current headers, Resources and Python version
	cd $(FRAMEWORK_DIR)/Versions && ln -fs $(PYTHON_VERSION) Current
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Headers
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Resources
	cd $(FRAMEWORK_DIR) && ln -fs Versions/Current/Python

	# Create the Info.plist file
	sed -e "s/xxxVERSIONxxx/$(PYTHON_VERSION)/g" Info.plist > $(FRAMEWORK_DIR)/Resources/Info.plist
