# picalc
### Calculate the constant π with the Leibniz formula

## Usage

```
./picalc [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i,--iterations UINT:POSITIVE REQUIRED
                              Number of iterations (n >= 2)
  -l,--location TEXT:DIR      Location for the partial result files
  -d,--delete                 Delete partial result files on program exit
```

## Prerequisites
- The Meson Build System
- A C++ compiler
- [CLI11](https://github.com/CLIUtils/CLI11)
- [spdlog](https://github.com/gabime/spdlog)
- [tfile](https://github.com/rec/tfile)

## Compilation

**Note: CLI11, spdlog and tfile must be present in the include folder!**

```sh
$ https://github.com/rafaelschreiber/schreiber_project_1
$ cd schreiber_project_1/build
$ meson ..
$ ninja
$ ./picalc -i 1000000000
```

Compilation was tested on the following platforms:
- macOS Big Sur (Apple clang version 12.0.0.)
- Ubuntu 20.04 LTS (gcc version 9.3.0)
