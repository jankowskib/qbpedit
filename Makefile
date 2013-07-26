qbpedit: qbpedit.c
	gcc -o $@ $^
clean:
	rm qbpedit
uninstall:
	rm /usr/bin/qbpedit
install:
	cp qbpedit /usr/bin/qbpedit
	chmod +x /usr/bin/qbpedit
