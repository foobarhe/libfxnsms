
install: 
	cp -f ./lib/libfxnsms.a /usr/local/lib
	cp -f ./lib/libfxnsms.so /usr/local/lib
	cp -f ./include/fxn_sms.h /usr/local/include
	ldconfig

clean:
	rm -f /usr/local/lib/libfxnsms.*
	rm -f /usr/local/include/fxn_sms.h
	ldconfig
