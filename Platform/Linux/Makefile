include ../../common.mk

SOURCES = Platform.cc ActiveWindowObserver.cc KWin.cc PrivateActiveWindowObserver.cc
QT_LIBS = Core DBus

all: Linux.a

Linux.a: kwin_script.c $(OBJECTS)
	rm -f $@
	$(AR) rcs $@ $(OBJECTS)

.work/node_modules/.bin/minify:
	rm -rf .work
	mkdir -p .work/node_modules
	cd .work && yes | yarn init
	cd .work && yarn add --modules-folder node_modules minify
	[ -f .work/node_modules/.bin/minify ]

kwin_script_minified.js: kwin_script.js .work/node_modules/.bin/minify
	.work/node_modules/.bin/minify --js < $< > $@

kwin_script.c: kwin_script_minified.js
	echo "static const char kwin_script[] = " > kwin_script.c
	hexdump -v -e '16/1 "_x%02X" "\n"' < kwin_script_minified.js | sed 's/_/\\/g; s/\\x  //g; s/.*/    "&"/' >> kwin_script.c
	echo ";" >> kwin_script.c
	echo -n "static const size_t kwin_script_len = " >> kwin_script.c
	wc -c < kwin_script_minified.js >> kwin_script.c
	echo ";" >> kwin_script.c

clean:
	rm -f $(OBJECTS) Linux.a kwin_script.c kwin_script_minified.js