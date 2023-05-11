# PureData

Pd-server is a PureData interface to cpp-httplib. This object intends to make one way to sync live electronic music performances (based on the patches of Cort Lippe). 

<img src="https://github.com/charlesneimog/pd-server/raw/master/resources/Exemplo.gif" width=100% height=100%>
<img src="https://github.com/charlesneimog/pd-server/raw/master/resources/Exemplo-II.gif" width=100% height=100%>

# Build
## Windows
* First download the repo using: `git clone https://github.com/charlesneimog/pd-server --recursive`.
* On mingw64, run `pacman -S mingw-w64-x86_64-openssl mingw-w64-x86_64-crypto++ mingw-w64-x86_64-libwinpthread-git`.
* Then run `make`.

## Linux
* First download the repo using: `git clone https://github.com/charlesneimog/pd-server --recursive`.
* Install openssl `sudo dnf install openssl-devel` or `sudo apt-get install libssl-dev` and run `make`.

## MacOS
* First download the repo using: `git clone https://github.com/charlesneimog/pd-server --recursive`.
* Then I don't know.

# License

MIT license (© 2022 Yuji Hirose)
