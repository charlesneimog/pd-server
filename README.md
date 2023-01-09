# PureData

Pd-server is a PureData interface to cpp-httplib. This object intends to make one way to sync live electronic music performances (based on the patches of Cort Lippe). 

<img src="https://github.com/charlesneimog/pd-server/raw/master/resources/Exemplo.gif" width=50% height=50%>
<img src="https://github.com/charlesneimog/pd-server/raw/master/resources/Exemplo.gif" width=50% height=50%>

# Build
## Windows

* On mingw64, run `pacman -S mingw-w64-x86_64-openssl mingw-w64-x86_64-crypto++ mingw-w64-x86_64-libwinpthread-git`.
* Then run `make`.

## Linux

* Install openssl `sudo dnf install openssl-devel` or `sudo apt-get install libssl-dev` and run `make`.

## MacOS

* I don't know.

# License

MIT license (© 2022 Yuji Hirose)
