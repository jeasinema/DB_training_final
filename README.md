### DB Trainning Final

#### Dependency
`metis`, a lib for graph partition.

- Debian
```bash
$ sudo apt install libmetis-dev
```
- macOS(with `Homebrew`)
```bash
$ brew install metis
```

#### Build
```bash
$ mkdir build; cd build
$ cmake ..; make
```
#### Run
```bash
$ unzip map/GP_Tree.data.zip; cd build
$ ln ./DriveSharing ../.; cd ..
$ ./DriveSharing
```
#### Files
```plain
--+-- ./src   source code
  |  
  +-- ./include  lib header file
  |
  +-- ./map  map file(Beijing)
  |
  +-- ./test    test data
```
