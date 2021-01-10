project('picalc_07', 'cpp',
        license : ['Boost'],
        version : '1.0',
        default_options : ['cpp_std=c++17', 'warning_level=3', 'werror=false'])


# CLI11: https://github.com/CLIUtils/CLI11
add_global_arguments('-I' + get_option('cli11_include_dir'), language : 'cpp')

# spdlog: https://github.com/gabime/spdlog
add_global_arguments('-I' + get_option('spdlog_include_dir'), language : 'cpp')

add_global_arguments('-w', language : 'cpp')

inc_dir = include_directories('include')
src = ['src/main.cpp']

executable('picalc',
           sources : src,
           include_directories : inc_dir
          )
